#include "Filter.h"

namespace cart {

void Filter::prepare (double sampleRate, int maxBlockSize)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (maxBlockSize);
    spec.numChannels      = 2;
    filter.prepare (spec);
    updateCoeffs = true;
}

void Filter::reset()
{
    filter.reset();
}

void Filter::setType (int type)
{
    using Type = juce::dsp::StateVariableTPTFilterType;
    switch (type)
    {
        case 0:  filter.setType (Type::lowpass);  break;
        case 1:  filter.setType (Type::bandpass); break;
        case 2:  filter.setType (Type::highpass); break;
        default: filter.setType (Type::lowpass);  break;
    }
}

void Filter::process (juce::AudioBuffer<float>& buffer)
{
    if (! enabled)
        return;

    if (updateCoeffs)
    {
        filter.setCutoffFrequency (cutoff);
        filter.setResonance (resonance);
        updateCoeffs = false;
    }

    auto block = juce::dsp::AudioBlock<float> (buffer);
    auto context = juce::dsp::ProcessContextReplacing<float> (block);
    filter.process (context);
}

} // namespace cart
