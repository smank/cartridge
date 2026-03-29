#include "Chorus.h"

namespace cart {

void Chorus::prepare (double sampleRate, int maxBlockSize)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (maxBlockSize);
    spec.numChannels      = 2;
    chorus.prepare (spec);
}

void Chorus::reset()
{
    chorus.reset();
}

void Chorus::process (juce::AudioBuffer<float>& buffer)
{
    if (! enabled)
        return;

    chorus.setRate (rate);
    chorus.setDepth (depth);
    chorus.setMix (mix);
    chorus.setCentreDelay (7.0f);
    chorus.setFeedback (-0.2f);

    auto block = juce::dsp::AudioBlock<float> (buffer);
    auto context = juce::dsp::ProcessContextReplacing<float> (block);
    chorus.process (context);
}

} // namespace cart
