#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "../Theme.h"

namespace cart::ui
{

// Labeled dropdown bound to an APVTS choice parameter.
class ChoiceControl : public juce::Component
{
public:
    ChoiceControl (juce::AudioProcessorValueTreeState& apvts,
                   juce::StringRef paramID,
                   juce::String labelText)
        : label (std::move (labelText))
    {
        if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (paramID.text)))
            combo.addItemList (p->choices, 1);

        addAndMakeVisible (combo);
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
            apvts, paramID.text, combo);
    }

    void paint (juce::Graphics& g) override
    {
        g.setColour (Palette::textSecondary);
        g.setFont (labelFont (11.0f));
        g.drawFittedText (label.toUpperCase(),
                          getLocalBounds().removeFromTop (14),
                          juce::Justification::centredLeft, 1);
    }

    void resized() override
    {
        auto r = getLocalBounds();
        r.removeFromTop (14);
        combo.setBounds (r.reduced (0, 2));
    }

    juce::ComboBox& getCombo() noexcept { return combo; }

private:
    juce::ComboBox combo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
    juce::String label;
};

} // namespace cart::ui
