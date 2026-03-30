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

    // Panic button
    panicButton.setColour (juce::TextButton::buttonColourId, Colors::bgLight);
    panicButton.setColour (juce::TextButton::textColourOffId, Colors::fxBright);
    panicButton.onClick = [this] { processorRef.getKeyboardState().allNotesOff (0); };
    addAndMakeVisible (panicButton);

    // ─── Master Volume ───────────────────────────────────────────────────
    masterVolSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    masterVolSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    masterVolSlider.setColour (juce::Slider::trackColourId, Colors::accentDim);
    masterVolSlider.setColour (juce::Slider::thumbColourId, Colors::accentActive);
    addAndMakeVisible (masterVolSlider);
    masterVolSlider.setPopupDisplayEnabled (true, false, this);
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
    masterTuneSlider.setPopupDisplayEnabled (true, false, this);
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
    velocitySensSlider.setPopupDisplayEnabled (true, false, this);
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
    pitchBendSlider.setPopupDisplayEnabled (true, false, this);
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

    // ─── Scale ComboBox ────────────────────────────────────────────────
    scaleCombo.addItem ("75%", 1);
    scaleCombo.addItem ("100%", 2);
    scaleCombo.addItem ("125%", 3);
    scaleCombo.addItem ("150%", 4);
    scaleCombo.setSelectedId (2, juce::dontSendNotification);
    scaleCombo.setColour (juce::ComboBox::backgroundColourId, Colors::bgLight);
    scaleCombo.setColour (juce::ComboBox::textColourId, Colors::textPrimary);
    scaleCombo.setColour (juce::ComboBox::outlineColourId, Colors::knobOutline);
    scaleCombo.onChange = [this]
    {
        static constexpr float scales[] = { 0.75f, 1.0f, 1.25f, 1.5f };
        int idx = scaleCombo.getSelectedId() - 1;
        if (idx >= 0 && idx < 4 && onScaleChanged)
            onScaleChanged (scales[idx]);
    };
    addAndMakeVisible (scaleCombo);

    // ─── MIDI Info Button ────────────────────────────────────────────────
    midiInfoButton.setColour (juce::TextButton::buttonColourId, Colors::bgLight);
    midiInfoButton.setColour (juce::TextButton::textColourOffId, Colors::textSecondary);
    midiInfoButton.onClick = [this]
    {
        auto* popup = new juce::AlertWindow ("MIDI CC Mappings", "", juce::MessageBoxIconType::NoIcon);
        popup->setColour (juce::AlertWindow::backgroundColourId, Colors::bgMid);
        popup->setColour (juce::AlertWindow::textColourId, Colors::textPrimary);
        popup->setColour (juce::AlertWindow::outlineColourId, Colors::knobOutline);
        popup->addTextBlock (
            "CC 1   Vibrato Depth\n"
            "CC 11  Master Volume\n"
            "CC 64  Sustain Pedal\n"
            "CC 71  Filter Resonance\n"
            "CC 74  Filter Cutoff\n"
            "CC 91  Reverb Mix\n"
            "CC 93  Chorus Mix");
        popup->addButton ("OK", 0);
        popup->enterModalState (true, juce::ModalCallbackFunction::create (
            [popup] (int) { delete popup; }), false);
    };
    addAndMakeVisible (midiInfoButton);

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

    presetCombo.setFactoryPresetCount (processorRef.getPresetManager().getFactoryPresetCount());
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

void TopBarComponent::mouseDoubleClick (const juce::MouseEvent&)
{
    if (auto* tlc = getTopLevelComponent())
        if (auto* peer = tlc->getPeer())
            peer->setFullScreen (! peer->isFullScreen());
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
    auto bounds = getLocalBounds().reduced (6, 2);
    int rowH = bounds.getHeight() / 2;

    // ─── Row 1: Preset navigation, combos, VRC6 toggle ──────────────────
    auto row1 = bounds.removeFromTop (rowH).reduced (0, 2);

    prevButton.setBounds (row1.removeFromLeft (26));
    row1.removeFromLeft (2);
    nextButton.setBounds (row1.removeFromLeft (26));
    row1.removeFromLeft (2);
    saveButton.setBounds (row1.removeFromLeft (40));
    row1.removeFromLeft (6);

    int presetW = juce::jmin (220, row1.getWidth() / 3);
    presetCombo.setBounds (row1.removeFromLeft (presetW));
    row1.removeFromLeft (6);
    panicButton.setBounds (row1.removeFromLeft (46));

    // Right side
    vrc6Toggle.setBounds (row1.removeFromRight (60));
    row1.removeFromRight (4);
    scaleCombo.setBounds (row1.removeFromRight (65));
    row1.removeFromRight (4);
    midiInfoButton.setBounds (row1.removeFromRight (42));
    row1.removeFromRight (4);
    midiModeCombo.setBounds (row1.removeFromRight (70));
    row1.removeFromRight (6);
    regionCombo.setBounds (row1.removeFromRight (65));

    // ─── Row 2: Sliders ─────────────────────────────────────────────────
    auto row2 = bounds.reduced (0, 2);

    int labelSpace = 28 + 34 + 26 + 24;
    int gaps = 8 * 3;
    int sliderW = (row2.getWidth() - labelSpace - gaps) / 4;

    masterVolLabel.setBounds (row2.removeFromLeft (28));
    masterVolSlider.setBounds (row2.removeFromLeft (juce::jmax (sliderW, 40)));
    row2.removeFromLeft (8);

    masterTuneLabel.setBounds (row2.removeFromLeft (34));
    masterTuneSlider.setBounds (row2.removeFromLeft (juce::jmax (sliderW, 40)));
    row2.removeFromLeft (8);

    velocitySensLabel.setBounds (row2.removeFromLeft (26));
    velocitySensSlider.setBounds (row2.removeFromLeft (juce::jmax (sliderW, 40)));
    row2.removeFromLeft (8);

    pitchBendLabel.setBounds (row2.removeFromLeft (24));
    pitchBendSlider.setBounds (row2.removeFromLeft (juce::jmax (sliderW, 40)));
}

} // namespace cart
