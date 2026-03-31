#include "MidiVoiceManager.h"
#include "NoteFrequencyTable.h"
#include "../dsp/ApuConstants.h"
#include "../dsp/DpcmSamples.h"
#include <cmath>

namespace cart {

float MidiVoiceManager::getAdjustedFrequency (int note, float bendSemitones, int nesChannel) const
{
    float transposeOffset = (nesChannel >= 0 && nesChannel < 8) ? transpose[nesChannel] : 0.0f;
    float adjustedNote = static_cast<float> (note) + bendSemitones + transposeOffset + (masterTuneCents / 100.0f);
    return midiNoteToFrequency (adjustedNote);
}

void MidiVoiceManager::processMidiMessage (const juce::MidiMessage& msg)
{
    if (apuPtr == nullptr)
        return;

    if (msg.isNoteOn())
    {
        handleNoteOn (msg.getChannel(), msg.getNoteNumber(),
                      msg.getFloatVelocity());
    }
    else if (msg.isNoteOff())
    {
        handleNoteOff (msg.getChannel(), msg.getNoteNumber());
    }
    else if (msg.isPitchWheel())
    {
        handlePitchBend (msg.getChannel(), msg.getPitchWheelValue());
    }
    else if (msg.isController())
    {
        if (onControlChange)
            onControlChange (msg.getControllerNumber(),
                             msg.getControllerValue() / 127.0f);
    }
    else if (msg.isAllNotesOff() || msg.isAllSoundOff())
    {
        handleAllNotesOff();
    }
}

void MidiVoiceManager::handleNoteOn (int channel, int note, float velocity)
{
    // Apply velocity sensitivity
    float vol = 1.0f - velocitySens + velocitySens * velocity;
    float bendSemitones = pitchBendValues[channel - 1] * static_cast<float> (pitchBendSemitones);

    if (mode == MidiMode::Split)
    {
        switch (channel)
        {
            case 1:  // Pulse 1
            {
                float freq = getAdjustedFrequency (note, bendSemitones, 0);
                porta[0].setTarget (freq);
                apuPtr->pulse1().setFrequency (porta[0].getCurrentFreq() > 0.0f ? porta[0].getCurrentFreq() : freq);
                apuPtr->pulse1().envelope().setVolume (
                    static_cast<uint8_t> (vol * 15.0f));
                apuPtr->pulse1().noteOn();
                activeNotes[0] = note;
                break;
            }
            case 2:  // Pulse 2
            {
                float freq = getAdjustedFrequency (note, bendSemitones, 1);
                porta[1].setTarget (freq);
                apuPtr->pulse2().setFrequency (porta[1].getCurrentFreq() > 0.0f ? porta[1].getCurrentFreq() : freq);
                apuPtr->pulse2().envelope().setVolume (
                    static_cast<uint8_t> (vol * 15.0f));
                apuPtr->pulse2().noteOn();
                activeNotes[1] = note;
                break;
            }
            case 3:  // Triangle
            {
                float freq = getAdjustedFrequency (note, bendSemitones, 2);
                porta[2].setTarget (freq);
                apuPtr->triangle().setFrequency (porta[2].getCurrentFreq() > 0.0f ? porta[2].getCurrentFreq() : freq);
                apuPtr->triangle().noteOn();
                activeNotes[2] = note;
                break;
            }
            case 10: // Noise
            {
                // Map MIDI note to noise period (higher note = higher pitch = lower period index)
                // Notes 36–96 mapped to period indices 15–0
                int periodIndex = 15 - std::clamp ((note - 36) * 16 / 60, 0, 15);
                apuPtr->noise().setPeriodIndex (periodIndex, isNtsc);
                apuPtr->noise().envelope().setVolume (
                    static_cast<uint8_t> (vol * 15.0f));
                apuPtr->noise().noteOn();
                activeNotes[3] = note;
                break;
            }
            case 4:  // DPCM
            {
                // Note-to-sample mapping: specific MIDI notes trigger specific drums
                int mappedSample = dpcmNoteToSampleIndex (note);
                if (mappedSample >= 0)
                    apuPtr->dpcm().loadSample (getDpcmSample (mappedSample));
                apuPtr->dpcm().noteOn();
                activeNotes[4] = note;
                break;
            }
            case 5:  // VRC6 Pulse 1
            {
                if (vrc6Available)
                {
                    float freq = getAdjustedFrequency (note, bendSemitones, 5);
                    porta[5].setTarget (freq);
                    float startFreq = porta[5].getCurrentFreq() > 0.0f ? porta[5].getCurrentFreq() : freq;
                    apuPtr->vrc6Pulse1().setVolume (static_cast<uint8_t> (vol * 15.0f));
                    apuPtr->vrc6Pulse1().noteOn (startFreq);
                    activeNotes[5] = note;
                }
                break;
            }
            case 6:  // VRC6 Pulse 2
            {
                if (vrc6Available)
                {
                    float freq = getAdjustedFrequency (note, bendSemitones, 6);
                    porta[6].setTarget (freq);
                    float startFreq = porta[6].getCurrentFreq() > 0.0f ? porta[6].getCurrentFreq() : freq;
                    apuPtr->vrc6Pulse2().setVolume (static_cast<uint8_t> (vol * 15.0f));
                    apuPtr->vrc6Pulse2().noteOn (startFreq);
                    activeNotes[6] = note;
                }
                break;
            }
            case 7:  // VRC6 Sawtooth
            {
                if (vrc6Available)
                {
                    float freq = getAdjustedFrequency (note, bendSemitones, 7);
                    porta[7].setTarget (freq);
                    float startFreq = porta[7].getCurrentFreq() > 0.0f ? porta[7].getCurrentFreq() : freq;
                    apuPtr->vrc6Saw().noteOn (startFreq);
                    activeNotes[7] = note;
                }
                break;
            }
            default:
                break;
        }
    }
    else if (mode == MidiMode::Mono)
    {
        // Mono mode: play on Pulse 1 only
        float freq = getAdjustedFrequency (note, bendSemitones, 0);
        porta[0].setTarget (freq);
        apuPtr->pulse1().setFrequency (porta[0].getCurrentFreq() > 0.0f ? porta[0].getCurrentFreq() : freq);
        apuPtr->pulse1().envelope().setVolume (
            static_cast<uint8_t> (vol * 15.0f));
        apuPtr->pulse1().noteOn();
        activeNotes[0] = note;
    }
    else if (mode == MidiMode::Auto)
    {
        // Auto poly: round-robin across ALL enabled channels
        static constexpr int allSlots[] = { 0, 1, 2, 3, 4, 5, 6, 7 };

        int candidates[8];
        int numCandidates = 0;
        for (int s : allSlots)
        {
            if (! channelEnabled[s]) continue;
            if (s >= 5 && ! vrc6Available) continue;
            candidates[numCandidates++] = s;
        }

        if (numCandidates == 0)
            return;

        // Prefer a free channel, starting from round-robin position
        int chosen = -1;
        for (int i = 0; i < numCandidates; ++i)
        {
            int idx = (nextAutoChannel + i) % numCandidates;
            if (activeNotes[candidates[idx]] < 0)
            {
                chosen = idx;
                break;
            }
        }

        // All channels busy — steal the round-robin slot
        if (chosen < 0)
        {
            chosen = nextAutoChannel % numCandidates;
            noteOffChannel (candidates[chosen]);
        }

        noteOnChannel (candidates[chosen], note, vol, bendSemitones);
        nextAutoChannel = (chosen + 1) % numCandidates;
    }
}

void MidiVoiceManager::handleNoteOff (int channel, int note)
{
    if (mode == MidiMode::Split)
    {
        switch (channel)
        {
            case 1:
                if (activeNotes[0] == note)
                {
                    apuPtr->pulse1().noteOff();
                    activeNotes[0] = -1;
                }
                break;
            case 2:
                if (activeNotes[1] == note)
                {
                    apuPtr->pulse2().noteOff();
                    activeNotes[1] = -1;
                }
                break;
            case 3:
                if (activeNotes[2] == note)
                {
                    apuPtr->triangle().noteOff();
                    activeNotes[2] = -1;
                }
                break;
            case 10:
                if (activeNotes[3] == note)
                {
                    apuPtr->noise().noteOff();
                    activeNotes[3] = -1;
                }
                break;
            case 4:
                if (activeNotes[4] == note)
                {
                    apuPtr->dpcm().noteOff();
                    activeNotes[4] = -1;
                }
                break;
            case 5:
                if (vrc6Available && activeNotes[5] == note)
                {
                    apuPtr->vrc6Pulse1().noteOff();
                    activeNotes[5] = -1;
                }
                break;
            case 6:
                if (vrc6Available && activeNotes[6] == note)
                {
                    apuPtr->vrc6Pulse2().noteOff();
                    activeNotes[6] = -1;
                }
                break;
            case 7:
                if (vrc6Available && activeNotes[7] == note)
                {
                    apuPtr->vrc6Saw().noteOff();
                    activeNotes[7] = -1;
                }
                break;
            default:
                break;
        }
    }
    else if (mode == MidiMode::Mono)
    {
        if (activeNotes[0] == note)
        {
            apuPtr->pulse1().noteOff();
            activeNotes[0] = -1;
        }
    }
    else if (mode == MidiMode::Auto)
    {
        if (activeNotes[0] == note) { apuPtr->pulse1().noteOff();    activeNotes[0] = -1; }
        if (activeNotes[1] == note) { apuPtr->pulse2().noteOff();    activeNotes[1] = -1; }
        if (activeNotes[2] == note) { apuPtr->triangle().noteOff();  activeNotes[2] = -1; }
        if (activeNotes[3] == note) { apuPtr->noise().noteOff();     activeNotes[3] = -1; }
        if (activeNotes[4] == note) { apuPtr->dpcm().noteOff();      activeNotes[4] = -1; }
        if (vrc6Available)
        {
            if (activeNotes[5] == note) { apuPtr->vrc6Pulse1().noteOff(); activeNotes[5] = -1; }
            if (activeNotes[6] == note) { apuPtr->vrc6Pulse2().noteOff(); activeNotes[6] = -1; }
            if (activeNotes[7] == note) { apuPtr->vrc6Saw().noteOff();    activeNotes[7] = -1; }
        }
    }
}

void MidiVoiceManager::handlePitchBend (int channel, int bendValue)
{
    // MIDI pitch bend: 0–16383, center at 8192
    float normalized = (static_cast<float> (bendValue) - 8192.0f) / 8192.0f;
    pitchBendValues[channel - 1] = normalized;

    float bendSemitones = normalized * static_cast<float> (pitchBendSemitones);

    if (mode == MidiMode::Split)
    {
        // Update frequency for the active note on the corresponding channel
        auto updateFreq = [&] (int nesChannel, auto& ch)
        {
            if (activeNotes[nesChannel] >= 0)
            {
                float freq = getAdjustedFrequency (activeNotes[nesChannel], bendSemitones, nesChannel);
                ch.setFrequency (freq);
            }
        };

        switch (channel)
        {
            case 1:  updateFreq (0, apuPtr->pulse1());   break;
            case 2:  updateFreq (1, apuPtr->pulse2());   break;
            case 3:  updateFreq (2, apuPtr->triangle()); break;
            case 5:  if (vrc6Available) updateFreq (5, apuPtr->vrc6Pulse1()); break;
            case 6:  if (vrc6Available) updateFreq (6, apuPtr->vrc6Pulse2()); break;
            case 7:  if (vrc6Available) updateFreq (7, apuPtr->vrc6Saw());    break;
            default: break;
        }
    }
    else if (mode == MidiMode::Mono)
    {
        if (activeNotes[0] >= 0)
        {
            float freq = getAdjustedFrequency (activeNotes[0], bendSemitones, 0);
            apuPtr->pulse1().setFrequency (freq);
        }
    }
    else if (mode == MidiMode::Auto)
    {
        auto updateIfActive = [&] (int idx, auto& ch)
        {
            if (activeNotes[idx] >= 0)
                ch.setFrequency (getAdjustedFrequency (activeNotes[idx], bendSemitones, idx));
        };
        updateIfActive (0, apuPtr->pulse1());
        updateIfActive (1, apuPtr->pulse2());
        updateIfActive (2, apuPtr->triangle());
        if (vrc6Available)
        {
            updateIfActive (5, apuPtr->vrc6Pulse1());
            updateIfActive (6, apuPtr->vrc6Pulse2());
            updateIfActive (7, apuPtr->vrc6Saw());
        }
    }
}

void MidiVoiceManager::handleAllNotesOff()
{
    if (activeNotes[0] >= 0) { apuPtr->pulse1().noteOff();    activeNotes[0] = -1; }
    if (activeNotes[1] >= 0) { apuPtr->pulse2().noteOff();    activeNotes[1] = -1; }
    if (activeNotes[2] >= 0) { apuPtr->triangle().noteOff();  activeNotes[2] = -1; }
    if (activeNotes[3] >= 0) { apuPtr->noise().noteOff();     activeNotes[3] = -1; }
    if (activeNotes[4] >= 0) { apuPtr->dpcm().noteOff();      activeNotes[4] = -1; }
    if (vrc6Available)
    {
        if (activeNotes[5] >= 0) { apuPtr->vrc6Pulse1().noteOff(); activeNotes[5] = -1; }
        if (activeNotes[6] >= 0) { apuPtr->vrc6Pulse2().noteOff(); activeNotes[6] = -1; }
        if (activeNotes[7] >= 0) { apuPtr->vrc6Saw().noteOff();    activeNotes[7] = -1; }
    }
}

void MidiVoiceManager::setPortamentoEnabled (bool en)
{
    for (auto& p : porta) p.setEnabled (en);
}

void MidiVoiceManager::setPortamentoTime (float seconds)
{
    for (auto& p : porta) p.setTime (seconds);
}

void MidiVoiceManager::setSampleRateForPorta (double sr)
{
    for (auto& p : porta) p.setSampleRate (sr);
}

void MidiVoiceManager::tickPortamento()
{
    if (apuPtr == nullptr) return;

    auto updateChannel = [&] (int idx, auto& channel)
    {
        if (activeNotes[idx] >= 0 && porta[idx].isGliding())
        {
            float freq = porta[idx].process();
            channel.setFrequency (freq);
        }
    };

    updateChannel (0, apuPtr->pulse1());
    updateChannel (1, apuPtr->pulse2());
    updateChannel (2, apuPtr->triangle());
    if (vrc6Available)
    {
        updateChannel (5, apuPtr->vrc6Pulse1());
        updateChannel (6, apuPtr->vrc6Pulse2());
        updateChannel (7, apuPtr->vrc6Saw());
    }
}

void MidiVoiceManager::applyPitchMultiplier (float multiplier)
{
    if (apuPtr == nullptr || multiplier == 1.0f) return;

    for (int i = 0; i < 8; ++i)
    {
        if (activeNotes[i] < 0) continue;

        float freq = midiNoteToFrequency (static_cast<float> (activeNotes[i]) + transpose[i] + masterTuneCents / 100.0f) * multiplier;

        switch (i)
        {
            case 0: apuPtr->pulse1().setFrequency (freq); break;
            case 1: apuPtr->pulse2().setFrequency (freq); break;
            case 2: apuPtr->triangle().setFrequency (freq); break;
            case 5: if (vrc6Available) apuPtr->vrc6Pulse1().setFrequency (freq); break;
            case 6: if (vrc6Available) apuPtr->vrc6Pulse2().setFrequency (freq); break;
            case 7: if (vrc6Available) apuPtr->vrc6Saw().setFrequency (freq); break;
            default: break;
        }
    }
}

void MidiVoiceManager::noteOnChannel (int ch, int note, float vol, float bendSemitones)
{
    float freq = getAdjustedFrequency (note, bendSemitones, ch);

    switch (ch)
    {
        case 0:
            porta[0].setTarget (freq);
            apuPtr->pulse1().setFrequency (porta[0].getCurrentFreq() > 0.0f ? porta[0].getCurrentFreq() : freq);
            apuPtr->pulse1().envelope().setVolume (static_cast<uint8_t> (vol * 15.0f));
            apuPtr->pulse1().noteOn();
            break;
        case 1:
            porta[1].setTarget (freq);
            apuPtr->pulse2().setFrequency (porta[1].getCurrentFreq() > 0.0f ? porta[1].getCurrentFreq() : freq);
            apuPtr->pulse2().envelope().setVolume (static_cast<uint8_t> (vol * 15.0f));
            apuPtr->pulse2().noteOn();
            break;
        case 2:
            porta[2].setTarget (freq);
            apuPtr->triangle().setFrequency (porta[2].getCurrentFreq() > 0.0f ? porta[2].getCurrentFreq() : freq);
            apuPtr->triangle().noteOn();
            break;
        case 3:
        {
            int periodIndex = 15 - std::clamp ((note - 36) * 16 / 60, 0, 15);
            apuPtr->noise().setPeriodIndex (periodIndex, isNtsc);
            apuPtr->noise().envelope().setVolume (static_cast<uint8_t> (vol * 15.0f));
            apuPtr->noise().noteOn();
            break;
        }
        case 4:
        {
            // Note-to-sample mapping for DPCM in Auto mode
            int mappedSample = dpcmNoteToSampleIndex (note);
            if (mappedSample >= 0)
                apuPtr->dpcm().loadSample (getDpcmSample (mappedSample));
            apuPtr->dpcm().noteOn();
            break;
        }
        case 5:
            porta[5].setTarget (freq);
            apuPtr->vrc6Pulse1().setVolume (static_cast<uint8_t> (vol * 15.0f));
            apuPtr->vrc6Pulse1().noteOn (porta[5].getCurrentFreq() > 0.0f ? porta[5].getCurrentFreq() : freq);
            break;
        case 6:
            porta[6].setTarget (freq);
            apuPtr->vrc6Pulse2().setVolume (static_cast<uint8_t> (vol * 15.0f));
            apuPtr->vrc6Pulse2().noteOn (porta[6].getCurrentFreq() > 0.0f ? porta[6].getCurrentFreq() : freq);
            break;
        case 7:
            porta[7].setTarget (freq);
            apuPtr->vrc6Saw().noteOn (porta[7].getCurrentFreq() > 0.0f ? porta[7].getCurrentFreq() : freq);
            break;
        default: break;
    }
    activeNotes[ch] = note;
}

void MidiVoiceManager::noteOffChannel (int ch)
{
    switch (ch)
    {
        case 0: apuPtr->pulse1().noteOff();    break;
        case 1: apuPtr->pulse2().noteOff();    break;
        case 2: apuPtr->triangle().noteOff();  break;
        case 3: apuPtr->noise().noteOff();     break;
        case 4: apuPtr->dpcm().noteOff();      break;
        case 5: apuPtr->vrc6Pulse1().noteOff(); break;
        case 6: apuPtr->vrc6Pulse2().noteOff(); break;
        case 7: apuPtr->vrc6Saw().noteOff();    break;
        default: break;
    }
    activeNotes[ch] = -1;
}

} // namespace cart
