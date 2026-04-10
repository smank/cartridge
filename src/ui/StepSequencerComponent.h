#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../dsp/StepSequencer.h"
#include "Colors.h"
#include <functional>
#include <atomic>

class CartridgeProcessor;

namespace cart {

class StepSequencerComponent : public juce::Component,
                               private juce::Timer
{
public:
    explicit StepSequencerComponent (CartridgeProcessor& proc);
    ~StepSequencerComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;

private:
    void timerCallback() override;
    void editStep (const juce::MouseEvent& e);
    void clearAllSteps();

    enum Lane { LANE_VOLUME = 0, LANE_PITCH, LANE_DUTY, NUM_LANES };

    CartridgeProcessor& processor;

    int selectedChannel = 0;
    int selectedLane = LANE_VOLUME;

    juce::TextButton channelButtons[8];
    juce::TextButton laneButtons[3];
    juce::ToggleButton loopToggle;
    juce::TextButton clearButton;

    // Step count: simple label + two buttons
    juce::Label stepCountDisplay;
    juce::TextButton stepCountMinus;
    juce::TextButton stepCountPlus;

    juce::Rectangle<int> getGridArea() const;
    juce::Rectangle<int> getCellRect (int step) const;

    int getStepCount() const;
    int getStepValue (int step) const;
    void setStepValue (int step, int value);
    int getMinValue() const;
    int getMaxValue() const;
    bool isLooping() const;
    void setLooping (bool loop);
    void setStepCount (int count);

    void notifyDataChanged();

    // Track last playback step to reduce repaints
    int lastPlayStep = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StepSequencerComponent)
};

} // namespace cart
