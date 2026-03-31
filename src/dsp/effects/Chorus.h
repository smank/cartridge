#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

namespace cart {

/// Custom stereo chorus with phase-offset LFOs for proper stereo spread.
/// JUCE's built-in chorus applies the same LFO phase to both channels,
/// which sounds like vibrato on simple NES waveforms. This implementation
/// uses two delay taps with 90-degree phase offset for true chorus effect.
class Chorus
{
public:
    void prepare (double sampleRate, int maxBlockSize);
    void reset();

    void setEnabled (bool on)    { enabled = on; }
    void setRate (float hz)      { rate = hz; }
    void setDepth (float d)      { depth = d; }
    void setMix (float m)        { mix = m; }

    void process (juce::AudioBuffer<float>& buffer);

private:
    bool  enabled    = false;
    float rate       = 0.5f;
    float depth      = 0.25f;
    float mix        = 0.3f;

    double sampleRate = 44100.0;

    // LFO state (two phases, 90 degrees apart)
    double lfoPhase = 0.0;

    // Delay buffer (circular) — one per channel
    static constexpr int maxDelaySize = 4096;
    float delayBufferL[maxDelaySize] = {};
    float delayBufferR[maxDelaySize] = {};
    int writePos = 0;

    // Centre delay in samples (7ms)
    static constexpr float centreDelayMs = 7.0f;

    float readInterpolated (const float* buffer, float delaySamples) const;
};

} // namespace cart
