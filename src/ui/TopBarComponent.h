#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "Colors.h"

// Forward declare
class CartridgeProcessor;

namespace cart {

class TopBarComponent : public juce::Component,
                        public juce::Timer
{
public:
    TopBarComponent (CartridgeProcessor& processor,
                     juce::AudioProcessorValueTreeState& apvts);
    ~TopBarComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

    std::function<void (bool)> onVrc6Toggle;
    std::function<void()> onPresetChanged;

    void navigatePreset (int delta);

private:
    CartridgeProcessor& processorRef;

    // Preset controls
    juce::ComboBox presetCombo;
    juce::TextButton prevButton { "<" };
    juce::TextButton nextButton { ">" };
    juce::TextButton saveButton { "Save" };

    // Global controls
    juce::Slider masterVolSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterVolAttach;
    juce::Label masterVolLabel;

    juce::Slider masterTuneSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterTuneAttach;
    juce::Label masterTuneLabel;

    juce::ComboBox regionCombo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> regionAttach;

    juce::ComboBox midiModeCombo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> midiModeAttach;

    juce::Slider velocitySensSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> velocitySensAttach;
    juce::Label velocitySensLabel;

    juce::Slider pitchBendSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pitchBendAttach;
    juce::Label pitchBendLabel;

    // VRC6 toggle
    juce::ToggleButton vrc6Toggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> vrc6Attach;

    void populatePresets();
    void selectPreset (int index);

    bool lastVrc6State = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TopBarComponent)
};

} // namespace cart
