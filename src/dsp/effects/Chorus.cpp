#include "Chorus.h"

namespace cart {

void Chorus::prepare (double sr, int /*maxBlockSize*/)
{
    sampleRate = sr;
    reset();
}

void Chorus::reset()
{
    std::memset (delayBufferL, 0, sizeof (delayBufferL));
    std::memset (delayBufferR, 0, sizeof (delayBufferR));
    writePos = 0;
    lfoPhase = 0.0;
}

float Chorus::readInterpolated (const float* buffer, float delaySamples) const
{
    float readPos = static_cast<float> (writePos) - delaySamples;
    if (readPos < 0.0f)
        readPos += static_cast<float> (maxDelaySize);

    int idx0 = static_cast<int> (readPos);
    int idx1 = idx0 + 1;
    if (idx1 >= maxDelaySize)
        idx1 -= maxDelaySize;

    float frac = readPos - static_cast<float> (idx0);
    return buffer[idx0] + frac * (buffer[idx1] - buffer[idx0]);
}

void Chorus::process (juce::AudioBuffer<float>& buffer)
{
    if (! enabled || buffer.getNumChannels() < 2)
        return;

    const int numSamples = buffer.getNumSamples();
    auto* dataL = buffer.getWritePointer (0);
    auto* dataR = buffer.getWritePointer (1);

    const float centreDelaySamples = centreDelayMs * 0.001f * static_cast<float> (sampleRate);
    // depth controls modulation range in ms (0-1 maps to 0-3ms swing)
    const float maxSwingSamples = depth * 3.0f * 0.001f * static_cast<float> (sampleRate);
    const double phaseInc = rate / sampleRate;
    const float wet = mix;
    const float dry = 1.0f - mix;

    for (int i = 0; i < numSamples; ++i)
    {
        const float inL = dataL[i];
        const float inR = dataR[i];

        // Write dry input into delay buffers
        delayBufferL[writePos] = inL;
        delayBufferR[writePos] = inR;

        // Two LFO phases, 90 degrees apart for stereo spread
        float lfo0 = std::sin (static_cast<float> (lfoPhase * 2.0 * juce::MathConstants<double>::pi));
        float lfo1 = std::sin (static_cast<float> ((lfoPhase + 0.25) * 2.0 * juce::MathConstants<double>::pi));

        // Compute delay times
        float delayL = centreDelaySamples + lfo0 * maxSwingSamples;
        float delayR = centreDelaySamples + lfo1 * maxSwingSamples;

        // Clamp to safe range
        delayL = juce::jlimit (1.0f, static_cast<float> (maxDelaySize - 2), delayL);
        delayR = juce::jlimit (1.0f, static_cast<float> (maxDelaySize - 2), delayR);

        // Read from delay lines with interpolation
        float wetL = readInterpolated (delayBufferL, delayL);
        float wetR = readInterpolated (delayBufferR, delayR);

        // Mix dry + wet
        dataL[i] = dry * inL + wet * wetL;
        dataR[i] = dry * inR + wet * wetR;

        // Advance write position and LFO
        writePos = (writePos + 1) % maxDelaySize;
        lfoPhase += phaseInc;
        if (lfoPhase >= 1.0)
            lfoPhase -= 1.0;
    }
}

} // namespace cart
