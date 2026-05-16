#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../dsp/WaveformBuffer.h"
#include "Theme.h"

namespace cart {

class WaveformDisplay : public juce::Component, private juce::Timer
{
public:
    explicit WaveformDisplay (WaveformBuffer& buffer);
    ~WaveformDisplay() override;

    void paint (juce::Graphics&) override;
    void timerCallback() override;

private:
    WaveformBuffer& waveformBuffer;
    std::vector<float> displayBuffer;

    // Launch animation
    std::vector<float> introWaveform;
    int introFramesLeft = 90;  // 3 seconds at 30fps
    bool introComputed = false;
    void computeIntroWaveform();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformDisplay)
};

} // namespace cart
