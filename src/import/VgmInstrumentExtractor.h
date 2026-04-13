#pragma once

#include "VgmParser.h"
#include "../dsp/StepSequencer.h"
#include <vector>

namespace cart {

struct ExtractedInstrument
{
    juce::String name;
    int  channelIndex = 0;     // 0=Pulse1, 1=Pulse2, 2=Triangle, 3=Noise
    int  duty         = 1;     // Initial duty cycle
    int  volume       = 15;    // Initial volume
    StepSequenceData seqData;
};

/// Extracts per-channel instrument envelopes from VGM NES APU register writes.
class VgmInstrumentExtractor
{
public:
    /// Process a parsed VGM and extract unique instruments.
    /// cpuClock should be the NES APU clock from the VGM header
    /// (typically 1789773 for NTSC or 1662607 for PAL).
    void extract (const std::vector<VgmCommand>& commands, uint32_t cpuClock);

    const std::vector<ExtractedInstrument>& getInstruments() const { return instruments; }

private:
    // Per-channel state tracking
    struct ChannelState
    {
        uint8_t  regs[4] = {};
        uint64_t lastWriteTime = 0;
        bool     noteActive = false;

        // Accumulated per-frame snapshots for current note
        struct Frame { int volume; int duty; float pitchSemitones; };
        std::vector<Frame> frames;
        float basePitchSemitone = 0.0f;
    };

    void processNoteEnd (int ch, ChannelState& state, uint32_t cpuClock);
    float timerToSemitone (uint16_t period, uint32_t cpuClock, bool isTriangle) const;
    bool isDuplicate (const StepSequenceData& sd) const;

    static constexpr int kFrameSamples = 735; // ~60 Hz NTSC frame

    ChannelState channels[4]; // Pulse1, Pulse2, Triangle, Noise
    std::vector<ExtractedInstrument> instruments;
    int noteCounters[4] = {};
};

} // namespace cart
