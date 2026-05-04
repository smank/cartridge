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
    using namespace cart::ui;
    auto bounds = getLocalBounds().toFloat();

    // Inset scope panel — radial gradient gives it depth like an oscilloscope tube
    juce::ColourGradient bgGrad (
        Palette::background.brighter (0.10f),
        bounds.getCentreX(), bounds.getCentreY(),
        Palette::background.darker (0.30f),
        bounds.getRight(), bounds.getBottom(),
        true);
    g.setGradientFill (bgGrad);
    g.fillRoundedRectangle (bounds, 4.0f);

    // Faint outline
    g.setColour (Palette::outlineDim.withAlpha (0.6f));
    g.drawRoundedRectangle (bounds.reduced (0.5f), 4.0f, 0.8f);

    if (bounds.getWidth() < 2.0f || bounds.getHeight() < 2.0f)
        return;

    if (!introComputed)
        computeIntroWaveform();

    int numSamples = static_cast<int> (bounds.getWidth());
    displayBuffer.resize (static_cast<size_t> (numSamples));
    waveformBuffer.read (displayBuffer.data(), numSamples);

    float midY = bounds.getCentreY();
    float halfH = bounds.getHeight() * 0.45f;

    // ─── Grid: centre line + ±50% rails ───────────────────────────────
    const float gridX0 = bounds.getX() + 4.0f;
    const float gridX1 = bounds.getRight() - 4.0f;
    g.setColour (Palette::outline.withAlpha (0.20f));
    g.fillRect (gridX0, midY, gridX1 - gridX0, 1.0f);
    g.setColour (Palette::outline.withAlpha (0.10f));
    g.fillRect (gridX0, midY - halfH * 0.5f, gridX1 - gridX0, 1.0f);
    g.fillRect (gridX0, midY + halfH * 0.5f, gridX1 - gridX0, 1.0f);

    // Vertical reticule ticks every ~64px
    const float vReticuleStep = 64.0f;
    g.setColour (Palette::outline.withAlpha (0.10f));
    for (float vx = gridX0 + vReticuleStep; vx < gridX1; vx += vReticuleStep)
        g.fillRect (vx, bounds.getY() + 4.0f, 1.0f, bounds.getHeight() - 8.0f);

    // ─── Animation blend ───────────────────────────────────────────────
    float introAlpha = 0.0f;
    if (introFramesLeft > 0)
    {
        if (introFramesLeft > 60)
            introAlpha = 1.0f;
        else
            introAlpha = static_cast<float> (introFramesLeft) / 60.0f;
    }

    // ─── Waveform path ─────────────────────────────────────────────────
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

        if (!started) { path.startNewSubPath (x, y); started = true; }
        else          { path.lineTo (x, y); }
    }

    // ─── Multi-pass glow + crisp core ─────────────────────────────────
    g.setColour (Palette::primary.withAlpha (0.10f));
    g.strokePath (path, juce::PathStrokeType (5.0f, juce::PathStrokeType::curved));
    g.setColour (Palette::primary.withAlpha (0.25f));
    g.strokePath (path, juce::PathStrokeType (2.8f, juce::PathStrokeType::curved));
    g.setColour (Palette::hot);
    g.strokePath (path, juce::PathStrokeType (1.4f, juce::PathStrokeType::curved));

    // ─── CRT scanlines — every other row, very low alpha ──────────────
    // Evokes the chiptune-era oscilloscope aesthetic without obscuring
    // the trace. Cheap (one fillRect per scanline at 30Hz).
    g.setColour (juce::Colours::black.withAlpha (0.10f));
    for (float y = bounds.getY() + 1.0f; y < bounds.getBottom(); y += 2.0f)
        g.fillRect (bounds.getX(), y, bounds.getWidth(), 1.0f);

    // Subtle watermark
    if (introAlpha < 1.0f)
    {
        float watermarkAlpha = (1.0f - introAlpha) * 0.05f;
        g.setColour (Palette::textDim.withAlpha (watermarkAlpha));
        g.setFont (displayFont (bounds.getHeight() * 0.30f));
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
