#pragma once

#include <juce_dsp/juce_dsp.h>

namespace cart {

class Delay
{
public:
    void prepare (double sampleRate, int maxBlockSize);
    void reset();

    void setEnabled (bool on)       { enabled = on; }
    void setTime (float ms)         { delayTimeMs = ms; }
    void setFeedback (float fb)     { feedback = fb; }
    void setMix (float m)           { mix = m; }

    void process (juce::AudioBuffer<float>& buffer);

private:
    bool  enabled     = false;
    float delayTimeMs = 375.0f;
    float feedback    = 0.4f;
    float mix         = 0.3f;

    double currentSampleRate = 44100.0;

    // Stereo ping-pong: separate delay lines per channel
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayL { 96000 };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayR { 96000 };

    // Feedback low-pass filter per channel
    juce::dsp::IIR::Filter<float> fbFilterL;
    juce::dsp::IIR::Filter<float> fbFilterR;
};

} // namespace cart
