#pragma once

#include <juce_dsp/juce_dsp.h>

namespace cart {

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
    bool  enabled = false;
    float rate    = 0.5f;
    float depth   = 0.25f;
    float mix     = 0.3f;

    juce::dsp::Chorus<float> chorus;
};

} // namespace cart
