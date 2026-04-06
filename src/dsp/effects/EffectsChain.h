#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "BitCrush.h"
#include "Filter.h"
#include "Chorus.h"
#include "Delay.h"
#include "Reverb.h"

namespace cart {

class EffectsChain
{
public:
    void prepare (double sampleRate, int maxBlockSize);
    void reset();

    void updateFromParams (juce::AudioProcessorValueTreeState& apvts);

    /// Process the buffer: mono effects on ch0, then mono→stereo, then stereo effects
    void process (juce::AudioBuffer<float>& buffer);

    /// Set true when input is already stereo (per-channel panning active)
    void setStereoInput (bool stereo) { stereoInput = stereo; }

    BitCrush&  getBitCrush()  { return bitCrush; }
    Filter& getFilter()    { return filter; }
    Chorus& getChorus()    { return chorus; }
    Delay&  getDelay()     { return delay; }
    Reverb& getReverb()    { return reverb; }

private:
    BitCrush  bitCrush;
    Filter filter;
    Chorus chorus;
    Delay  delay;
    Reverb reverb;
    bool   stereoInput = false;
};

} // namespace cart
