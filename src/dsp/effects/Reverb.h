#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

namespace cart {

class Reverb
{
public:
    void prepare (double sampleRate, int maxBlockSize);
    void reset();

    void setEnabled (bool on)    { enabled = on; }
    void setSize (float s)       { roomSize = s; }
    void setDamping (float d)    { damping = d; }
    void setWidth (float w)      { width = w; }
    void setMix (float m)        { mix = m; }

    void process (juce::AudioBuffer<float>& buffer);

private:
    bool  enabled  = false;
    float roomSize = 0.4f;
    float damping  = 0.5f;
    float width    = 0.8f;
    float mix      = 0.2f;

    juce::Reverb reverb;
};

} // namespace cart
