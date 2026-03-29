#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "../dsp/Apu.h"
#include "../dsp/Portamento.h"

namespace cart {

/// MIDI routing modes
enum class MidiMode
{
    Split,   // Fixed MIDI channel → APU channel mapping
    Auto,    // Round-robin across melodic channels
    Mono     // Single channel mode
};

/// Manages MIDI → APU channel routing.
/// Split mode (default):
///   MIDI Ch 1 → Pulse 1, Ch 2 → Pulse 2, Ch 3 → Triangle, Ch 10 → Noise, Ch 4 → DPCM
class MidiVoiceManager
{
public:
    void setApu (Apu* apu)          { apuPtr = apu; }
    void setMode (MidiMode m)          { mode = m; }
    void setRegion (bool ntsc)         { isNtsc = ntsc; }

    /// Velocity sensitivity: 0 = ignore velocity, 1 = full velocity response
    void setVelocitySensitivity (float sens) { velocitySens = sens; }

    /// Pitch bend range in semitones
    void setPitchBendRange (int semitones)   { pitchBendSemitones = semitones; }

    /// Master tune offset in cents
    void setMasterTune (float cents)         { masterTuneCents = cents; }

    /// Whether VRC6 expansion channels are available for routing
    void setVrc6Available (bool avail)       { vrc6Available = avail; }

    /// Channel enabled state (for Auto mode routing)
    void setChannelEnabled (int nesChannel, bool en) { channelEnabled[nesChannel] = en; }

    /// Process a single MIDI message
    void processMidiMessage (const juce::MidiMessage& msg);

    /// Apply a pitch multiplier to all active voices (for LFO vibrato)
    void applyPitchMultiplier (float multiplier);

    // Portamento control
    void setPortamentoEnabled (bool en);
    void setPortamentoTime (float seconds);
    void setSampleRateForPorta (double sr);
    void tickPortamento();  // call per-sample

    /// Clear all active notes (call on preset change)
    void handleAllNotesOff();

private:
    // Portamento for each melodic channel
    Portamento porta[8];

    void handleNoteOn (int channel, int note, float velocity);
    void handleNoteOff (int channel, int note);
    void handlePitchBend (int channel, int bendValue);

    float getAdjustedFrequency (int note, float bendSemitones = 0.0f) const;

    Apu*   apuPtr              = nullptr;
    MidiMode  mode                = MidiMode::Split;
    bool      isNtsc              = true;
    bool      vrc6Available       = false;
    float     velocitySens        = 0.0f;
    int       pitchBendSemitones  = 2;
    float     masterTuneCents     = 0.0f;

    // Per-channel state for pitch bend
    float     pitchBendValues[16] = {};   // Normalized -1 to 1
    // Channels: 0=P1, 1=P2, 2=Tri, 3=Noise, 4=DPCM, 5=VRC6P1, 6=VRC6P2, 7=VRC6Saw
    int       activeNotes[8]      = { -1, -1, -1, -1, -1, -1, -1, -1 };
    bool      channelEnabled[8]   = { true, false, false, false, false, false, false, false };
};

} // namespace cart
