#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"
#include "ui/TopBarComponent.h"
#include "ui/ChannelStripArea.h"
#include "ui/EffectsBarComponent.h"
#include "ui/ModulationBarComponent.h"
#include "ui/Colors.h"

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
    juce::MidiKeyboardComponent keyboard;

    int currentOctaveOffset = 0;
    float currentVelocity = 0.8f;

    static constexpr int topBarHeight = 40;
    static constexpr int keyboardHeight = 140;
    static constexpr int defaultWidth = 900;
    static constexpr int defaultHeight = 700;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CartridgeEditor)
};
