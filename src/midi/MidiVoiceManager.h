#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "../dsp/Apu.h"
#include "../dsp/Portamento.h"
#include "TuningTable.h"

namespace cart {

/// MIDI routing modes
enum class MidiMode
{
    Split,   // Fixed MIDI channel → APU channel mapping
    Auto,    // Round-robin across melodic channels
    Mono,    // Single channel mode
    Layer    // All enabled channels play every note (thick chiptune stacks)
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

    /// Per-channel transpose in semitones (melodic channels only)
    void setTranspose (int nesChannel, float semitones) { transpose[nesChannel] = semitones; }

    /// Set the fallback DPCM sample index (used when note mapping returns -1)
    void setDpcmSample (int idx) { dpcmSampleParam = idx; }

    /// Set the tuning table for microtuning support
    void setTuningTable (TuningTable* table) { tuningTablePtr = table; }

    /// Set step sequencer pitch offset for a channel (semitones)
    void setSeqPitchOffset (int ch, float semitones) { if (ch >= 0 && ch < 8) seqPitchOffset[ch] = semitones; }

    /// Process a single MIDI message
    void processMidiMessage (const juce::MidiMessage& msg);

    /// Apply a pitch multiplier to all active voices (for LFO vibrato)
    void applyPitchMultiplier (float multiplier);

    /// Recompute frequency for a single channel (used by step sequencer pitch)
    void recomputeFrequency (int nesChannel);

    // Portamento control
    void setPortamentoEnabled (bool en);
    void setPortamentoTime (float seconds);
    void setSampleRateForPorta (double sr);
    void tickPortamento();  // call per-sample

    /// Clear all active notes (call on preset change)
    void handleAllNotesOff();

    /// Check if a channel is currently playing a note
    bool isChannelActive (int ch) const { return ch >= 0 && ch < 8 && activeNotes[ch] >= 0; }

    /// Callback for MIDI CC messages (context, ccNumber, value01)
    void (*onControlChange)(void* ctx, int cc, float val) = nullptr;
    void* ccContext = nullptr;

    /// Callback for note gate events (context, channel, noteOn)
    void (*onNoteGate)(void* ctx, int ch, bool on) = nullptr;
    void* gateContext = nullptr;

private:
    // Portamento for each melodic channel
    Portamento porta[8];

    void handleNoteOn (int channel, int note, float velocity);
    void handleNoteOff (int channel, int note);
    void handlePitchBend (int channel, int bendValue);

    float getAdjustedFrequency (int note, float bendSemitones = 0.0f, int nesChannel = -1) const;

    Apu*   apuPtr              = nullptr;
    MidiMode  mode                = MidiMode::Split;
    bool      isNtsc              = true;
    bool      vrc6Available       = false;
    float     velocitySens        = 0.0f;
    int       pitchBendSemitones  = 2;
    float     masterTuneCents     = 0.0f;

    float     transpose[8]        = {};   // Semitones per NES channel
    float     seqPitchOffset[8]   = {};   // Step sequencer pitch offset (semitones)
    int       dpcmSampleParam     = 0;    // Fallback DPCM sample index
    TuningTable* tuningTablePtr   = nullptr;

    // Per-channel state for pitch bend
    float     pitchBendValues[16] = {};   // Normalized -1 to 1
    // Channels: 0=P1, 1=P2, 2=Tri, 3=Noise, 4=DPCM, 5=VRC6P1, 6=VRC6P2, 7=VRC6Saw
    int       activeNotes[8]      = { -1, -1, -1, -1, -1, -1, -1, -1 };
    bool      channelEnabled[8]   = { true, false, false, false, false, false, false, false };
    uint32_t  noteAge[8]          = {};   // Monotonic counter for oldest-note stealing
    uint32_t  noteAgeCounter      = 0;    // Global age counter

    void noteOnChannel (int nesChannel, int note, float vol, float bendSemitones);
    void noteOffChannel (int nesChannel);
};

} // namespace cart
