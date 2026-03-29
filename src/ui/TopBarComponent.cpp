#include "TopBarComponent.h"
#include "PluginProcessor.h"

namespace cart {

TopBarComponent::TopBarComponent (CartridgeProcessor& processor,
                                  juce::AudioProcessorValueTreeState& apvts)
    : processorRef (processor)
{
    // ─── Preset Combo ────────────────────────────────────────────────────
    presetCombo.setColour (juce::ComboBox::backgroundColourId, Colors::bgLight);
    presetCombo.setColour (juce::ComboBox::textColourId, Colors::fxBright);
    presetCombo.setColour (juce::ComboBox::outlineColourId, Colors::knobOutline);
    populatePresets();
    presetCombo.onChange = [this] { selectPreset (presetCombo.getSelectedId() - 1); };
    addAndMakeVisible (presetCombo);

    // Prev/Next buttons
    auto styleBtn = [] (juce::TextButton& btn)
    {
        btn.setColour (juce::TextButton::buttonColourId, Colors::bgLight);
        btn.setColour (juce::TextButton::textColourOffId, Colors::textPrimary);
    };
    styleBtn (prevButton);
    styleBtn (nextButton);

    prevButton.onClick = [this]
    {
        int cur = processorRef.getCurrentProgram();
        int total = processorRef.getNumPrograms();
        selectPreset ((cur - 1 + total) % total);
    };
    nextButton.onClick = [this]
    {
        int cur = processorRef.getCurrentProgram();
        int total = processorRef.getNumPrograms();
        selectPreset ((cur + 1) % total);
    };
    addAndMakeVisible (prevButton);
    addAndMakeVisible (nextButton);

    // Save button
    styleBtn (saveButton);
    saveButton.onClick = [this]
    {
        auto* dlg = new juce::AlertWindow ("Save Preset", "Enter preset name:", juce::MessageBoxIconType::NoIcon);
        dlg->addTextEditor ("name", "My Preset", "Name:");
        dlg->addButton ("Save", 1);
        dlg->addButton ("Cancel", 0);

        // Style the alert window
        dlg->setColour (juce::AlertWindow::backgroundColourId, Colors::bgMid);
        dlg->setColour (juce::AlertWindow::textColourId, Colors::textPrimary);
        dlg->setColour (juce::AlertWindow::outlineColourId, Colors::knobOutline);

        dlg->enterModalState (true, juce::ModalCallbackFunction::create (
            [this, dlg] (int result)
            {
                if (result == 1)
                {
                    auto presetName = dlg->getTextEditorContents ("name");
                    if (presetName.isNotEmpty())
                    {
                        processorRef.getPresetManager().saveUserPreset (presetName, processorRef.getApvts());
                        populatePresets();
                    }
                }
                delete dlg;
            }), false);
    };
    addAndMakeVisible (saveButton);

    // ─── Master Volume ───────────────────────────────────────────────────
    masterVolSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    masterVolSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    masterVolSlider.setColour (juce::Slider::trackColourId, Colors::accentDim);
    masterVolSlider.setColour (juce::Slider::thumbColourId, Colors::accentActive);
    addAndMakeVisible (masterVolSlider);
    masterVolAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::MasterVolume, masterVolSlider);

    masterVolLabel.setText ("Vol", juce::dontSendNotification);
    masterVolLabel.setFont (juce::FontOptions (11.0f));
    masterVolLabel.setColour (juce::Label::textColourId, Colors::textSecondary);
    addAndMakeVisible (masterVolLabel);

    // ─── Master Tune ─────────────────────────────────────────────────────
    masterTuneSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    masterTuneSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    masterTuneSlider.setColour (juce::Slider::trackColourId, Colors::accentDim);
    masterTuneSlider.setColour (juce::Slider::thumbColourId, Colors::accentActive);
    addAndMakeVisible (masterTuneSlider);
    masterTuneAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::MasterTune, masterTuneSlider);

    masterTuneLabel.setText ("Tune", juce::dontSendNotification);
    masterTuneLabel.setFont (juce::FontOptions (11.0f));
    masterTuneLabel.setColour (juce::Label::textColourId, Colors::textSecondary);
    addAndMakeVisible (masterTuneLabel);

    // ─── Region Combo ────────────────────────────────────────────────────
    regionCombo.addItem ("NTSC", 1);
    regionCombo.addItem ("PAL", 2);
    regionCombo.setColour (juce::ComboBox::backgroundColourId, Colors::bgLight);
    regionCombo.setColour (juce::ComboBox::textColourId, Colors::textPrimary);
    regionCombo.setColour (juce::ComboBox::outlineColourId, Colors::knobOutline);
    addAndMakeVisible (regionCombo);
    regionAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        apvts, ParamIDs::Region, regionCombo);

    // ─── MIDI Mode Combo ─────────────────────────────────────────────────
    midiModeCombo.addItem ("Split", 1);
    midiModeCombo.addItem ("Auto", 2);
    midiModeCombo.addItem ("Mono", 3);
    midiModeCombo.setColour (juce::ComboBox::backgroundColourId, Colors::bgLight);
    midiModeCombo.setColour (juce::ComboBox::textColourId, Colors::textPrimary);
    midiModeCombo.setColour (juce::ComboBox::outlineColourId, Colors::knobOutline);
    addAndMakeVisible (midiModeCombo);
    midiModeAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        apvts, ParamIDs::MidiMode, midiModeCombo);

    // ─── Velocity Sensitivity ─────────────────────────────────────────────
    velocitySensSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    velocitySensSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    velocitySensSlider.setColour (juce::Slider::trackColourId, Colors::accentDim);
    velocitySensSlider.setColour (juce::Slider::thumbColourId, Colors::accentActive);
    addAndMakeVisible (velocitySensSlider);
    velocitySensAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::VelocitySens, velocitySensSlider);

    velocitySensLabel.setText ("Vel", juce::dontSendNotification);
    velocitySensLabel.setFont (juce::FontOptions (11.0f));
    velocitySensLabel.setColour (juce::Label::textColourId, Colors::textSecondary);
    addAndMakeVisible (velocitySensLabel);

    // ─── Pitch Bend Range ─────────────────────────────────────────────────
    pitchBendSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    pitchBendSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    pitchBendSlider.setColour (juce::Slider::trackColourId, Colors::accentDim);
    pitchBendSlider.setColour (juce::Slider::thumbColourId, Colors::accentActive);
    addAndMakeVisible (pitchBendSlider);
    pitchBendAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::PitchBendRange, pitchBendSlider);

    pitchBendLabel.setText ("PB", juce::dontSendNotification);
    pitchBendLabel.setFont (juce::FontOptions (11.0f));
    pitchBendLabel.setColour (juce::Label::textColourId, Colors::textSecondary);
    addAndMakeVisible (pitchBendLabel);

    // ─── VRC6 Toggle ─────────────────────────────────────────────────────
    vrc6Toggle.setButtonText ("VRC6");
    vrc6Toggle.setColour (juce::ToggleButton::textColourId, Colors::orangeAccent);
    vrc6Toggle.setColour (juce::ToggleButton::tickColourId, Colors::orangeAccent);
    vrc6Toggle.onClick = [this]
    {
        if (onVrc6Toggle)
            onVrc6Toggle (vrc6Toggle.getToggleState());
    };
    addAndMakeVisible (vrc6Toggle);
    vrc6Attach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::Vrc6Enabled, vrc6Toggle);

    // Poll for DAW-initiated preset changes
    startTimerHz (4);
}

TopBarComponent::~TopBarComponent()
{
    stopTimer();
}

void TopBarComponent::populatePresets()
{
    presetCombo.clear (juce::dontSendNotification);
    int numPresets = processorRef.getNumPrograms();
    for (int i = 0; i < numPresets; ++i)
        presetCombo.addItem (processorRef.getProgramName (i), i + 1);

    presetCombo.setSelectedId (processorRef.getCurrentProgram() + 1, juce::dontSendNotification);
}

void TopBarComponent::selectPreset (int index)
{
    processorRef.setCurrentProgram (index);
    presetCombo.setSelectedId (index + 1, juce::dontSendNotification);

    if (onPresetChanged)
        onPresetChanged();
}

void TopBarComponent::navigatePreset (int delta)
{
    int cur = processorRef.getCurrentProgram();
    int total = processorRef.getNumPrograms();
    selectPreset ((cur + delta + total) % total);
}

void TopBarComponent::timerCallback()
{
    int current = processorRef.getCurrentProgram();
    if (presetCombo.getSelectedId() != current + 1)
    {
        presetCombo.setSelectedId (current + 1, juce::dontSendNotification);

        if (onPresetChanged)
            onPresetChanged();
    }

    // Only fire VRC6 toggle callback when state actually changes
    bool vrc6On = vrc6Toggle.getToggleState();
    if (vrc6On != lastVrc6State)
    {
        lastVrc6State = vrc6On;
        if (onVrc6Toggle)
            onVrc6Toggle (vrc6On);
    }
}

void TopBarComponent::paint (juce::Graphics& g)
{
    g.setColour (Colors::bgMid);
    g.fillRect (getLocalBounds());

    // Bottom divider line
    g.setColour (Colors::divider);
    g.fillRect (0, getHeight() - 1, getWidth(), 1);
}

void TopBarComponent::resized()
{
    auto area = getLocalBounds().reduced (6, 4);

    // Preset section
    prevButton.setBounds (area.removeFromLeft (26));
    area.removeFromLeft (2);
    nextButton.setBounds (area.removeFromLeft (26));
    area.removeFromLeft (2);
    saveButton.setBounds (area.removeFromLeft (40));
    area.removeFromLeft (4);

    int presetW = juce::jmin (200, area.getWidth() / 3);
    presetCombo.setBounds (area.removeFromLeft (presetW));
    area.removeFromLeft (8);

    // Right side: VRC6 toggle
    vrc6Toggle.setBounds (area.removeFromRight (60));
    area.removeFromRight (4);

    // MIDI mode
    midiModeCombo.setBounds (area.removeFromRight (65));
    area.removeFromRight (4);

    // Region
    regionCombo.setBounds (area.removeFromRight (60));
    area.removeFromRight (8);

    // Remaining space: Vol, Tune, Vel, PB sliders
    int remaining = area.getWidth();
    int labelSpace = 24 + 30 + 24 + 24; // Vol + Tune + Vel + PB labels
    int gaps = 8 * 3;                     // 3 inter-slider gaps
    int sliderW = (remaining - labelSpace - gaps) / 4;

    masterVolLabel.setBounds (area.removeFromLeft (24));
    masterVolSlider.setBounds (area.removeFromLeft (juce::jmax (sliderW, 30)));
    area.removeFromLeft (8);

    masterTuneLabel.setBounds (area.removeFromLeft (30));
    masterTuneSlider.setBounds (area.removeFromLeft (juce::jmax (sliderW, 30)));
    area.removeFromLeft (8);

    velocitySensLabel.setBounds (area.removeFromLeft (24));
    velocitySensSlider.setBounds (area.removeFromLeft (juce::jmax (sliderW, 30)));
    area.removeFromLeft (8);

    pitchBendLabel.setBounds (area.removeFromLeft (24));
    pitchBendSlider.setBounds (area.removeFromLeft (juce::jmax (sliderW, 30)));
}

} // namespace cart
