#include "WaveformDisplay.h"
#include <cmath>

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
    if (introFramesLeft > 0)
        --introFramesLeft;
    repaint();
}

void WaveformDisplay::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background with subtle inset
    g.setColour (Colors::bgDark);
    g.fillRoundedRectangle (bounds, 3.0f);

    // Border — thin accent line at bottom for visual weight
    g.setColour (Colors::divider.withAlpha (0.4f));
    g.drawRoundedRectangle (bounds.reduced (0.5f), 3.0f, 0.5f);

    if (bounds.getWidth() < 2.0f || bounds.getHeight() < 2.0f)
        return;

    // Compute intro waveform on first paint (needs valid bounds)
    if (!introComputed)
        computeIntroWaveform();

    int numSamples = static_cast<int> (bounds.getWidth());
    displayBuffer.resize (static_cast<size_t> (numSamples));
    waveformBuffer.read (displayBuffer.data(), numSamples);

    float midY = bounds.getCentreY();
    float halfH = bounds.getHeight() * 0.45f;

    // Draw centre line
    g.setColour (Colors::textDark.withAlpha (0.2f));
    g.fillRect (bounds.getX() + 4.0f, midY, bounds.getWidth() - 8.0f, 1.0f);

    // Compute animation blend: 1.0 = full intro, 0.0 = full live
    float introAlpha = 0.0f;
    if (introFramesLeft > 0)
    {
        if (introFramesLeft > 60)
            introAlpha = 1.0f;   // First 1 second: full intro
        else
            introAlpha = static_cast<float> (introFramesLeft) / 60.0f;  // Fade over 2 seconds
    }

    // Build the displayed waveform (blend intro and live)
    juce::Path path;
    bool started = false;

    for (int i = 0; i < numSamples; ++i)
    {
        float liveSample = displayBuffer[static_cast<size_t> (i)];
        float introSample = (i < static_cast<int> (introWaveform.size()))
            ? introWaveform[static_cast<size_t> (i)]
            : 0.0f;

        float sample = introAlpha * introSample + (1.0f - introAlpha) * liveSample;

        float x = bounds.getX() + static_cast<float> (i);
        float y = midY - sample * halfH;
        y = juce::jlimit (bounds.getY(), bounds.getBottom(), y);

        if (!started)
        {
            path.startNewSubPath (x, y);
            started = true;
        }
        else
        {
            path.lineTo (x, y);
        }
    }

    // Draw waveform with glow effect
    g.setColour (Colors::accentActive.withAlpha (0.15f));
    g.strokePath (path, juce::PathStrokeType (3.5f));
    g.setColour (Colors::accentActive);
    g.strokePath (path, juce::PathStrokeType (1.5f));

    // Subtle watermark
    if (introAlpha < 1.0f)
    {
        float watermarkAlpha = (1.0f - introAlpha) * 0.06f;
        g.setColour (Colors::textDark.withAlpha (watermarkAlpha));
        g.setFont (juce::FontOptions (bounds.getHeight() * 0.30f));
        g.drawText ("CARTRIDGE", bounds.toNearestInt(), juce::Justification::centred);
    }
}

void WaveformDisplay::computeIntroWaveform()
{
    int w = getWidth();
    int h = getHeight();
    if (w <= 0 || h <= 0) return;

    introWaveform.resize (static_cast<size_t> (w), 0.0f);

    // Render "CARTRIDGE" to a temporary image to get a pixel mask
    juce::Image textImg (juce::Image::SingleChannel, w, h, true);
    {
        juce::Graphics ig (textImg);
        ig.setColour (juce::Colours::white);
        ig.setFont (juce::FontOptions (static_cast<float> (h) * 0.55f));
        ig.drawText ("CARTRIDGE", 0, 0, w, h, juce::Justification::centred);
    }

    // Build oscillating waveform modulated by text envelope
    for (int x = 0; x < w; ++x)
    {
        bool insideText = false;
        for (int y = 0; y < h && !insideText; y += 2)
        {
            if (textImg.getPixelAt (x, y).getAlpha() > 64)
                insideText = true;
        }

        float t = static_cast<float> (x) * 0.25f;
        introWaveform[static_cast<size_t> (x)] = insideText
            ? std::sin (t) * 0.65f
            : 0.0f;
    }

    introComputed = true;
}

} // namespace cart
