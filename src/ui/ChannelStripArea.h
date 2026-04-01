#pragma once

#include <atomic>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "ChannelStripComponent.h"

namespace cart {

class ChannelStripArea : public juce::Component
{
public:
    ChannelStripArea (juce::AudioProcessorValueTreeState& apvts);
    ~ChannelStripArea() override;

    /// Wire activity flags from processor (call once after construction)
    void setActivityFlags (std::atomic<bool>* flags);

    void paint (juce::Graphics&) override;
    void resized() override;

    void setVrc6Visible (bool visible);
    bool isVrc6Visible() const { return vrc6Visible; }
    int getFocusedChannel() const { return focusedChannel; }
    void setFocusedChannel (int ch) { focusedChannel = ch; }

private:
    // APU channels (always present)
    ChannelStripComponent pulse1Strip;
    ChannelStripComponent pulse2Strip;
    ChannelStripComponent triangleStrip;
    ChannelStripComponent noiseStrip;
    ChannelStripComponent dpcmStrip;

    // VRC6 channels
    ChannelStripComponent vrc6Pulse1Strip;
    ChannelStripComponent vrc6Pulse2Strip;
    ChannelStripComponent vrc6SawStrip;

    bool vrc6Visible = false;
    int focusedChannel = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChannelStripArea)
};

} // namespace cart
