#include "ModernPanelComponent.h"
#include "Parameters.h"

namespace cart {

ModernPanelComponent::ModernPanelComponent (juce::AudioProcessorValueTreeState& apvts)
{
    // ─── Osc A ──────────────────────────────────────────────────────────
    oscAToggle.setButtonText ("A");
    oscAToggle.setColour (juce::ToggleButton::textColourId, Colors::textPrimary);
    oscAToggle.setColour (juce::ToggleButton::tickColourId, Colors::accentActive);
    addAndMakeVisible (oscAToggle);
    oscAToggleAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::ModOscAEnabled, oscAToggle);

    waveformCombo.addItemList ({ "Pulse 25%", "Pulse 50%", "Pulse 75%", "Pulse 12.5%",
                                 "Triangle", "Sawtooth", "Noise" }, 1);
    waveformCombo.setColour (juce::ComboBox::backgroundColourId, Colors::bgLight);
    waveformCombo.setColour (juce::ComboBox::textColourId, Colors::textPrimary);
    waveformCombo.setColour (juce::ComboBox::outlineColourId, Colors::knobOutline);
    addAndMakeVisible (waveformCombo);
    waveformAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        apvts, ParamIDs::ModWaveform, waveformCombo);
    makeLabel (waveformLabel, "Waveform A");

    // ─── Osc B ──────────────────────────────────────────────────────────
    oscBToggle.setButtonText ("B");
    oscBToggle.setColour (juce::ToggleButton::textColourId, Colors::textPrimary);
    oscBToggle.setColour (juce::ToggleButton::tickColourId, Colors::accentActive);
    addAndMakeVisible (oscBToggle);
    oscBToggleAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::ModOscBEnabled, oscBToggle);

    waveformBCombo.addItemList ({ "Pulse 25%", "Pulse 50%", "Pulse 75%", "Pulse 12.5%",
                                  "Triangle", "Sawtooth", "Noise" }, 1);
    waveformBCombo.setColour (juce::ComboBox::backgroundColourId, Colors::bgLight);
    waveformBCombo.setColour (juce::ComboBox::textColourId, Colors::textPrimary);
    waveformBCombo.setColour (juce::ComboBox::outlineColourId, Colors::knobOutline);
    addAndMakeVisible (waveformBCombo);
    waveformBAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        apvts, ParamIDs::ModWaveformB, waveformBCombo);
    makeLabel (waveformBLabel, "Waveform B");

    styleKnob (oscBLevelKnob);
    addAndMakeVisible (oscBLevelKnob);
    oscBLevelAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::ModOscBLevel, oscBLevelKnob);
    makeLabel (oscBLevelLabel, "Level");

    styleKnob (oscBDetuneKnob);
    addAndMakeVisible (oscBDetuneKnob);
    oscBDetuneAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::ModOscBDetune, oscBDetuneKnob);
    makeLabel (oscBDetuneLabel, "Semi");

    // ─── Voices ─────────────────────────────────────────────────────────
    styleKnob (voicesKnob);
    addAndMakeVisible (voicesKnob);
    voicesAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::ModVoices, voicesKnob);
    makeLabel (voicesLabel, "Voices");

    // ─── ADSR ───────────────────────────────────────────────────────────
    styleKnob (attackKnob);
    addAndMakeVisible (attackKnob);
    attackAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::ModAttack, attackKnob);
    makeLabel (attackLabel, "Attack");

    styleKnob (decayKnob);
    addAndMakeVisible (decayKnob);
    decayAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::ModDecay, decayKnob);
    makeLabel (decayLabel, "Decay");

    styleKnob (sustainKnob);
    addAndMakeVisible (sustainKnob);
    sustainAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::ModSustain, sustainKnob);
    makeLabel (sustainLabel, "Sustain");

    styleKnob (releaseKnob);
    addAndMakeVisible (releaseKnob);
    releaseAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::ModRelease, releaseKnob);
    makeLabel (releaseLabel, "Release");

    // ─── Unison ─────────────────────────────────────────────────────────
    styleKnob (unisonKnob);
    addAndMakeVisible (unisonKnob);
    unisonAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::ModUnison, unisonKnob);
    makeLabel (unisonLabel, "Unison");

    styleKnob (detuneKnob);
    addAndMakeVisible (detuneKnob);
    detuneAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::ModDetune, detuneKnob);
    makeLabel (detuneLabel, "Detune");

    // ─── Portamento ─────────────────────────────────────────────────────
    portaToggle.setButtonText ("Porta");
    portaToggle.setColour (juce::ToggleButton::textColourId, Colors::textPrimary);
    portaToggle.setColour (juce::ToggleButton::tickColourId, Colors::accentActive);
    addAndMakeVisible (portaToggle);
    portaToggleAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::ModPortaEnabled, portaToggle);

    styleKnob (portaTimeKnob);
    addAndMakeVisible (portaTimeKnob);
    portaTimeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::ModPortaTime, portaTimeKnob);
    makeLabel (portaTimeLabel, "Time");

    // ─── Volume ─────────────────────────────────────────────────────────
    styleKnob (volumeKnob);
    addAndMakeVisible (volumeKnob);
    volumeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::ModVolume, volumeKnob);
    makeLabel (volumeLabel, "Volume");

    // ─── Vel->Filter ─────────────────────────────────────────────────────
    styleKnob (velFilterKnob);
    addAndMakeVisible (velFilterKnob);
    velFilterAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::ModVelToFilter, velFilterKnob);
    makeLabel (velFilterLabel, "Vel>Flt");
}

void ModernPanelComponent::styleKnob (juce::Slider& knob)
{
    knob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    knob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 56, 14);
    knob.setColour (juce::Slider::rotarySliderFillColourId, Colors::accentActive);
    knob.setColour (juce::Slider::rotarySliderOutlineColourId, Colors::knobOutline);
    knob.setColour (juce::Slider::thumbColourId, Colors::accentActive);
    knob.setColour (juce::Slider::textBoxTextColourId, Colors::textPrimary);
    knob.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
}

void ModernPanelComponent::makeLabel (juce::Label& label, const juce::String& text)
{
    label.setText (text, juce::dontSendNotification);
    label.setFont (juce::FontOptions (11.0f));
    label.setColour (juce::Label::textColourId, Colors::textSecondary);
    label.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (label);
}

void ModernPanelComponent::paint (juce::Graphics& g)
{
    // Dark chassis background (visible between section panels)
    g.setColour (Colors::bgDark);
    g.fillRect (getLocalBounds());

    auto bounds = getLocalBounds().reduced (10, 8);
    const int sectionGap = 8;
    const int sectionH = (bounds.getHeight() - 2 * sectionGap) / 3;
    const float cornerR = 6.0f;
    const int accentH = 3;
    const int headerH = 18;
    const juce::String sectionNames[] = { "OSCILLATORS", "ENVELOPE", "GLOBAL" };

    for (int i = 0; i < 3; ++i)
    {
        auto sectionBounds = juce::Rectangle<float> (
            (float) bounds.getX(),
            (float) (bounds.getY() + i * (sectionH + sectionGap)),
            (float) bounds.getWidth(),
            (float) sectionH);

        // Section panel background
        g.setColour (Colors::bgStrip);
        g.fillRoundedRectangle (sectionBounds, cornerR);

        // Subtle outline
        g.setColour (Colors::divider);
        g.drawRoundedRectangle (sectionBounds, cornerR, 0.5f);

        // Accent stripe at top — fill rounded rect then cover bottom half to keep only top corners rounded
        auto accentBar = sectionBounds.withHeight ((float) accentH);
        g.setColour (Colors::accentActive);
        g.fillRoundedRectangle (accentBar.withHeight ((float) accentH + cornerR), cornerR);
        // Cover the bottom rounded corners with a flat rect
        g.fillRect (accentBar.withTrimmedTop (1.0f));

        // Section label
        g.setColour (Colors::textPrimary);
        g.setFont (juce::FontOptions (12.0f).withStyle ("Bold"));
        g.drawText (sectionNames[i],
                    sectionBounds.getX() + 10.0f,
                    sectionBounds.getY() + (float) accentH + 2.0f,
                    sectionBounds.getWidth() - 20.0f,
                    (float) headerH,
                    juce::Justification::centredLeft);

        // Row 1: draw OSC B label on right half and vertical divider
        if (i == 0)
        {
            float divX = sectionBounds.getX() + sectionBounds.getWidth() * 0.5f;
            g.setColour (Colors::divider);
            g.drawVerticalLine ((int) divX,
                                sectionBounds.getY() + (float) accentH + 4.0f,
                                sectionBounds.getBottom() - 6.0f);

            g.setColour (Colors::textPrimary);
            g.setFont (juce::FontOptions (12.0f).withStyle ("Bold"));
            g.drawText ("OSC B",
                        divX + 10.0f,
                        sectionBounds.getY() + (float) accentH + 2.0f,
                        sectionBounds.getWidth() * 0.5f - 20.0f,
                        (float) headerH,
                        juce::Justification::centredLeft);

            // Rename first section label to "OSC A"
            g.setColour (Colors::bgStrip);
            g.fillRect (sectionBounds.getX() + 10.0f,
                        sectionBounds.getY() + (float) accentH + 2.0f,
                        100.0f, (float) headerH);
            g.setColour (Colors::textPrimary);
            g.setFont (juce::FontOptions (12.0f).withStyle ("Bold"));
            g.drawText ("OSC A",
                        sectionBounds.getX() + 10.0f,
                        sectionBounds.getY() + (float) accentH + 2.0f,
                        sectionBounds.getWidth() * 0.5f - 20.0f,
                        (float) headerH,
                        juce::Justification::centredLeft);
        }
    }

    // NES vent groove decoration — 3 thin horizontal lines near bottom-left
    {
        float ventX = (float) bounds.getX() + 6.0f;
        float ventY = (float) (bounds.getY() + 3 * sectionH + 2 * sectionGap) - 2.0f;
        float ventW = 24.0f;
        g.setColour (Colors::bgLight);
        for (int v = 0; v < 3; ++v)
        {
            float y = ventY + (float) v * 3.0f;
            g.drawHorizontalLine ((int) y, ventX, ventX + ventW);
        }
    }
}

void ModernPanelComponent::resized()
{
    auto bounds = getLocalBounds().reduced (10, 8);
    const int sectionGap = 8;
    const int sectionH = (bounds.getHeight() - 2 * sectionGap) / 3;
    const int accentH = 3;
    const int headerH = 18;
    const int pad = 10;      // horizontal padding inside section
    const int knobW = 56;
    const int knobH = 56;
    const int labelH = 14;

    // Helper: content area within a section (below accent + header, with padding)
    auto sectionContent = [&] (int index) -> juce::Rectangle<int>
    {
        int sy = bounds.getY() + index * (sectionH + sectionGap) + accentH + headerH + 2;
        int sh = sectionH - accentH - headerH - 2 - 6; // 6px bottom padding
        return juce::Rectangle<int> (bounds.getX() + pad, sy, bounds.getWidth() - 2 * pad, sh);
    };

    // ─── Row 1: Osc A (left half) | Osc B (right half) ────────────────
    auto r1 = sectionContent (0);
    int r1y = r1.getY();
    int halfW = r1.getWidth() / 2;

    // Osc A: toggle + waveform combo
    int x = r1.getX();
    oscAToggle.setBounds (x, r1y, 36, 24);
    x += 38;
    waveformCombo.setBounds (x, r1y, 110, 24);
    waveformLabel.setBounds (x, r1y + 26, 110, labelH);

    // Osc B: toggle + waveform combo + level knob + detune knob
    x = r1.getX() + halfW + 4;
    oscBToggle.setBounds (x, r1y, 36, 24);
    x += 38;
    waveformBCombo.setBounds (x, r1y, 110, 24);
    waveformBLabel.setBounds (x, r1y + 26, 110, labelH);
    x += 118;
    oscBLevelKnob.setBounds (x, r1y - 4, knobW, knobH);
    oscBLevelLabel.setBounds (x, r1y + knobH - 8, knobW, labelH);
    x += knobW + 4;
    oscBDetuneKnob.setBounds (x, r1y - 4, knobW, knobH);
    oscBDetuneLabel.setBounds (x, r1y + knobH - 8, knobW, labelH);

    // ─── Row 2: Envelope — ADSR ─────────────────────────────────────────
    auto r2 = sectionContent (1);
    int r2y = r2.getY();
    x = r2.getX();

    attackKnob.setBounds (x, r2y, knobW, knobH);
    attackLabel.setBounds (x, r2y + knobH, knobW, labelH);
    x += knobW + 8;

    decayKnob.setBounds (x, r2y, knobW, knobH);
    decayLabel.setBounds (x, r2y + knobH, knobW, labelH);
    x += knobW + 8;

    sustainKnob.setBounds (x, r2y, knobW, knobH);
    sustainLabel.setBounds (x, r2y + knobH, knobW, labelH);
    x += knobW + 8;

    releaseKnob.setBounds (x, r2y, knobW, knobH);
    releaseLabel.setBounds (x, r2y + knobH, knobW, labelH);

    // ─── Row 3: Global — Voices, Volume, Unison, Detune, Porta, Vel->Flt
    auto r3 = sectionContent (2);
    int r3y = r3.getY();
    x = r3.getX();

    voicesKnob.setBounds (x, r3y, knobW, knobH);
    voicesLabel.setBounds (x, r3y + knobH, knobW, labelH);
    x += knobW + 4;

    volumeKnob.setBounds (x, r3y, knobW, knobH);
    volumeLabel.setBounds (x, r3y + knobH, knobW, labelH);
    x += knobW + 4;

    unisonKnob.setBounds (x, r3y, knobW, knobH);
    unisonLabel.setBounds (x, r3y + knobH, knobW, labelH);
    x += knobW + 4;

    detuneKnob.setBounds (x, r3y, knobW, knobH);
    detuneLabel.setBounds (x, r3y + knobH, knobW, labelH);
    x += knobW + 8;

    portaToggle.setBounds (x, r3y, 54, 24);
    portaTimeKnob.setBounds (x + 54, r3y, knobW, knobH);
    portaTimeLabel.setBounds (x + 54, r3y + knobH, knobW, labelH);
    x += 54 + knobW + 8;

    velFilterKnob.setBounds (x, r3y, knobW, knobH);
    velFilterLabel.setBounds (x, r3y + knobH, knobW, labelH);
}

} // namespace cart
