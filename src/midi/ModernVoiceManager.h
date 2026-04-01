#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "../dsp/modern/ModernEngine.h"
#include "TuningTable.h"
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
    void setTuningTable (TuningTable* table)            { tuningTablePtr = table; }

    void processMidiMessage (const juce::MidiMessage& msg);
    void applyPitchMultiplier (float mult);
    void handleAllNotesOff();

    float getLastVelocity() const { return lastVelocity; }

    /// Returns true if MPE mode is active (MCM received with zone size > 0)
    bool isMpeEnabled() const { return mpeEnabled; }

    /// Callback for MIDI CC messages (ccNumber, value01)
    std::function<void(int, float)> onControlChange;

private:
    void handleNoteOn (int note, float velocity, int midiChannel = 1);
    void handleNoteOff (int note, int midiChannel = 1);
    void handlePitchBend (int bendValue, int midiChannel = 1);
    void handleMpeSlide (int midiChannel, float value01);
    void handleMpePressure (int midiChannel, float value01);

    float getFrequencyForNote (int note) const;
    float getFrequencyForNoteWithBend (int note, float bendSemitones) const;

    ModernEngine* enginePtr       = nullptr;
    float         velocitySens    = 0.0f;
    int           pitchBendSemitones = 2;
    float         masterTuneCents = 0.0f;
    TuningTable*  tuningTablePtr  = nullptr;
    float         pitchBendValue  = 0.0f;   // normalized -1 to 1
    float         lastVelocity    = 0.8f;

    // MPE support
    bool          mpeEnabled         = false;
    int           mpeMemberChannels  = 0;     // 0 = MPE off, >0 = number of member channels
    float         perVoiceBendSemitones[16] = {};  // Per-voice pitch bend in semitones

    // RPN tracking for MPE Configuration Message detection
    int           rpnMsb = -1;
    int           rpnLsb = -1;
};

} // namespace cart
