#include "BitCrush.h"

namespace cart {

void BitCrush::prepare (double /*sampleRate*/, int /*maxBlockSize*/)
{
    reset();
}

void BitCrush::reset()
{
    holdSample  = 0.0f;
    holdCounter = 0.0f;
}

void BitCrush::process (juce::AudioBuffer<float>& buffer)
{
    if (! enabled)
        return;

    auto* data = buffer.getWritePointer (0);
    const int numSamples = buffer.getNumSamples();

    const float levels = cachedLevels;

    for (int i = 0; i < numSamples; ++i)
    {
        holdCounter += 1.0f;

        if (holdCounter >= rateReduce)
        {
            holdCounter -= rateReduce;
            // Quantize to bit depth
            holdSample = std::round (data[i] * levels) / levels;
        }

        data[i] = data[i] + mix * (holdSample - data[i]);
    }
}

} // namespace cart
