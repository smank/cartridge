#include "StatusBarComponent.h"
#include "PluginProcessor.h"

namespace cart {

using namespace cart::ui;

StatusBarComponent::StatusBarComponent (CartridgeProcessor& processor)
    : processorRef (processor)
{
    setOpaque (true);

    holdToggle.setButtonText ("HOLD");
    holdToggle.setColour (juce::ToggleButton::textColourId, Palette::textSecondary);
    holdToggle.setColour (juce::ToggleButton::tickColourId, Palette::hotBright);
    holdToggle.onClick = [this]
    {
        processorRef.setHoldMode (holdToggle.getToggleState());
    };
    addAndMakeVisible (holdToggle);

    versionLabel.setText ("v" JucePlugin_VersionString, juce::dontSendNotification);
    versionLabel.setFont (labelFont (Metrics::fontValue));
    versionLabel.setColour (juce::Label::textColourId, Palette::textDim);
    versionLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (versionLabel);

    startTimerHz (8);
}

void StatusBarComponent::timerCallback()
{
    holdToggle.setToggleState (processorRef.getHoldMode(), juce::dontSendNotification);
    repaint();
}

void StatusBarComponent::resized()
{
    auto area = getLocalBounds().reduced (8, 0);
    versionLabel.setBounds (area.removeFromRight (80));
    holdToggle.setBounds (area.removeFromRight (60));
}

void StatusBarComponent::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();

    // Subtle vertical gradient — slightly darker than the editor body
    g.setGradientFill (juce::ColourGradient (
        Palette::background.darker (0.05f), 0.0f, (float) area.getY(),
        Palette::background.darker (0.20f), 0.0f, (float) area.getBottom(),
        false));
    g.fillRect (area);

    // Hairline divider with primary tint at top
    g.setColour (Palette::primary.withAlpha (0.35f));
    g.fillRect (area.removeFromTop (1));

    auto font = labelFont (Metrics::fontValue);
    g.setFont (font);

    float baseline = area.getY() + (area.getHeight() + font.getAscent() - font.getDescent()) * 0.5f;
    float x = 12.0f;

    auto drawRun = [&] (const juce::String& text, juce::Colour colour)
    {
        g.setColour (colour);
        juce::GlyphArrangement ga;
        ga.addLineOfText (font, text, x, baseline);
        ga.draw (g);
        x = ga.getBoundingBox (0, -1, false).getRight();
    };

    auto drawSep = [&] ()
    {
        drawRun ("  \xc2\xb7  ", Palette::textDim);  // centered dot separator
    };

    // Active voices
    int activeCount = 0;
    for (int i = 0; i < 8; ++i)
        if (processorRef.channelActive[i].load (std::memory_order_relaxed))
            ++activeCount;

    if (activeCount > 0)
    {
        drawRun (juce::String (activeCount) + " voice" + (activeCount > 1 ? "s" : ""), Palette::primary);
        drawSep();
    }

    // Engine mode
    auto* engineParam = processorRef.getApvts().getRawParameterValue (cart::ParamIDs::EngineMode);
    bool modern = engineParam && static_cast<int> (engineParam->load()) == 1;
    drawRun (modern ? "Modern" : "Classic", Palette::textPrimary);
    drawSep();

    // MIDI mode
    auto* midiParam = processorRef.getApvts().getRawParameterValue (cart::ParamIDs::MidiMode);
    if (midiParam)
    {
        static const char* modeNames[] = { "Split", "Auto", "Mono", "Layer" };
        int mode = juce::jlimit (0, 3, static_cast<int> (midiParam->load()));
        drawRun (modeNames[mode], Palette::textSecondary);
        drawSep();
    }

    // Octave
    drawRun ("C" + juce::String (4 + currentOctOffset), Palette::textPrimary);
    drawSep();

    // Velocity
    drawRun (juce::String (juce::roundToInt (currentVelocity * 100.0f)) + "%", Palette::textSecondary);

    // Sustain pedal indicator
    if (processorRef.getSustainPedal())
    {
        drawSep();
        drawRun ("SUS", Palette::vrc6Accent);
    }
}

} // namespace cart
