#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Colors.h"

class CartridgeProcessor;

namespace cart {

class StatusBarComponent : public juce::Component,
                           private juce::Timer
{
public:
    StatusBarComponent (CartridgeProcessor& processor);

    void paint (juce::Graphics&) override;
    void resized() override;

    void setVelocity (float v) { currentVelocity = v; }
    void setOctaveOffset (int o) { currentOctOffset = o; }

private:
    void timerCallback() override;

    CartridgeProcessor& processorRef;
    juce::ToggleButton holdToggle;
    juce::Label versionLabel;

    float  currentVelocity    = 0.8f;
    int    currentOctOffset   = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StatusBarComponent)
};

} // namespace cart
