#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "../Theme.h"

namespace cart::ui
{

// APVTS-bound labeled toggle.
class ToggleControl : public juce::Component
{
public:
    ToggleControl (juce::AudioProcessorValueTreeState& apvts,
                   juce::StringRef paramID,
                   juce::String labelText)
    {
        button.setButtonText (labelText);
        addAndMakeVisible (button);
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
            apvts, paramID.text, button);
    }

    void resized() override { button.setBounds (getLocalBounds()); }

    juce::ToggleButton& getButton() noexcept { return button; }

private:
    juce::ToggleButton button;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;
};

} // namespace cart::ui
