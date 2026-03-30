#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"
#include "ui/TopBarComponent.h"
#include "ui/ChannelStripArea.h"
#include "ui/EffectsBarComponent.h"
#include "ui/ModulationBarComponent.h"
#include "ui/StatusBarComponent.h"
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
    cart::ChannelStripArea channelStrips;
    cart::EffectsBarComponent effectsBar;
    cart::ModulationBarComponent modulationBar;
    cart::StatusBarComponent statusBar;
    juce::MidiKeyboardComponent keyboard;

    int currentOctaveOffset = 0;
    float currentVelocity = 0.8f;

    static constexpr int topBarHeight = 40;
    static constexpr int statusBarHeight = 28;
    static constexpr int defaultWidth = 900;
    static constexpr int defaultHeight = 700;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CartridgeEditor)
};
