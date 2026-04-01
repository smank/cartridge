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
    g.setColour (Colors::bgDark);
    g.fillRect (getLocalBounds());

    // Draw VRC6 divider if visible
    if (vrc6Visible)
    {
        int numApu = 5;
        int numTotal = 8;
        int stripW = getWidth() / numTotal;
        int dividerX = numApu * stripW;

        g.setColour (Colors::vrc6Divider);
        g.fillRect (dividerX - 1, 4, 2, getHeight() - 8);
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
