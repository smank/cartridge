#include "VgmInstrumentExtractor.h"
#include <cmath>
#include <algorithm>

namespace cart {

namespace
{
    const juce::String channelPrefixes[] = { "Pulse1", "Pulse2", "Triangle", "Noise" };

    bool mapRegister (uint8_t reg, int& ch, int& localReg)
    {
        if (reg <= 0x03)                    { ch = 0; localReg = reg;        return true; }
        if (reg >= 0x04 && reg <= 0x07)     { ch = 1; localReg = reg - 0x04; return true; }
        if (reg >= 0x08 && reg <= 0x0B)     { ch = 2; localReg = reg - 0x08; return true; }
        if (reg >= 0x0C && reg <= 0x0F)     { ch = 3; localReg = reg - 0x0C; return true; }
        return false;
    }

    int getVolume (const uint8_t* regs, int ch)
    {
        if (ch == 2) return 15; // Triangle: always max when active
        return regs[0] & 0x0F;
    }

    int getDuty (const uint8_t* regs, int ch)
    {
        if (ch >= 2) return 0;
        return (regs[0] >> 6) & 0x03;
    }

    uint16_t getTimerPeriod (const uint8_t* regs)
    {
        return static_cast<uint16_t> (regs[2] | ((regs[3] & 0x07) << 8));
    }
}

float VgmInstrumentExtractor::timerToSemitone (uint16_t period, uint32_t cpuClock, bool isTriangle) const
{
    if (period == 0) return 0.0f;
    double divisor = isTriangle ? 32.0 : 16.0;
    double freq = static_cast<double> (cpuClock) / (divisor * (period + 1));
    if (freq <= 0.0) return 0.0f;
    return static_cast<float> (69.0 + 12.0 * std::log2 (freq / 440.0));
}

void VgmInstrumentExtractor::processNoteEnd (int ch, ChannelState& state, uint32_t cpuClock)
{
    state.noteActive = false;

    if (state.frames.size() < 2)
        return; // Need at least 2 frames for meaningful envelope

    StepSequenceData sd;
    int numFrames = static_cast<int> (state.frames.size());
    int numSteps = std::min (numFrames, StepSequenceData::kMaxSteps);

    bool hasVolumeVariation = false;
    bool hasPitchVariation = false;
    bool hasDutyVariation = false;

    int firstVol = state.frames[0].volume;
    int firstDuty = state.frames[0].duty;

    for (int i = 0; i < numSteps; ++i)
    {
        auto& f = state.frames[i];
        sd.volumeSteps[i] = std::clamp (f.volume, 0, 15);
        sd.dutySteps[i]   = std::clamp (f.duty, 0, 7);

        float pitchDelta = f.pitchSemitones - state.basePitchSemitone;
        sd.pitchSteps[i] = std::clamp (static_cast<int> (std::round (pitchDelta)), -12, 12);

        if (f.volume != firstVol) hasVolumeVariation = true;
        if (sd.pitchSteps[i] != 0) hasPitchVariation = true;
        if (f.duty != firstDuty) hasDutyVariation = true;
    }

    // Only create lanes that have variation
    sd.numVolumeSteps = hasVolumeVariation ? numSteps : 0;
    sd.numPitchSteps  = hasPitchVariation ? numSteps : 0;
    sd.numDutySteps   = hasDutyVariation ? numSteps : 0;

    if (!hasVolumeVariation && !hasPitchVariation && !hasDutyVariation)
    {
        state.frames.clear();
        return;
    }

    // Loop detection
    auto detectLoop = [] (const int* steps, int count) -> bool
    {
        if (count < 4) return false;
        for (int len = 1; len <= count / 2; ++len)
        {
            bool matches = true;
            for (int i = 0; i < len && i + len < count; ++i)
            {
                if (steps[count - 2 * len + i] != steps[count - len + i])
                { matches = false; break; }
            }
            if (matches) return true;
        }
        return false;
    };

    if (sd.numVolumeSteps > 0) sd.volumeLoop = detectLoop (sd.volumeSteps, sd.numVolumeSteps);
    if (sd.numPitchSteps > 0)  sd.pitchLoop  = detectLoop (sd.pitchSteps, sd.numPitchSteps);
    if (sd.numDutySteps > 0)   sd.dutyLoop   = detectLoop (sd.dutySteps, sd.numDutySteps);

    if (!isDuplicate (sd))
    {
        ExtractedInstrument inst;
        inst.channelIndex = ch;
        inst.duty = firstDuty;
        inst.volume = firstVol;
        inst.seqData = sd;

        const char* label = hasVolumeVariation ? (firstVol > 10 ? "Lead" : "Soft")
                          : hasPitchVariation  ? "Slide"
                          : "Morph";
        inst.name = channelPrefixes[ch] + " " + label + " " + juce::String (++noteCounters[ch]);
        instruments.push_back (std::move (inst));
    }

    state.frames.clear();
}

bool VgmInstrumentExtractor::isDuplicate (const StepSequenceData& sd) const
{
    for (const auto& inst : instruments)
    {
        const auto& o = inst.seqData;
        if (o.numVolumeSteps != sd.numVolumeSteps ||
            o.numPitchSteps != sd.numPitchSteps ||
            o.numDutySteps != sd.numDutySteps)
            continue;

        bool match = true;
        for (int i = 0; i < sd.numVolumeSteps && match; ++i)
            if (o.volumeSteps[i] != sd.volumeSteps[i]) match = false;
        for (int i = 0; i < sd.numPitchSteps && match; ++i)
            if (o.pitchSteps[i] != sd.pitchSteps[i]) match = false;
        for (int i = 0; i < sd.numDutySteps && match; ++i)
            if (o.dutySteps[i] != sd.dutySteps[i]) match = false;

        if (match) return true;
    }
    return false;
}

void VgmInstrumentExtractor::extract (const std::vector<VgmCommand>& commands, uint32_t cpuClock)
{
    instruments.clear();
    for (auto& ch : channels) ch = {};
    for (auto& n : noteCounters) n = 0;

    if (cpuClock == 0) cpuClock = 1789773;

    // Two-pass approach:
    // 1. Apply all register writes with timestamps
    // 2. For each channel, sample state at frame boundaries between note-on events

    // First, collect all events with absolute timestamps
    struct Event
    {
        uint64_t time;
        int ch;
        int localReg;
        uint8_t data;
    };

    std::vector<Event> events;
    events.reserve (commands.size());

    for (const auto& cmd : commands)
    {
        if (cmd.type != VgmCommand::NesApuWrite) continue;
        int ch, lr;
        if (mapRegister (cmd.reg, ch, lr))
            events.push_back ({ cmd.sampleTime, ch, lr, cmd.data });
    }

    if (events.empty()) return;

    // Process events chronologically
    uint64_t nextFrameTime = kFrameSamples; // First frame tick

    for (size_t ei = 0; ei < events.size(); ++ei)
    {
        auto& ev = events[ei];

        // Sample all active channels at frame boundaries up to this event's time
        while (nextFrameTime <= ev.time)
        {
            for (int c = 0; c < 4; ++c)
            {
                if (!channels[c].noteActive) continue;
                auto& s = channels[c];

                ChannelState::Frame f;
                f.volume = getVolume (s.regs, c);
                f.duty = getDuty (s.regs, c);
                f.pitchSemitones = (c < 3) ? timerToSemitone (getTimerPeriod (s.regs), cpuClock, c == 2) : 0.0f;
                s.frames.push_back (f);

                // Cap frame count
                if (s.frames.size() >= 64)
                    processNoteEnd (c, s, cpuClock);
            }
            nextFrameTime += kFrameSamples;
        }

        // Apply register write
        auto& state = channels[ev.ch];
        state.regs[ev.localReg] = ev.data;

        // Detect note-on: write to register 3
        if (ev.localReg == 3)
        {
            if (state.noteActive)
                processNoteEnd (ev.ch, state, cpuClock);

            int vol = getVolume (state.regs, ev.ch);
            uint16_t period = (ev.ch < 3) ? getTimerPeriod (state.regs) : 1;

            if (vol > 0 && period > 0)
            {
                state.noteActive = true;
                state.frames.clear();
                state.basePitchSemitone = (ev.ch < 3)
                    ? timerToSemitone (period, cpuClock, ev.ch == 2) : 0.0f;

                // Capture initial frame immediately
                ChannelState::Frame f;
                f.volume = vol;
                f.duty = getDuty (state.regs, ev.ch);
                f.pitchSemitones = state.basePitchSemitone;
                state.frames.push_back (f);
            }
        }

        // Channel silence detection: if volume written to 0, end note
        if (ev.localReg == 0 && ev.ch != 2) // reg 0 contains volume for pulse/noise
        {
            if ((ev.data & 0x0F) == 0 && state.noteActive)
                processNoteEnd (ev.ch, state, cpuClock);
        }

        // Limit total extracted instruments
        if (noteCounters[ev.ch] >= 50)
            channels[ev.ch].noteActive = false;
    }

    // Finalize any remaining active notes
    for (int c = 0; c < 4; ++c)
        if (channels[c].noteActive)
            processNoteEnd (c, channels[c], cpuClock);
}

} // namespace cart
