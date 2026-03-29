#pragma once

#include "ApuConstants.h"
#include "channels/PulseChannel.h"
#include "channels/TriangleChannel.h"
#include "channels/NoiseChannel.h"
#include "channels/DpcmChannel.h"
#include "channels/Vrc6PulseChannel.h"
#include "channels/Vrc6SawChannel.h"
#include "mixing/ApuMixer.h"

namespace cart {

/// Top-level 2A03 APU emulation.
/// Owns all 5 base channels + optional VRC6 expansion (3 channels), frame counter, and mixer.
class Apu
{
public:
    Apu();

    void reset();
    void setSampleRate (double sr);
    void setRegion (bool ntsc);

    // Base channel access
    PulseChannel&    pulse1()    { return pulse1Ch; }
    PulseChannel&    pulse2()    { return pulse2Ch; }
    TriangleChannel& triangle()  { return triangleCh; }
    NoiseChannel&    noise()     { return noiseCh; }
    DpcmChannel&     dpcm()      { return dpcmCh; }
    ApuMixer&        mixer()     { return apuMixer; }

    // VRC6 expansion channel access
    Vrc6PulseChannel& vrc6Pulse1()  { return vrc6Pulse1Ch; }
    Vrc6PulseChannel& vrc6Pulse2()  { return vrc6Pulse2Ch; }
    Vrc6SawChannel&   vrc6Saw()     { return vrc6SawCh; }

    void setVrc6Enabled (bool en)   { vrc6Enabled = en; }
    bool isVrc6Enabled() const      { return vrc6Enabled; }

    /// Process one sample. Returns mono float in [-1, 1].
    float process();

    /// Master volume (0.0–1.0)
    void setMasterVolume (float vol)  { masterVolume = vol; }

    /// VRC6 mix levels
    void setVrc6Pulse1Mix (float lvl) { vrc6Pulse1Mix = lvl; }
    void setVrc6Pulse2Mix (float lvl) { vrc6Pulse2Mix = lvl; }
    void setVrc6SawMix (float lvl)    { vrc6SawMix = lvl; }

private:
    void clockFrameCounter();

    PulseChannel    pulse1Ch { 0 };
    PulseChannel    pulse2Ch { 1 };
    TriangleChannel triangleCh;
    NoiseChannel    noiseCh;
    DpcmChannel     dpcmCh;
    ApuMixer        apuMixer;

    // VRC6 expansion
    Vrc6PulseChannel vrc6Pulse1Ch;
    Vrc6PulseChannel vrc6Pulse2Ch;
    Vrc6SawChannel   vrc6SawCh;
    bool  vrc6Enabled   = false;
    float vrc6Pulse1Mix = 1.0f;
    float vrc6Pulse2Mix = 1.0f;
    float vrc6SawMix    = 1.0f;

    double sampleRate   = 44100.0;
    bool   isNtsc       = true;
    float  masterVolume = 0.8f;

    // Frame counter (phase accumulator approach)
    double framePhase     = 0.0;
    double framePhaseInc  = 0.0;   // quarterFrameRate / sampleRate
    int    frameStep      = 0;     // 0–3 for 4-step mode
};

} // namespace cart
