#include "ModernPanelComponent.h"
#include "Parameters.h"
#include <cmath>

namespace cart {

using namespace cart::ui;

ModernPanelComponent::ModernPanelComponent (juce::AudioProcessorValueTreeState& apvts)
{
    // ─── Osc A ──────────────────────────────────────────────────────────
    oscAToggle.setButtonText ("A");
    addAndMakeVisible (oscAToggle);
    oscAToggleAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::ModOscAEnabled, oscAToggle);

    waveformCombo.addItemList ({ "Pulse 25%", "Pulse 50%", "Pulse 75%", "Pulse 12.5%",
                                 "Triangle", "Sawtooth", "Noise" }, 1);
    addAndMakeVisible (waveformCombo);
    waveformAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        apvts, ParamIDs::ModWaveform, waveformCombo);
    makeLabel (waveformLabel, "Waveform A");

    // ─── Osc B ──────────────────────────────────────────────────────────
    oscBToggle.setButtonText ("B");
    addAndMakeVisible (oscBToggle);
    oscBToggleAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::ModOscBEnabled, oscBToggle);

    waveformBCombo.addItemList ({ "Pulse 25%", "Pulse 50%", "Pulse 75%", "Pulse 12.5%",
                                  "Triangle", "Sawtooth", "Noise" }, 1);
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
    knob.setRotaryParameters (juce::MathConstants<float>::pi * 1.2f,
                              juce::MathConstants<float>::pi * 2.8f,
                              true);
    knob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, Metrics::rotaryKnobDiameter, 14);
    knob.setPopupMenuEnabled (true);
}

void ModernPanelComponent::makeLabel (juce::Label& label, const juce::String& text)
{
    label.setText (text, juce::dontSendNotification);
    label.setFont (labelFont (Metrics::fontLabelSmall));
    label.setColour (juce::Label::textColourId, Palette::textSecondary);
    label.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (label);
}

void ModernPanelComponent::paint (juce::Graphics& g)
{
    using namespace cart::ui;
    // Panel background painted by editor's gradient — leave transparent.

    auto bounds = getLocalBounds().reduced (8, 4);
    const int sectionGap = 4;
    const int sectionH = (bounds.getHeight() - 2 * sectionGap) / 3;
    const float cornerR = 6.0f;
    const int accentH = 3;
    const int headerH = 20;
    const int pad = 12;

    const char* sectionNames[] = { nullptr, "ENVELOPE", "GLOBAL" };

    for (int i = 0; i < 3; ++i)
    {
        auto sb = juce::Rectangle<float> (
            (float) bounds.getX(),
            (float) (bounds.getY() + i * (sectionH + sectionGap)),
            (float) bounds.getWidth(),
            (float) sectionH);

        // Panel body — vertical gradient for depth
        juce::ColourGradient bodyGrad (
            Palette::surfaceAlt.brighter (0.05f), 0.0f, sb.getY(),
            Palette::surface.darker (0.10f),      0.0f, sb.getBottom(),
            false);
        g.setGradientFill (bodyGrad);
        g.fillRoundedRectangle (sb, cornerR);

        g.setColour (Palette::outlineDim.withAlpha (0.7f));
        g.drawRoundedRectangle (sb.reduced (0.25f), cornerR, 0.8f);

        // Accent stripe with horizontal gradient + soft glow trailing down
        {
            g.saveState();
            g.reduceClipRegion (sb.toNearestIntEdges());

            juce::ColourGradient stripeGrad (
                Palette::primary,         sb.getX(),     sb.getY(),
                Palette::primary.withAlpha (0.55f), sb.getRight(), sb.getY(),
                false);
            g.setGradientFill (stripeGrad);
            g.fillRect (sb.withHeight ((float) accentH));

            // Diagonal hairlines on the right — smank signature
            g.setColour (Palette::primary.withAlpha (0.35f));
            const float dx0 = sb.getRight() - 28.0f;
            const float dy0 = sb.getY() + (float) accentH + 2.0f;
            const float dy1 = sb.getY() + (float) accentH + (float) headerH;
            g.drawLine (dx0,         dy0, dx0 + 16.0f, dy1, 1.0f);
            g.drawLine (dx0 + 6.0f,  dy0, dx0 + 22.0f, dy1, 1.0f);

            // Soft accent glow under the stripe
            juce::ColourGradient glow (
                Palette::primary.withAlpha (0.12f), sb.getCentreX(), sb.getY(),
                Palette::primary.withAlpha (0.0f),  sb.getCentreX(), sb.getY() + 14.0f,
                false);
            g.setGradientFill (glow);
            g.fillRect (sb.withHeight (14.0f));

            g.restoreState();
        }

        // Header text
        float labelY = sb.getY() + (float) accentH + 2.0f;
        float p = (float) pad;

        if (i == 0)
        {
            float divX = sb.getX() + std::floor (sb.getWidth() * 0.5f);

            g.setColour (Palette::textPrimary);
            g.setFont (displayFont (Metrics::fontLabelSmall));
            g.drawText ("OSC A",
                        juce::Rectangle<float> (sb.getX() + p, labelY, divX - sb.getX() - p, (float) headerH),
                        juce::Justification::centredLeft);
            g.drawText ("OSC B",
                        juce::Rectangle<float> (divX + p, labelY, sb.getRight() - divX - p, (float) headerH),
                        juce::Justification::centredLeft);

            g.setColour (Palette::outlineDim);
            g.drawVerticalLine ((int) divX,
                                sb.getY() + (float) accentH + 2.0f,
                                sb.getBottom() - 4.0f);
        }
        else
        {
            g.setColour (Palette::textPrimary);
            g.setFont (displayFont (Metrics::fontLabelSmall));
            g.drawText (sectionNames[i],
                        juce::Rectangle<float> (sb.getX() + p, labelY, sb.getWidth() - 2.0f * p, (float) headerH),
                        juce::Justification::centredLeft);
        }
    }
}

void ModernPanelComponent::resized()
{
    auto bounds = getLocalBounds().reduced (8, 4);
    const int sectionGap = 4;
    const int sectionH = (bounds.getHeight() - 2 * sectionGap) / 3;
    const int accentH = 3;
    const int headerH = 20;
    const int headerArea = accentH + 2 + headerH;  // accent + gap + header text
    const int pad = 12;
    const int labelH = 14;
    const int bottomPad = 2;

    // Derive slider sizes from available section content height
    int contentH = sectionH - headerArea - bottomPad;
    int knobH = juce::jmax (60, contentH - labelH);  // slider height
    int knobW = 64;  // rotary diameter target

    // Combo / toggle sizing
    int comboH = juce::jlimit (22, 28, knobH / 2);
    int comboW = juce::jlimit (100, 140, knobH * 2);
    int toggleW = juce::jlimit (36, 44, knobH * 2 / 3);
    int toggleH = comboH;

    // Content area within a section
    auto sectionContent = [&] (int index) -> juce::Rectangle<int>
    {
        int sy = bounds.getY() + index * (sectionH + sectionGap) + headerArea;
        return juce::Rectangle<int> (bounds.getX() + pad, sy, bounds.getWidth() - 2 * pad, contentH);
    };

    // Nudge controls slightly above center
    const int nudgeUp = 6;

    // ─── Section 1: Oscillators — OSC A (left) | OSC B (right) ─────────
    auto r1 = sectionContent (0);
    int halfW = r1.getWidth() / 2;

    int comboBlockH = comboH + 2 + labelH;
    int comboY = r1.getY() + (r1.getHeight() - comboBlockH) / 2 - nudgeUp;
    int sliderBlockH = knobH + labelH;
    int knobY1 = r1.getY() + (r1.getHeight() - sliderBlockH) / 2 - nudgeUp;

    // Osc A: center toggle+combo in left half
    {
        int groupW = toggleW + 6 + comboW;
        int x0 = r1.getX() + (halfW - groupW) / 2;
        oscAToggle.setBounds (x0, comboY, toggleW, toggleH);
        waveformCombo.setBounds (x0 + toggleW + 6, comboY, comboW, comboH);
        waveformLabel.setBounds (x0 + toggleW + 6, comboY + comboH + 2, comboW, labelH);
    }

    // Osc B: center toggle+combo+sliders in right half
    {
        int groupW = toggleW + 6 + comboW + 12 + knobW + 6 + knobW;
        int x0 = r1.getX() + halfW + (halfW - groupW) / 2;
        oscBToggle.setBounds (x0, comboY, toggleW, toggleH);
        x0 += toggleW + 6;
        waveformBCombo.setBounds (x0, comboY, comboW, comboH);
        waveformBLabel.setBounds (x0, comboY + comboH + 2, comboW, labelH);
        x0 += comboW + 12;
        oscBLevelKnob.setBounds (x0, knobY1, knobW, knobH);
        oscBLevelLabel.setBounds (x0, knobY1 + knobH, knobW, labelH);
        x0 += knobW + 6;
        oscBDetuneKnob.setBounds (x0, knobY1, knobW, knobH);
        oscBDetuneLabel.setBounds (x0, knobY1 + knobH, knobW, labelH);
    }

    // ─── Section 2: Envelope — 4 ADSR sliders, evenly distributed ──────
    auto r2 = sectionContent (1);
    int knobY2 = r2.getY() + (r2.getHeight() - sliderBlockH) / 2 - nudgeUp;
    {
        const int n = 4;
        int gap = (r2.getWidth() - n * knobW) / (n + 1);
        int x = r2.getX() + gap;

        juce::Slider* knobs[] = { &attackKnob, &decayKnob, &sustainKnob, &releaseKnob };
        juce::Label* labels[] = { &attackLabel, &decayLabel, &sustainLabel, &releaseLabel };

        for (int k = 0; k < n; ++k)
        {
            knobs[k]->setBounds (x, knobY2, knobW, knobH);
            labels[k]->setBounds (x, knobY2 + knobH, knobW, labelH);
            x += knobW + gap;
        }
    }

    // ─── Section 3: Global — 5 sliders + porta(toggle+slider) + vel ────
    auto r3 = sectionContent (2);
    int knobY3 = r3.getY() + (r3.getHeight() - sliderBlockH) / 2 - nudgeUp;
    {
        int portaToggleW = 50;
        int portaSlotW = portaToggleW + 4 + knobW;
        int totalW = 5 * knobW + portaSlotW;
        int nSlots = 7;
        int gap = juce::jmax (2, (r3.getWidth() - totalW) / (nSlots + 1));
        int x = r3.getX() + gap;

        juce::Slider* gKnobs[] = { &voicesKnob, &volumeKnob, &unisonKnob, &detuneKnob };
        juce::Label* gLabels[] = { &voicesLabel, &volumeLabel, &unisonLabel, &detuneLabel };

        for (int k = 0; k < 4; ++k)
        {
            gKnobs[k]->setBounds (x, knobY3, knobW, knobH);
            gLabels[k]->setBounds (x, knobY3 + knobH, knobW, labelH);
            x += knobW + gap;
        }

        // Porta toggle + time slider
        int portaToggleY = knobY3 + (knobH - toggleH) / 2;
        portaToggle.setBounds (x, portaToggleY, portaToggleW, toggleH);
        x += portaToggleW + 4;
        portaTimeKnob.setBounds (x, knobY3, knobW, knobH);
        portaTimeLabel.setBounds (x, knobY3 + knobH, knobW, labelH);
        x += knobW + gap;

        // Vel>Flt
        velFilterKnob.setBounds (x, knobY3, knobW, knobH);
        velFilterLabel.setBounds (x, knobY3 + knobH, knobW, labelH);
    }
}

} // namespace cart
