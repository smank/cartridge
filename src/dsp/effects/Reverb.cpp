#include "Reverb.h"

namespace cart {

void Reverb::prepare (double sampleRate, int /*maxBlockSize*/)
{
    reverb.setSampleRate (sampleRate);
    reset();
}

void Reverb::reset()
{
    reverb.reset();
}

void Reverb::process (juce::AudioBuffer<float>& buffer)
{
    if (! enabled)
        return;

    juce::Reverb::Parameters params;
    params.roomSize   = roomSize;
    params.damping    = damping;
    params.width      = width;
    params.wetLevel   = mix;
    params.dryLevel   = 1.0f - mix;
    params.freezeMode = 0.0f;
    reverb.setParameters (params);

    if (buffer.getNumChannels() >= 2)
        reverb.processStereo (buffer.getWritePointer (0),
                              buffer.getWritePointer (1),
                              buffer.getNumSamples());
    else
        reverb.processMono (buffer.getWritePointer (0),
                            buffer.getNumSamples());
}

} // namespace cart
