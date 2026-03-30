#include "StatusBarComponent.h"

namespace cart {

StatusBarComponent::StatusBarComponent()
{
    setOpaque (true);
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

    auto textWidth = [&] (const juce::String& text)
    {
        juce::GlyphArrangement ga;
        ga.addLineOfText (font, text, 0.0f, 0.0f);
        return juce::roundToInt (ga.getBoundingBox (0, -1, false).getWidth());
    };

    int x = 12;
    int y = area.getY();
    int h = area.getHeight();

    auto drawLabel = [&] (const juce::String& label, const juce::String& value)
    {
        g.setColour (Colors::textSecondary);
        auto labelWidth = textWidth (label);
        g.drawText (label, x, y, labelWidth, h, juce::Justification::centredLeft);
        x += labelWidth;

        g.setColour (Colors::textPrimary);
        auto valueWidth = textWidth (value);
        g.drawText (value, x, y, valueWidth, h, juce::Justification::centredLeft);
        x += valueWidth;

        // Separator
        g.setColour (Colors::textSecondary);
        juce::String sep ("  |  ");
        auto sepWidth = textWidth (sep);
        g.drawText (sep, x, y, sepWidth, h, juce::Justification::centredLeft);
        x += sepWidth;
    };

    // Channel
    drawLabel ("Ch: ", channelNames[currentChannel]);

    // Velocity
    int velPercent = juce::roundToInt (currentVelocity * 100.0f);
    drawLabel ("Vel: ", juce::String (velPercent) + "%");

    // Octave
    juce::String octStr = "C" + juce::String (4 + currentOctOffset);
    drawLabel ("Oct: ", octStr);

    // Hold indicator
    if (currentHold)
    {
        g.setColour (Colors::fxBright);
        g.drawText ("HOLD", x, y, 50, h, juce::Justification::centredLeft);
    }

    // Version — right-aligned
    g.setColour (Colors::textDark);
    g.drawText ("v" JucePlugin_VersionString, area.withTrimmedRight (12),
                juce::Justification::centredRight);
}

void StatusBarComponent::update (int channelIndex, float velocity,
                                  int octaveOffset, bool holdMode)
{
    bool changed = channelIndex != currentChannel
                || std::abs (velocity - currentVelocity) > 0.001f
                || octaveOffset != currentOctOffset
                || holdMode != currentHold;

    if (! changed)
        return;

    currentChannel   = juce::jlimit (0, 7, channelIndex);
    currentVelocity  = velocity;
    currentOctOffset = octaveOffset;
    currentHold      = holdMode;
    repaint();
}

} // namespace cart
