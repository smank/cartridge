#include "StatusBarComponent.h"
#include "PluginProcessor.h"

namespace cart {

StatusBarComponent::StatusBarComponent (CartridgeProcessor& processor)
    : processorRef (processor)
{
    setOpaque (true);

    holdToggle.setButtonText ("HOLD");
    holdToggle.setColour (juce::ToggleButton::textColourId, Colors::textSecondary);
    holdToggle.setColour (juce::ToggleButton::tickColourId, Colors::fxBright);
    holdToggle.onClick = [this]
    {
        processorRef.setHoldMode (holdToggle.getToggleState());
    };
    addAndMakeVisible (holdToggle);

    versionLabel.setText ("by smank  |  v" JucePlugin_VersionString, juce::dontSendNotification);
    versionLabel.setFont (juce::FontOptions (13.0f));
    versionLabel.setColour (juce::Label::textColourId, Colors::textDark);
    versionLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (versionLabel);
}

void StatusBarComponent::resized()
{
    auto area = getLocalBounds().reduced (4, 0);
    versionLabel.setBounds (area.removeFromRight (140));
    holdToggle.setBounds (area.removeFromRight (70));
}

void StatusBarComponent::paint (juce::Graphics& g)
{
    auto area = getLocalBounds();

    g.setColour (Colors::bgMid);
    g.fillRect (area);

    g.setColour (Colors::divider);
    g.fillRect (area.removeFromTop (1));

    auto font = juce::Font (juce::FontOptions (13.0f));
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

    drawRun ("Ch: ",  Colors::textSecondary);
    drawRun (channelNames[currentChannel], Colors::textPrimary);
    drawRun ("   |   ", Colors::textSecondary);

    drawRun ("Vel: ", Colors::textSecondary);
    drawRun (juce::String (juce::roundToInt (currentVelocity * 100.0f)) + "%", Colors::textPrimary);
    drawRun ("   |   ", Colors::textSecondary);

    drawRun ("Oct: ", Colors::textSecondary);
    drawRun ("C" + juce::String (4 + currentOctOffset), Colors::textPrimary);

    if (processorRef.getSustainPedal())
    {
        drawRun ("   |   ", Colors::textSecondary);
        drawRun ("SUS", Colors::orangeAccent);
    }
}

void StatusBarComponent::update (int channelIndex, float velocity,
                                  int octaveOffset, bool holdMode)
{
    bool sustain = processorRef.getSustainPedal();
    bool changed = channelIndex != currentChannel
                || std::abs (velocity - currentVelocity) > 0.001f
                || octaveOffset != currentOctOffset
                || holdMode != currentHold
                || sustain != currentSustain;

    if (! changed)
        return;

    currentChannel   = juce::jlimit (0, 7, channelIndex);
    currentVelocity  = velocity;
    currentOctOffset = octaveOffset;
    currentHold      = holdMode;
    currentSustain   = sustain;

    holdToggle.setToggleState (holdMode, juce::dontSendNotification);
    repaint();
}

} // namespace cart
