#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "Colors.h"

namespace cart {

class EffectsBarComponent : public juce::Component,
                            private juce::Timer
{
public:
    EffectsBarComponent (juce::AudioProcessorValueTreeState& apvts);
    ~EffectsBarComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void timerCallback() override;

    int getDesiredHeight() const;
    void collapseAll();

    std::function<void()> onHeightChanged;

    static constexpr int headerHeight = 32;
    static constexpr int detailHeight = 100;

private:
    enum EffectIndex { FX_CRUSH = 0, FX_FILTER, FX_CHORUS, FX_DELAY, FX_REVERB, NUM_FX };

    void toggleExpand (int fxIndex);
    void styleKnob (juce::Slider& knob);
    void layoutDetailKnobs (juce::Rectangle<int> area, int fxIndex);
    void setDetailVisible (int fxIndex, bool visible);

    int expandedEffect = -1;

    // ─── BitCrush ──────────────────────────────────────────────────────
    juce::ToggleButton bcEnable;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bcEnableAttach;

    juce::Slider bcBitDepth, bcRate, bcMix;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bcBitDepthAttach, bcRateAttach, bcMixAttach;
    juce::Label bcBitDepthLabel, bcRateLabel, bcMixLabel;

    // ─── Filter ────────────────────────────────────────────────────────
    juce::ToggleButton fltEnable;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> fltEnableAttach;

    juce::ComboBox fltType;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> fltTypeAttach;
    juce::Label fltTypeLabel;

    juce::Slider fltCutoff, fltResonance;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> fltCutoffAttach, fltResonanceAttach;
    juce::Label fltCutoffLabel, fltResonanceLabel;

    // ─── Chorus ────────────────────────────────────────────────────────
    juce::ToggleButton chEnable;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> chEnableAttach;

    juce::Slider chRate, chDepth, chMix;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> chRateAttach, chDepthAttach, chMixAttach;
    juce::Label chRateLabel, chDepthLabel, chMixLabel;

    // ─── Delay ─────────────────────────────────────────────────────────
    juce::ToggleButton dlEnable;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> dlEnableAttach;

    juce::Slider dlTime, dlFeedback, dlMix;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dlTimeAttach, dlFeedbackAttach, dlMixAttach;
    juce::Label dlTimeLabel, dlFeedbackLabel, dlMixLabel;

    // Delay tempo sync
    juce::ToggleButton dlSyncToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> dlSyncAttach;
    juce::ComboBox dlSyncDiv;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> dlSyncDivAttach;

    // ─── Reverb ────────────────────────────────────────────────────────
    juce::ToggleButton rvEnable;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> rvEnableAttach;

    juce::Slider rvSize, rvDamping, rvWidth, rvMix;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rvSizeAttach, rvDampingAttach, rvWidthAttach, rvMixAttach;
    juce::Label rvSizeLabel, rvDampingLabel, rvWidthLabel, rvMixLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EffectsBarComponent)
};

} // namespace cart
