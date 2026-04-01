#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "../dsp/modern/ModernEngine.h"
#include <functional>

namespace cart {

/// MIDI router for the Modern polyphonic engine.
/// Simpler than MidiVoiceManager — no per-channel routing, no split/auto modes.
class ModernVoiceManager
{
public:
    void setEngine (ModernEngine* eng)                { enginePtr = eng; }
    void setVelocitySensitivity (float sens)          { velocitySens = sens; }
    void setPitchBendRange (int semitones)             { pitchBendSemitones = semitones; }
    void setMasterTune (float cents)                   { masterTuneCents = cents; }

    void processMidiMessage (const juce::MidiMessage& msg);
    void applyPitchMultiplier (float mult);
    void handleAllNotesOff();

    float getLastVelocity() const { return lastVelocity; }

    /// Callback for MIDI CC messages (ccNumber, value01)
    std::function<void(int, float)> onControlChange;

private:
    void handleNoteOn (int note, float velocity);
    void handleNoteOff (int note);
    void handlePitchBend (int bendValue);

    float getFrequencyForNote (int note) const;

    ModernEngine* enginePtr       = nullptr;
    float         velocitySens    = 0.0f;
    int           pitchBendSemitones = 2;
    float         masterTuneCents = 0.0f;
    float         pitchBendValue  = 0.0f;   // normalized -1 to 1
    float         lastVelocity    = 0.8f;
};

} // namespace cart
