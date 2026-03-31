#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "Colors.h"

namespace cart {

enum class ChannelType
{
    Pulse1,
    Pulse2,
    Triangle,
    Noise,
    Dpcm,
    Vrc6Pulse1,
    Vrc6Pulse2,
    Vrc6Saw
};

class ChannelStripComponent : public juce::Component
{
public:
    ChannelStripComponent (ChannelType type,
                           juce::AudioProcessorValueTreeState& apvts);
    ~ChannelStripComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    ChannelType getChannelType() const { return channelType; }

private:
    void setupPulseControls (const juce::String& prefix,
                             juce::AudioProcessorValueTreeState& apvts);
    void setupTriangleControls (juce::AudioProcessorValueTreeState& apvts);
    void setupNoiseControls (juce::AudioProcessorValueTreeState& apvts);
    void setupDpcmControls (juce::AudioProcessorValueTreeState& apvts);
    void setupVrc6PulseControls (const juce::String& prefix,
                                 juce::AudioProcessorValueTreeState& apvts);
    void setupVrc6SawControls (juce::AudioProcessorValueTreeState& apvts);

    bool isVrc6() const;

    ChannelType channelType;

    // Name label
    juce::Label nameLabel;

    // Enable toggle
    juce::ToggleButton enableToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> enableAttach;

    // Main control (duty combo, mode combo, rate knob, etc.)
    juce::ComboBox mainCombo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> mainComboAttach;

    juce::Slider mainKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mainKnobAttach;

    juce::Label mainLabel; // "Fixed" label for triangle/vrc6saw

    // Volume knob
    juce::Slider volumeKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttach;

    juce::Label volumeLabel; // "Fixed" for triangle

    // Details toggle button
    juce::TextButton detailsButton { "Details" };
    bool detailsVisible = false;

    // Detail controls (envelope, sweep, linear counter)
    juce::ToggleButton constVolToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> constVolAttach;

    juce::ToggleButton envLoopToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> envLoopAttach;

    juce::ToggleButton sweepEnableToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> sweepEnableAttach;

    juce::Slider sweepPeriodKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sweepPeriodAttach;

    juce::ToggleButton sweepNegateToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> sweepNegateAttach;

    juce::Slider sweepShiftKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sweepShiftAttach;

    // Triangle linear counter
    juce::Slider linearReloadKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> linearReloadAttach;

    juce::ToggleButton linearControlToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> linearControlAttach;

    // DPCM loop
    juce::ToggleButton dpcmLoopToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> dpcmLoopAttach;

    // Noise period knob (shown alongside mode combo)
    juce::Slider noisePeriodKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> noisePeriodAttach;

    // Transpose knob (melodic channels only)
    juce::Slider transposeKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> transposeAttach;

    // Mix fader
    juce::Slider mixFader;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttach;

    bool hasDetails = false;
    bool hasVolume = true;
    bool hasMainCombo = false;
    bool hasMainKnob = false;
    bool hasTranspose = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChannelStripComponent)
};

} // namespace cart
