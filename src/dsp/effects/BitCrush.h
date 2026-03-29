#pragma once

#include <juce_dsp/juce_dsp.h>
#include <cmath>

namespace cart {

class BitCrush
{
public:
    void prepare (double sampleRate, int maxBlockSize);
    void reset();

    void setEnabled (bool on)       { enabled = on; }
    void setBitDepth (float bits)   { bitDepth = bits; cachedLevels = std::pow (2.0f, bits); }
    void setRateReduce (float rate) { rateReduce = rate; }
    void setMix (float m)           { mix = m; }

    void process (juce::AudioBuffer<float>& buffer);

private:
    bool  enabled    = false;
    float bitDepth   = 16.0f;
    float rateReduce = 1.0f;
    float mix        = 1.0f;

    float holdSample = 0.0f;
    float holdCounter = 0.0f;
    float cachedLevels = 65536.0f;
};

} // namespace cart
