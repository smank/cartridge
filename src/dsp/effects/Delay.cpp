#include "Delay.h"

namespace cart {

void Delay::prepare (double sampleRate, int maxBlockSize)
{
    currentSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (maxBlockSize);
    spec.numChannels      = 1;

    delayL.prepare (spec);
    delayR.prepare (spec);

    auto coeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, 8000.0f);
    fbFilterL.coefficients = coeffs;
    fbFilterR.coefficients = coeffs;
    fbFilterL.prepare (spec);
    fbFilterR.prepare (spec);

    reset();
}

void Delay::reset()
{
    delayL.reset();
    delayR.reset();
    fbFilterL.reset();
    fbFilterR.reset();
}

void Delay::process (juce::AudioBuffer<float>& buffer)
{
    if (! enabled)
        return;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    const float delaySamples = static_cast<float> (delayTimeMs * 0.001 * currentSampleRate);

    auto* dataL = buffer.getWritePointer (0);
    auto* dataR = numChannels > 1 ? buffer.getWritePointer (1) : nullptr;

    delayL.setDelay (delaySamples);
    delayR.setDelay (delaySamples);

    for (int i = 0; i < numSamples; ++i)
    {
        // Read from delay lines
        float wetL = delayL.popSample (0);
        float wetR = delayR.popSample (0);

        // Ping-pong: feed L's wet into R's delay and vice versa
        float fbL = fbFilterL.processSample (wetL);
        float fbR = fbFilterR.processSample (wetR);

        // Push input + cross-feedback into delay lines
        delayL.pushSample (0, dataL[i] + fbR * feedback);
        delayR.pushSample (0, (dataR ? dataR[i] : dataL[i]) + fbL * feedback);

        // Mix dry/wet
        dataL[i] = dataL[i] + wetL * mix;
        if (dataR)
            dataR[i] = dataR[i] + wetR * mix;
    }
}

} // namespace cart
