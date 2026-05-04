#include "ChannelStripArea.h"

namespace cart {

ChannelStripArea::ChannelStripArea (juce::AudioProcessorValueTreeState& apvts)
    : pulse1Strip     (ChannelType::Pulse1,     apvts),
      pulse2Strip     (ChannelType::Pulse2,     apvts),
      triangleStrip   (ChannelType::Triangle,   apvts),
      noiseStrip      (ChannelType::Noise,      apvts),
      dpcmStrip       (ChannelType::Dpcm,       apvts),
      vrc6Pulse1Strip (ChannelType::Vrc6Pulse1, apvts),
      vrc6Pulse2Strip (ChannelType::Vrc6Pulse2, apvts),
      vrc6SawStrip    (ChannelType::Vrc6Saw,    apvts)
{
    addAndMakeVisible (pulse1Strip);
    addAndMakeVisible (pulse2Strip);
    addAndMakeVisible (triangleStrip);
    addAndMakeVisible (noiseStrip);
    addAndMakeVisible (dpcmStrip);

    addChildComponent (vrc6Pulse1Strip);
    addChildComponent (vrc6Pulse2Strip);
    addChildComponent (vrc6SawStrip);
}

ChannelStripArea::~ChannelStripArea() = default;

void ChannelStripArea::setActivityFlags (std::atomic<bool>* flags)
{
    pulse1Strip.setActivityFlag     (&flags[0]);
    pulse2Strip.setActivityFlag     (&flags[1]);
    triangleStrip.setActivityFlag   (&flags[2]);
    noiseStrip.setActivityFlag      (&flags[3]);
    dpcmStrip.setActivityFlag       (&flags[4]);
    vrc6Pulse1Strip.setActivityFlag (&flags[5]);
    vrc6Pulse2Strip.setActivityFlag (&flags[6]);
    vrc6SawStrip.setActivityFlag    (&flags[7]);
}

void ChannelStripArea::setVrc6Visible (bool visible)
{
    if (vrc6Visible == visible)
        return;

    vrc6Visible = visible;
    vrc6Pulse1Strip.setVisible (visible);
    vrc6Pulse2Strip.setVisible (visible);
    vrc6SawStrip.setVisible (visible);
    resized();
    repaint();
}

void ChannelStripArea::paint (juce::Graphics& g)
{
    // Background is painted by the editor's gradient — leave transparent.

    // VRC6 divider: gradient stripe between APU and Konami sections
    if (vrc6Visible)
    {
        int numApu = 5;
        int numTotal = 8;
        int stripW = getWidth() / numTotal;
        int dividerX = numApu * stripW;

        // Wider, softer gradient — fades out top and bottom
        const auto accent = cart::ui::Palette::vrc6Accent;
        juce::ColourGradient grad (accent.withAlpha (0.0f), (float) dividerX, 0.0f,
                                   accent.withAlpha (0.0f), (float) dividerX, (float) getHeight(),
                                   false);
        grad.addColour (0.15, accent.withAlpha (0.85f));
        grad.addColour (0.85, accent.withAlpha (0.85f));
        g.setGradientFill (grad);
        g.fillRect (dividerX - 1, 0, 2, getHeight());
    }
}

void ChannelStripArea::resized()
{
    auto area = getLocalBounds().reduced (4, 4);
    const int pad = 2;

    if (vrc6Visible)
    {
        int numStrips = 8;
        int stripW = area.getWidth() / numStrips;

        pulse1Strip.setBounds     (area.removeFromLeft (stripW).reduced (pad, 0));
        pulse2Strip.setBounds     (area.removeFromLeft (stripW).reduced (pad, 0));
        triangleStrip.setBounds   (area.removeFromLeft (stripW).reduced (pad, 0));
        noiseStrip.setBounds      (area.removeFromLeft (stripW).reduced (pad, 0));
        dpcmStrip.setBounds       (area.removeFromLeft (stripW).reduced (pad, 0));
        vrc6Pulse1Strip.setBounds (area.removeFromLeft (stripW).reduced (pad, 0));
        vrc6Pulse2Strip.setBounds (area.removeFromLeft (stripW).reduced (pad, 0));
        vrc6SawStrip.setBounds    (area.reduced (pad, 0));
    }
    else
    {
        int numStrips = 5;
        int stripW = area.getWidth() / numStrips;

        pulse1Strip.setBounds   (area.removeFromLeft (stripW).reduced (pad, 0));
        pulse2Strip.setBounds   (area.removeFromLeft (stripW).reduced (pad, 0));
        triangleStrip.setBounds (area.removeFromLeft (stripW).reduced (pad, 0));
        noiseStrip.setBounds    (area.removeFromLeft (stripW).reduced (pad, 0));
        dpcmStrip.setBounds     (area.reduced (pad, 0));
    }
}

} // namespace cart
