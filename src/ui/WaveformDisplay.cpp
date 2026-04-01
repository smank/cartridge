#include "WaveformDisplay.h"

namespace cart {

WaveformDisplay::WaveformDisplay (WaveformBuffer& buffer)
    : waveformBuffer (buffer)
{
    startTimerHz (30);
}

WaveformDisplay::~WaveformDisplay()
{
    stopTimer();
}

void WaveformDisplay::timerCallback()
{
    repaint();
}

void WaveformDisplay::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour (Colors::bgDark);
    g.fillRoundedRectangle (bounds, 4.0f);

    // Subtle border
    g.setColour (Colors::divider);
    g.drawRoundedRectangle (bounds, 4.0f, 0.5f);

    if (bounds.getWidth() < 2.0f || bounds.getHeight() < 2.0f)
        return;

    int numSamples = static_cast<int> (bounds.getWidth());
    displayBuffer.resize (static_cast<size_t> (numSamples));
    waveformBuffer.read (displayBuffer.data(), numSamples);

    float midY = bounds.getCentreY();
    float halfH = bounds.getHeight() * 0.45f;

    juce::Path path;
    bool started = false;

    for (int i = 0; i < numSamples; ++i)
    {
        float x = bounds.getX() + static_cast<float> (i);
        float y = midY - displayBuffer[static_cast<size_t> (i)] * halfH;
        y = juce::jlimit (bounds.getY(), bounds.getBottom(), y);

        if (! started)
        {
            path.startNewSubPath (x, y);
            started = true;
        }
        else
        {
            path.lineTo (x, y);
        }
    }

    // Draw centre line (dim)
    g.setColour (Colors::textDark.withAlpha (0.3f));
    g.drawHorizontalLine (static_cast<int> (midY), bounds.getX(), bounds.getRight());

    // Draw waveform
    g.setColour (Colors::accentActive);
    g.strokePath (path, juce::PathStrokeType (1.2f));
}

} // namespace cart
