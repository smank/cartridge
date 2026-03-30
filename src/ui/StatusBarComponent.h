#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Colors.h"

namespace cart {

class StatusBarComponent : public juce::Component
{
public:
    StatusBarComponent();

    void paint (juce::Graphics&) override;

    void update (int channelIndex, float velocity, int octaveOffset, bool holdMode);

private:
    int    currentChannel     = 0;
    float  currentVelocity    = 0.8f;
    int    currentOctOffset   = 0;
    bool   currentHold        = false;

    static constexpr const char* channelNames[] = {
        "Pulse 1", "Pulse 2", "Triangle", "Noise",
        "DPCM", "VRC6 P1", "VRC6 P2", "VRC6 Saw"
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StatusBarComponent)
};

} // namespace cart
