#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "Theme.h"

namespace cart {

/// UI panel shown when Engine Mode = Modern.
/// Replaces the channel strip area with Modern engine controls.
class ModernPanelComponent : public juce::Component
{
public:
    explicit ModernPanelComponent (juce::AudioProcessorValueTreeState& apvts);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void styleKnob (juce::Slider& knob);
    void makeLabel (juce::Label& label, const juce::String& text);

    // Osc A
    juce::ToggleButton oscAToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> oscAToggleAttach;
    juce::ComboBox waveformCombo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveformAttach;
    juce::Label waveformLabel;

    // Osc B
    juce::ToggleButton oscBToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> oscBToggleAttach;
    juce::ComboBox waveformBCombo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveformBAttach;
    juce::Label waveformBLabel;
    juce::Slider oscBLevelKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> oscBLevelAttach;
    juce::Label oscBLevelLabel;
    juce::Slider oscBDetuneKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> oscBDetuneAttach;
    juce::Label oscBDetuneLabel;

    // Voices
    juce::Slider voicesKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> voicesAttach;
    juce::Label voicesLabel;

    // ADSR
    juce::Slider attackKnob, decayKnob, sustainKnob, releaseKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttach, decayAttach, sustainAttach, releaseAttach;
    juce::Label attackLabel, decayLabel, sustainLabel, releaseLabel;

    // Unison
    juce::Slider unisonKnob, detuneKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> unisonAttach, detuneAttach;
    juce::Label unisonLabel, detuneLabel;

    // Portamento
    juce::ToggleButton portaToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> portaToggleAttach;
    juce::Slider portaTimeKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> portaTimeAttach;
    juce::Label portaTimeLabel;

    // Volume
    juce::Slider volumeKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttach;
    juce::Label volumeLabel;

    // Vel→Filter
    juce::Slider velFilterKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> velFilterAttach;
    juce::Label velFilterLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModernPanelComponent)
};

} // namespace cart
