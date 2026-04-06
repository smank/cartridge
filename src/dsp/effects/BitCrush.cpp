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

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    const float levels = cachedLevels;

    // Process channel 0 with hold counter (drives sample-rate reduction)
    auto* data0 = buffer.getWritePointer (0);
    for (int i = 0; i < numSamples; ++i)
    {
        holdCounter += 1.0f;

        if (holdCounter >= rateReduce)
        {
            holdCounter -= rateReduce;
            holdSample = std::round (data0[i] * levels) / levels;
        }

        data0[i] = data0[i] + mix * (holdSample - data0[i]);
    }

    // Process additional channels (bit-crush only, no rate reduction state)
    for (int ch = 1; ch < numChannels; ++ch)
    {
        auto* data = buffer.getWritePointer (ch);
        for (int i = 0; i < numSamples; ++i)
        {
            float crushed = std::round (data[i] * levels) / levels;
            data[i] = data[i] + mix * (crushed - data[i]);
        }
    }
}

} // namespace cart
