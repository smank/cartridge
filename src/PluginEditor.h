#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"
#include "ui/TopBarComponent.h"
#include "ui/ChannelStripArea.h"
#include "ui/EffectsBarComponent.h"
#include "ui/ModulationBarComponent.h"
#include "ui/StatusBarComponent.h"
#include "ui/ModernPanelComponent.h"
#include "ui/WaveformDisplay.h"
#include "ui/Colors.h"
#include "ui/FullscreenHelper.h"

class CartridgeEditor : public juce::AudioProcessorEditor
{
public:
    explicit CartridgeEditor (CartridgeProcessor&);
    ~CartridgeEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    bool keyPressed (const juce::KeyPress& key) override;
    bool keyStateChanged (bool isKeyDown) override;

private:
    [[maybe_unused]] CartridgeProcessor& processorRef;

    cart::TopBarComponent topBar;
    cart::WaveformDisplay waveformDisplay;
    cart::ChannelStripArea channelStrips;
    cart::ModernPanelComponent modernPanel;
    cart::EffectsBarComponent effectsBar;
    cart::ModulationBarComponent modulationBar;
    cart::StatusBarComponent statusBar;
    juce::MidiKeyboardComponent keyboard;
    juce::TooltipWindow tooltipWindow { this, 500 };

    bool modernModeActive = false;

    int currentOctaveOffset = 0;
    float currentVelocity = 0.8f;
    float currentScale = 1.0f;

    void applyScale (float scale);
    void loadScalePreference();
    void saveScalePreference();
    juce::File getSettingsFile() const;

    static constexpr int topBarHeight = 96;
    static constexpr int waveformHeight = 60;
    static constexpr int statusBarHeight = 28;
    static constexpr int baseWidth = 940;
    static constexpr int baseHeight = 720;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CartridgeEditor)
};
