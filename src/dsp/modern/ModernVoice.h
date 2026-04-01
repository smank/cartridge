#pragma once

#include "NesOscillator.h"
#include "AdsrEnvelope.h"
#include "../Portamento.h"

namespace cart {

/// One polyphonic voice: oscillator stack + ADSR + portamento.
class ModernVoice
{
public:
    void setSampleRate (double sr);
    void setWaveform (NesWaveform w);
    void setWaveformB (NesWaveform w);
    void setOscAEnabled (bool en);
    void setOscBEnabled (bool en);
    void setOscBLevel (float level);
    void setOscBDetune (float semitones);
    void setUnisonCount (int count);
    void setUnisonDetune (float cents);
    void setAdsr (float a, float d, float s, float r);
    void setPortamento (bool en, float time);

    void noteOn (int note, float velocity, float freqHz);
    void noteOff();
    float process();

    bool isActive() const     { return !envelope.isFinished(); }
    bool isReleasing() const  { return envelope.isReleasing(); }
    int  getNote() const      { return currentNote; }
    int  getAge() const       { return age; }
    void reset();

    // MPE per-voice modulation
    void setPitchBendMultiplier (float mult)  { perVoicePitchBend = mult; }
    void setMpeSlide (float value01)          { mpeSlide = value01; }   // CC74 timbre
    void setMpePressure (float value01)       { mpePressure = value01; } // channel pressure

private:
    void updateOscFrequencies();

    static constexpr int maxUnison = 7;

    NesOscillator  oscillators[maxUnison];    // Osc A
    NesOscillator  oscillatorsB[maxUnison];   // Osc B
    AdsrEnvelope   envelope;
    Portamento     porta;

    NesWaveform    waveform      = NesWaveform::Pulse50;
    NesWaveform    waveformB     = NesWaveform::Pulse50;
    bool           oscAEnabled   = true;
    bool           oscBEnabled   = false;
    float          oscBLevel     = 0.8f;
    float          oscBDetuneSemi = 0.0f;  // semitones
    double         sampleRate    = 44100.0;
    int            unisonCount   = 1;
    float          unisonDetune  = 0.0f;   // cents
    float          baseFreq      = 440.0f;
    float          velocityGain  = 1.0f;
    int            currentNote   = -1;
    int            age           = 0;

    // MPE per-voice state
    float          perVoicePitchBend = 1.0f;  // multiplier (1.0 = no bend)
    float          mpeSlide     = 0.5f;       // CC74 timbre (0-1, 0.5 = center)
    float          mpePressure  = 0.0f;       // channel pressure (0-1)
};

} // namespace cart
