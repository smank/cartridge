#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "../Theme.h"

namespace cart::ui
{

namespace detail
{
// Slider that adds a right-click popup listing choice values by name,
// so stepped (choice/bool) knobs can be set directly without dragging.
// Behaves like juce::Slider when no choice labels are set.
class StepperSlider : public juce::Slider
{
public:
    void setChoiceLabels (juce::StringArray labels) noexcept
    {
        choiceLabels = std::move (labels);
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        if (e.mods.isPopupMenu() && choiceLabels.size() > 0)
        {
            showChoicePopup();
            return;
        }
        juce::Slider::mouseDown (e);
    }

private:
    void showChoicePopup()
    {
        juce::PopupMenu menu;
        const int currentIndex = juce::roundToInt (getValue());
        for (int i = 0; i < choiceLabels.size(); ++i)
            menu.addItem (i + 1, choiceLabels[i], true, i == currentIndex);

        auto opts = juce::PopupMenu::Options{}.withTargetComponent (this);
        menu.showMenuAsync (opts, [safe = juce::Component::SafePointer<StepperSlider> (this)]
                                  (int chosen)
        {
            if (auto* s = safe.getComponent())
                if (chosen > 0)
                    s->setValue (static_cast<double> (chosen - 1),
                                 juce::sendNotificationSync);
        });
    }

    juce::StringArray choiceLabels;
};
} // namespace detail

// Label + rotary + value readout, bound to an APVTS parameter.
class KnobControl : public juce::Component
{
public:
    KnobControl (juce::AudioProcessorValueTreeState& apvts,
                 juce::StringRef paramID,
                 juce::String labelText)
        : label (std::move (labelText))
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 64, 16);
        slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.2f,
                                    juce::MathConstants<float>::pi * 2.8f,
                                    true);
        slider.setColour (juce::Slider::textBoxTextColourId, Palette::textSecondary);
        addAndMakeVisible (slider);

        if (auto* p = apvts.getParameter (paramID.text))
        {
            auto* choice = dynamic_cast<juce::AudioParameterChoice*> (p);
            auto* boolP  = dynamic_cast<juce::AudioParameterBool*>   (p);

            if (choice != nullptr || boolP != nullptr)
            {
                slider.textFromValueFunction = [p] (double v) -> juce::String
                {
                    return p->getText (p->convertTo0to1 (static_cast<float> (v)), 16);
                };

                const int numSteps = choice != nullptr ? choice->choices.size() : 2;
                slider.setMouseDragSensitivity (juce::jmax (60, numSteps * 22));
                slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);

                juce::StringArray labels;
                if (choice != nullptr)
                    for (auto& c : choice->choices) labels.add (c);
                else
                    labels = juce::StringArray { "Off", "On" };
                slider.setChoiceLabels (labels);

                slider.setPopupDisplayEnabled (true, true, this);
                slider.setTooltip ("Drag to step • Scroll to step • Right-click for list");
            }
        }

        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            apvts, paramID.text, slider);
    }

    void paint (juce::Graphics& g) override
    {
        g.setColour (Palette::textSecondary);
        g.setFont (labelFont (11.0f));
        g.drawFittedText (label.toUpperCase(),
                          getLocalBounds().removeFromTop (14),
                          juce::Justification::centred, 1);
    }

    void resized() override
    {
        auto r = getLocalBounds();
        r.removeFromTop (14);
        slider.setBounds (r.reduced (2));
    }

    juce::Slider& getSlider() noexcept { return slider; }

private:
    detail::StepperSlider slider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    juce::String label;
};

} // namespace cart::ui
