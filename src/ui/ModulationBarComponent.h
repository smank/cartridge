#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "Colors.h"

namespace cart {

class ModulationBarComponent : public juce::Component,
                               private juce::Timer
{
public:
    ModulationBarComponent (juce::AudioProcessorValueTreeState& apvts);
    ~ModulationBarComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void timerCallback() override;

    int getDesiredHeight() const;
    void collapseAll();

    std::function<void()> onHeightChanged;
    std::function<void(const juce::File&)> onDpcmLoad;

    static constexpr int headerHeight = 32;
    static constexpr int detailHeight = 100;

private:
    enum ModIndex { MOD_LFO = 0, MOD_PORTA, MOD_ARP, MOD_DPCM, NUM_MOD };

    void toggleExpand (int modIndex);
    void styleKnob (juce::Slider& knob);
    void layoutDetailKnobs (juce::Rectangle<int> area, int modIndex);
    void setDetailVisible (int modIndex, bool visible);

    int expandedMod = -1;

    // ─── LFO ────────────────────────────────────────────────────────────
    juce::ToggleButton lfoEnable;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> lfoEnableAttach;

    juce::Slider lfoRate, lfoVibrato, lfoTremolo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfoRateAttach, lfoVibratoAttach, lfoTremoloAttach;
    juce::Label lfoRateLabel, lfoVibratoLabel, lfoTremoloLabel;

    // ─── Portamento ─────────────────────────────────────────────────────
    juce::ToggleButton portaEnable;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> portaEnableAttach;

    juce::Slider portaTime;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> portaTimeAttach;
    juce::Label portaTimeLabel;

    // ─── Arpeggiator ────────────────────────────────────────────────────
    juce::ToggleButton arpEnable;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> arpEnableAttach;

    juce::ComboBox arpPattern;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> arpPatternAttach;
    juce::Label arpPatternLabel;

    juce::Slider arpRate, arpOctaves, arpGate;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> arpRateAttach, arpOctavesAttach, arpGateAttach;
    juce::Label arpRateLabel, arpOctavesLabel, arpGateLabel;

    // Arp tempo sync
    juce::ToggleButton arpSyncToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> arpSyncAttach;
    juce::ComboBox arpSyncDiv;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> arpSyncDivAttach;

    // ─── DPCM ───────────────────────────────────────────────────────────
    juce::ComboBox dpcmSample;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> dpcmSampleAttach;
    juce::Label dpcmSampleLabel;
    juce::TextButton dpcmLoadButton { "Load" };
    std::shared_ptr<juce::FileChooser> dpcmFileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModulationBarComponent)
};

} // namespace cart
