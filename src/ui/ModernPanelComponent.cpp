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
    g.setColour (Colors::bgMid);
    g.fillRect (getLocalBounds());

    // Section headers
    g.setColour (Colors::textSecondary);
    g.setFont (juce::FontOptions (12.0f).withStyle ("Bold"));

    auto bounds = getLocalBounds().reduced (12, 8);
    int rowH = bounds.getHeight() / 3;

    // Row headers
    auto headerArea = bounds;
    g.drawText ("OSC A", headerArea.removeFromTop (16).toFloat(), juce::Justification::centredLeft);
    headerArea.removeFromTop (rowH - 16);
    g.drawText ("ENVELOPE", headerArea.removeFromTop (16).toFloat(), juce::Justification::centredLeft);
    headerArea.removeFromTop (rowH - 16);
    g.drawText ("GLOBAL", headerArea.removeFromTop (16).toFloat(), juce::Justification::centredLeft);

    // Draw "OSC B" header at the right position in Row 1
    auto row1Header = bounds.removeFromTop (16);
    int oscBHeaderX = row1Header.getX() + row1Header.getWidth() / 2 + 4;
    g.drawText ("OSC B", oscBHeaderX, row1Header.getY(), row1Header.getWidth() / 2 - 4,
                row1Header.getHeight(), juce::Justification::centredLeft);
}

void ModernPanelComponent::resized()
{
    auto bounds = getLocalBounds().reduced (12, 8);
    int rowH = bounds.getHeight() / 3;
    int knobW = 56;
    int knobH = 56;
    int labelH = 14;

    // ─── Row 1: Osc A (left half) | Osc B (right half) ────────────────
    auto row1 = bounds.removeFromTop (rowH);
    row1.removeFromTop (18);  // Header space

    auto r1area = row1.reduced (0, 2);
    int r1y = r1area.getY();
    int halfW = r1area.getWidth() / 2;

    // Osc A: toggle + waveform combo
    int x = r1area.getX();
    oscAToggle.setBounds (x, r1y, 36, 24);
    x += 38;
    waveformCombo.setBounds (x, r1y, 110, 24);
    waveformLabel.setBounds (x, r1y + 26, 110, labelH);

    // Osc B: toggle + waveform combo + level knob + detune knob
    x = r1area.getX() + halfW + 4;
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
    auto row2 = bounds.removeFromTop (rowH);
    row2.removeFromTop (18);

    auto r2area = row2.reduced (0, 2);
    int r2y = r2area.getY();
    x = r2area.getX();

    attackLabel.setBounds (x, r2y + knobH, knobW, labelH);
    attackKnob.setBounds (x, r2y, knobW, knobH);
    x += knobW + 8;

    decayLabel.setBounds (x, r2y + knobH, knobW, labelH);
    decayKnob.setBounds (x, r2y, knobW, knobH);
    x += knobW + 8;

    sustainLabel.setBounds (x, r2y + knobH, knobW, labelH);
    sustainKnob.setBounds (x, r2y, knobW, knobH);
    x += knobW + 8;

    releaseLabel.setBounds (x, r2y + knobH, knobW, labelH);
    releaseKnob.setBounds (x, r2y, knobW, knobH);

    // ─── Row 3: Global — Voices, Volume, Unison, Detune, Porta, Vel->Flt
    auto row3 = bounds;
    row3.removeFromTop (18);

    auto r3area = row3.reduced (0, 2);
    int r3y = r3area.getY();
    x = r3area.getX();

    voicesLabel.setBounds (x, r3y + knobH, knobW, labelH);
    voicesKnob.setBounds (x, r3y, knobW, knobH);
    x += knobW + 4;

    volumeLabel.setBounds (x, r3y + knobH, knobW, labelH);
    volumeKnob.setBounds (x, r3y, knobW, knobH);
    x += knobW + 4;

    unisonLabel.setBounds (x, r3y + knobH, knobW, labelH);
    unisonKnob.setBounds (x, r3y, knobW, knobH);
    x += knobW + 4;

    detuneLabel.setBounds (x, r3y + knobH, knobW, labelH);
    detuneKnob.setBounds (x, r3y, knobW, knobH);
    x += knobW + 8;

    portaToggle.setBounds (x, r3y, 54, 24);
    portaTimeLabel.setBounds (x + 54, r3y + knobH, knobW, labelH);
    portaTimeKnob.setBounds (x + 54, r3y, knobW, knobH);
    x += 54 + knobW + 8;

    velFilterLabel.setBounds (x, r3y + knobH, knobW, labelH);
    velFilterKnob.setBounds (x, r3y, knobW, knobH);
}

} // namespace cart
