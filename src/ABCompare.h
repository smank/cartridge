#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace cart {

class ABCompare
{
public:
    void initialize(juce::AudioProcessorValueTreeState& apvts)
    {
        stateA = apvts.copyState();
        stateB = apvts.copyState();
    }

    void toggle(juce::AudioProcessorValueTreeState& apvts)
    {
        if (showingA)
        {
            stateA = apvts.copyState();
            apvts.replaceState(stateB.createCopy());
        }
        else
        {
            stateB = apvts.copyState();
            apvts.replaceState(stateA.createCopy());
        }
        showingA = !showingA;
    }

    bool isShowingA() const { return showingA; }

private:
    juce::ValueTree stateA;
    juce::ValueTree stateB;
    bool showingA = true;
};

} // namespace cart
