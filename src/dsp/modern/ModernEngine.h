#pragma once

#include "ModernVoice.h"

namespace cart {

/// Voice pool manager + mixer for the Modern polyphonic engine.
class ModernEngine
{
public:
    void setSampleRate (double sr);
    void reset();
    void setMaxVoices (int n);
    void setWaveform (NesWaveform w);
    void setWaveformB (NesWaveform w);
    void setOscAEnabled (bool en);
    void setOscBEnabled (bool en);
    void setOscBLevel (float level);
    void setOscBDetune (float semitones);
    void setUnisonCount (int n);
    void setUnisonDetune (float cents);
    void setAdsr (float a, float d, float s, float r);
    void setPortamento (bool en, float time);
    void setMasterVolume (float vol);

    /// Returns voice index assigned, or -1 if no voice available.
    int  noteOn (int note, float velocity, float freqHz);
    void noteOff (int note);
    void noteOffByIndex (int voiceIndex);
    void allNotesOff();

    float process();

    /// For external pitch modulation (pitch bend)
    void setPitchBendMultiplier (float mult);

    /// For LFO vibrato
    void applyPitchMultiplier (float mult);

    // MPE per-voice methods
    void setPerVoicePitchBend (int voiceIndex, float semitones);
    void setPerVoiceSlide (int voiceIndex, float value01);
    void setPerVoicePressure (int voiceIndex, float value01);

    /// Store MIDI channel for a voice (for MPE routing)
    void setVoiceMidiChannel (int voiceIndex, int midiChannel);
    int  getVoiceMidiChannel (int voiceIndex) const;

    /// Find voice by MIDI channel (for MPE per-note routing)
    int findVoiceForChannel (int midiChannel) const;

    /// Find voice by note and channel (for MPE note-off)
    int findVoiceForNoteAndChannel (int note, int midiChannel) const;

    static constexpr int getMaxVoiceCount() { return maxVoiceCount; }

private:
    int findVoiceForNote (int note) const;
    int findFreeVoice() const;
    int findOldestReleasingVoice() const;
    int findOldestActiveVoice() const;

    static constexpr int maxVoiceCount = 16;

    ModernVoice voices[maxVoiceCount];
    int         voiceMidiChannel[maxVoiceCount] = {}; // MIDI channel per voice (for MPE)
    int         maxVoices    = 8;
    float       masterVolume = 0.8f;
    float       pitchBendMult = 1.0f;
    double      sampleRate   = 44100.0;

    // Cached parameters for new voice setup
    NesWaveform cachedWaveform  = NesWaveform::Pulse50;
    NesWaveform cachedWaveformB = NesWaveform::Pulse50;
    bool        cachedOscAEn    = true;
    bool        cachedOscBEn    = false;
    float       cachedOscBLevel = 0.8f;
    float       cachedOscBDetune = 0.0f;
    int         cachedUnison   = 1;
    float       cachedDetune   = 0.0f;
    float       cachedAttack   = 0.005f;
    float       cachedDecay    = 0.1f;
    float       cachedSustain  = 1.0f;
    float       cachedRelease  = 0.1f;
    bool        cachedPortaEn  = false;
    float       cachedPortaTime = 0.1f;
};

} // namespace cart
