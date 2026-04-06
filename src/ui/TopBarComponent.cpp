#include "TopBarComponent.h"
#include "PluginProcessor.h"

namespace cart {

TopBarComponent::TopBarComponent (CartridgeProcessor& processor,
                                  juce::AudioProcessorValueTreeState& apvts)
    : processorRef (processor), apvtsRef (apvts)
{
    // ─── Preset Combo ────────────────────────────────────────────────────
    presetCombo.setColour (juce::ComboBox::backgroundColourId, Colors::bgLight);
    presetCombo.setColour (juce::ComboBox::textColourId, Colors::fxBright);
    presetCombo.setColour (juce::ComboBox::outlineColourId, Colors::knobOutline);
    populatePresets();
    presetCombo.onChange = [this] { selectPreset (presetCombo.getSelectedId() - 1); };
    presetCombo.setTooltip ("Select preset (Left/Right arrows to navigate)");
    addAndMakeVisible (presetCombo);

    // Prev/Next buttons
    auto styleBtn = [] (juce::TextButton& btn)
    {
        btn.setColour (juce::TextButton::buttonColourId, Colors::bgLight);
        btn.setColour (juce::TextButton::textColourOffId, Colors::textPrimary);
    };
    styleBtn (prevButton);
    styleBtn (nextButton);

    prevButton.onClick = [this] { navigatePreset (-1); };
    nextButton.onClick = [this] { navigatePreset (1); };
    prevButton.setTooltip ("Previous preset");
    nextButton.setTooltip ("Next preset");
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
    saveButton.setTooltip ("Save current settings as user preset");
    addAndMakeVisible (saveButton);

    // Import button
    styleBtn (importButton);
    importButton.onClick = [this]
    {
        auto chooser = std::make_shared<juce::FileChooser> (
            "Import Preset or Bank",
            juce::File::getSpecialLocation (juce::File::userDesktopDirectory),
            "*.cartpreset;*.cartbank;*.xml");

        chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this, chooser] (const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                if (! file.existsAsFile()) return;

                auto& pm = processorRef.getPresetManager();
                int result = -1;

                if (file.getFileExtension() == ".cartbank")
                    result = pm.importBank (file) > 0 ? pm.getNumPresets() - 1 : -1;
                else
                    result = pm.importPreset (file);

                if (result >= 0)
                {
                    populatePresets();
                    selectPreset (result);
                }
            });
    };
    importButton.setTooltip ("Import preset or bank from file");
    addAndMakeVisible (importButton);

    // Export button
    styleBtn (exportButton);
    exportButton.onClick = [this]
    {
        auto chooser = std::make_shared<juce::FileChooser> (
            "Export Preset",
            juce::File::getSpecialLocation (juce::File::userDesktopDirectory)
                .getChildFile (processorRef.getProgramName (processorRef.getCurrentProgram()) + ".cartpreset"),
            "*.cartpreset");

        chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
            [this, chooser] (const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file == juce::File{}) return;

                auto destFile = file.getFileExtension().isEmpty()
                    ? file.withFileExtension ("cartpreset")
                    : file;

                processorRef.getPresetManager().exportPreset (
                    processorRef.getProgramName (processorRef.getCurrentProgram()),
                    processorRef.getApvts(),
                    destFile);
            });
    };
    exportButton.setTooltip ("Export current preset to file");
    addAndMakeVisible (exportButton);

    // Delete button
    styleBtn (deleteButton);
    deleteButton.setTooltip ("Delete selected user preset");
    deleteButton.onClick = [this]
    {
        int idx = presetCombo.getSelectedId() - 1;
        if (processorRef.getPresetManager().isFactoryPreset (idx))
            return;

        auto* dlg = new juce::AlertWindow ("Delete Preset",
            "Delete \"" + presetCombo.getText() + "\"?",
            juce::MessageBoxIconType::WarningIcon);
        dlg->addButton ("Delete", 1);
        dlg->addButton ("Cancel", 0);
        dlg->setColour (juce::AlertWindow::backgroundColourId, Colors::bgMid);
        dlg->setColour (juce::AlertWindow::textColourId, Colors::textPrimary);
        dlg->setColour (juce::AlertWindow::outlineColourId, Colors::knobOutline);

        dlg->enterModalState (true, juce::ModalCallbackFunction::create (
            [this, dlg, idx] (int result)
            {
                if (result == 1)
                {
                    processorRef.getPresetManager().deleteUserPreset (idx);
                    populatePresets();
                    selectPreset (0);
                }
                delete dlg;
            }), false);
    };
    addAndMakeVisible (deleteButton);

    // Panic button
    panicButton.setColour (juce::TextButton::buttonColourId, Colors::bgLight);
    panicButton.setColour (juce::TextButton::textColourOffId, Colors::fxBright);
    panicButton.onClick = [this] { processorRef.getKeyboardState().allNotesOff (0); };
    panicButton.setTooltip ("Stop all notes (Space)");
    addAndMakeVisible (panicButton);

    // ─── Master Volume ───────────────────────────────────────────────────
    masterVolSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    masterVolSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    masterVolSlider.setColour (juce::Slider::trackColourId, Colors::accentDim);
    masterVolSlider.setColour (juce::Slider::thumbColourId, Colors::accentActive);
    addAndMakeVisible (masterVolSlider);
    masterVolSlider.setPopupDisplayEnabled (true, false, this);
    masterVolSlider.setPopupMenuEnabled (true);
    masterVolSlider.setTooltip ("Master output volume");
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
    masterTuneSlider.setPopupMenuEnabled (true);
    masterTuneSlider.setTooltip ("Master tuning offset in cents");
    masterTuneAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::MasterTune, masterTuneSlider);

    masterTuneLabel.setText ("Tune", juce::dontSendNotification);
    masterTuneLabel.setFont (juce::FontOptions (11.0f));
    masterTuneLabel.setColour (juce::Label::textColourId, Colors::textSecondary);
    addAndMakeVisible (masterTuneLabel);

    // ─── Region Combo ────────────────────────────────────────────────────
    regionCombo.addItem ("NTSC", 1);
    regionCombo.addItem ("PAL", 2);
    regionCombo.setTooltip ("Console region (affects frame timing and pitch tables)");
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
    midiModeCombo.addItem ("Layer", 4);
    midiModeCombo.setTooltip ("Split = per-channel MIDI, Auto = round-robin, Mono = single voice, Layer = all channels play every note");
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
    velocitySensSlider.setPopupMenuEnabled (true);
    velocitySensSlider.setTooltip ("MIDI velocity sensitivity (0 = fixed, 1 = full range)");
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
    pitchBendSlider.setPopupMenuEnabled (true);
    pitchBendSlider.setTooltip ("Pitch bend range in semitones");
    pitchBendAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::PitchBendRange, pitchBendSlider);

    pitchBendLabel.setText ("Bend", juce::dontSendNotification);
    pitchBendLabel.setFont (juce::FontOptions (11.0f));
    pitchBendLabel.setColour (juce::Label::textColourId, Colors::textSecondary);
    addAndMakeVisible (pitchBendLabel);

    // ─── Engine Mode ────────────────────────────────────────────────────
    engineModeCombo.addItem ("Classic", 1);
    engineModeCombo.addItem ("Modern", 2);
    engineModeCombo.setTooltip ("Classic = 2A03 APU channels, Modern = polyphonic synth engine");
    engineModeCombo.setColour (juce::ComboBox::backgroundColourId, Colors::bgLight);
    engineModeCombo.setColour (juce::ComboBox::textColourId, Colors::fxBright);
    engineModeCombo.setColour (juce::ComboBox::outlineColourId, Colors::knobOutline);
    addAndMakeVisible (engineModeCombo);
    engineModeAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        apvts, ParamIDs::EngineMode, engineModeCombo);

    // ─── VRC6 Toggle ─────────────────────────────────────────────────────
    vrc6Toggle.setButtonText ("VRC6");
    vrc6Toggle.setTooltip ("Enable VRC6 Konami expansion chip (3 extra channels)");
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
    scaleCombo.addItem ("100%", 1);
    scaleCombo.addItem ("125%", 2);
    scaleCombo.addItem ("150%", 3);
    scaleCombo.setTooltip ("UI scale factor");
    scaleCombo.setSelectedId (1, juce::dontSendNotification);
    scaleCombo.setColour (juce::ComboBox::backgroundColourId, Colors::bgLight);
    scaleCombo.setColour (juce::ComboBox::textColourId, Colors::textPrimary);
    scaleCombo.setColour (juce::ComboBox::outlineColourId, Colors::knobOutline);
    scaleCombo.onChange = [this]
    {
        static constexpr float scales[] = { 1.0f, 1.25f, 1.5f };
        int idx = scaleCombo.getSelectedId() - 1;
        if (idx >= 0 && idx < 3 && onScaleChanged)
            onScaleChanged (scales[idx]);
    };
    addAndMakeVisible (scaleCombo);

    // ─── MIDI Info Button ────────────────────────────────────────────────
    midiInfoButton.setColour (juce::TextButton::buttonColourId, Colors::bgLight);
    midiInfoButton.setColour (juce::TextButton::textColourOffId, Colors::textSecondary);
    midiInfoButton.setTooltip ("Show MIDI CC mapping reference");
    midiInfoButton.onClick = [this]
    {
        juce::String info =
            "CC 1   Vibrato Depth\n"
            "CC 11  Master Volume\n"
            "CC 64  Sustain Pedal\n"
            "CC 71  Filter Resonance\n"
            "CC 74  Filter Cutoff\n"
            "CC 91  Reverb Mix\n"
            "CC 93  Chorus Mix";

        // Show user CC mappings
        static const char* ccTargetNames[] = {
            "None", "Master Volume", "Pulse 1 Volume", "Pulse 2 Volume",
            "Noise Volume", "VRC6 P1 Volume", "VRC6 P2 Volume",
            "Filter Cutoff", "Filter Resonance", "Chorus Mix",
            "Delay Mix", "Reverb Mix", "LFO Rate", "Vibrato Depth", "Tremolo Depth"
        };

        const char* ccNumIDs[] = { cart::ParamIDs::UserCC1Num, cart::ParamIDs::UserCC2Num,
                                    cart::ParamIDs::UserCC3Num, cart::ParamIDs::UserCC4Num };
        const char* ccTargetIDs[] = { cart::ParamIDs::UserCC1Target, cart::ParamIDs::UserCC2Target,
                                       cart::ParamIDs::UserCC3Target, cart::ParamIDs::UserCC4Target };

        bool hasUserMappings = false;
        for (int i = 0; i < 4; ++i)
        {
            int target = static_cast<int> (*apvtsRef.getRawParameterValue (ccTargetIDs[i]));
            if (target > 0 && target < 15)
            {
                if (!hasUserMappings)
                {
                    info += "\n\n--- User Mappings ---\n";
                    hasUserMappings = true;
                }
                int ccNum = static_cast<int> (*apvtsRef.getRawParameterValue (ccNumIDs[i]));
                info += "CC " + juce::String (ccNum) + "  " + ccTargetNames[target] + "\n";
            }
        }

        info += "\n\nTo learn a CC: click Learn 1-4, then turn a MIDI knob.";

        auto* popup = new juce::AlertWindow ("MIDI CC Mappings", "", juce::MessageBoxIconType::NoIcon);
        popup->setColour (juce::AlertWindow::backgroundColourId, Colors::bgMid);
        popup->setColour (juce::AlertWindow::textColourId, Colors::textPrimary);
        popup->setColour (juce::AlertWindow::outlineColourId, Colors::knobOutline);
        popup->addTextBlock (info);
        popup->addButton ("Learn 1", 1);
        popup->addButton ("Learn 2", 2);
        popup->addButton ("Learn 3", 3);
        popup->addButton ("Learn 4", 4);
        popup->addButton ("OK", 0);
        popup->enterModalState (true, juce::ModalCallbackFunction::create (
            [this, popup] (int result)
            {
                if (result >= 1 && result <= 4)
                    processorRef.setMidiLearnSlot (result - 1);
                delete popup;
            }), false);
    };
    addAndMakeVisible (midiInfoButton);

    // ─── Audio/MIDI Settings (Standalone only) ──────────────────────────
    audioSettingsButton.setButtonText ("Settings");
    audioSettingsButton.setColour (juce::TextButton::buttonColourId, Colors::bgLight);
    audioSettingsButton.setColour (juce::TextButton::textColourOffId, Colors::textSecondary);
    audioSettingsButton.setTooltip ("Audio & MIDI device settings");
    audioSettingsButton.onClick = [this]
    {
        if (onAudioSettings)
            onAudioSettings();
    };
    // Starts hidden; editor shows it if running standalone
    audioSettingsButton.setVisible (false);

    // ─── A/B Comparison ─────────────────────────────────────────────────
    styleBtn(abButton);
    abButton.setColour(juce::TextButton::textColourOffId, Colors::fxBright);
    abButton.setTooltip ("Toggle A/B comparison (stores two parameter states)");
    abButton.onClick = [this]
    {
        processorRef.abCompare.toggle(processorRef.getApvts());
        processorRef.triggerDspReset();
        abButton.setButtonText(processorRef.abCompare.isShowingA() ? "A" : "B");
    };
    addAndMakeVisible(abButton);

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
    auto& pm = processorRef.getPresetManager();
    int numPresets = processorRef.getNumPrograms();

    for (int i = 0; i < numPresets; ++i)
        presetCombo.addItem (processorRef.getProgramName (i), i + 1);

    presetCombo.setFactoryPresetCount (pm.getFactoryPresetCount());

    // Build engine mode vector for factory presets
    int factoryCount = pm.getFactoryPresetCount();
    std::vector<int> modes;
    modes.reserve (static_cast<size_t> (factoryCount));
    for (int i = 0; i < factoryCount; ++i)
        modes.push_back (pm.getPresetEngineMode (i));
    presetCombo.setPresetEngineModes (std::move (modes));

    // Set current engine mode on combo
    int engineMode = engineModeCombo.getSelectedId();
    presetCombo.setEngineMode (engineMode >= 1 ? engineMode - 1 : 0);

    refreshUserCategories();
    presetCombo.setSelectedId (processorRef.getCurrentProgram() + 1, juce::dontSendNotification);
}

void TopBarComponent::refreshUserCategories()
{
    auto& pm = processorRef.getPresetManager();
    int factoryCount = pm.getFactoryPresetCount();
    int total = pm.getNumPresets();

    auto catNames = pm.getUserCategories();

    std::vector<PresetComboBox::UserCategory> cats;

    // Build category groups
    for (const auto& catName : catNames)
    {
        PresetComboBox::UserCategory uc;
        uc.name = catName;

        for (int i = factoryCount; i < total; ++i)
        {
            if (auto* p = pm.getPreset (i))
            {
                if (p->category == catName)
                    uc.indices.push_back (i);
            }
        }

        cats.push_back (std::move (uc));
    }

    // Uncategorized user presets
    PresetComboBox::UserCategory uncategorized;
    for (int i = factoryCount; i < total; ++i)
    {
        if (auto* p = pm.getPreset (i))
        {
            if (p->category.isEmpty())
                uncategorized.indices.push_back (i);
        }
    }
    if (! uncategorized.indices.empty())
        cats.push_back (std::move (uncategorized));

    presetCombo.setUserCategories (std::move (cats));
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

    // Skip presets that don't match the current engine mode
    for (int attempt = 0; attempt < total; ++attempt)
    {
        cur = (cur + delta + total) % total;
        if (presetCombo.isPresetVisibleForMode (cur))
        {
            selectPreset (cur);
            return;
        }
    }
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

    // Update delete button enabled state
    bool isFactory = processorRef.getPresetManager().isFactoryPreset (current);
    deleteButton.setEnabled (!isFactory);
    deleteButton.setAlpha (isFactory ? 0.4f : 1.0f);

    // Only fire VRC6 toggle callback when state actually changes
    bool vrc6On = vrc6Toggle.getToggleState();
    if (vrc6On != lastVrc6State)
    {
        lastVrc6State = vrc6On;
        if (onVrc6Toggle)
            onVrc6Toggle (vrc6On);
    }

    // Engine mode change
    int engineMode = engineModeCombo.getSelectedId();
    if (engineMode != lastEngineMode)
    {
        lastEngineMode = engineMode;
        presetCombo.setEngineMode (engineMode >= 1 ? engineMode - 1 : 0);
        if (onEngineToggle)
            onEngineToggle (engineMode == 2);  // 2 = Modern

        bool isModern = (engineMode == 2);
        vrc6Toggle.setEnabled (!isModern);
        vrc6Toggle.setAlpha (isModern ? 0.4f : 1.0f);
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
    auto bounds = getLocalBounds().reduced (6, 3);
    int rowH = bounds.getHeight() / 3;

    // ─── Row 1: Preset navigation + A/B ─────────────────────────────────
    auto row1 = bounds.removeFromTop (rowH).reduced (0, 1);

    // Left: [<][>] [===preset===]  then action buttons spread across remaining space
    {
        // Nav arrows side by side (no gap between them)
        prevButton.setBounds (row1.removeFromLeft (24));
        nextButton.setBounds (row1.removeFromLeft (24));
        row1.removeFromLeft (4);

        // Preset dropdown — wider, top-left anchor
        presetCombo.setBounds (row1.removeFromLeft (280));
        row1.removeFromLeft (8);

        // Action buttons fill remaining space equally with small gaps
        constexpr int numBtns = 6;
        constexpr int btnGap = 4;
        int btnW = (row1.getWidth() - btnGap * (numBtns - 1)) / numBtns;

        saveButton.setBounds   (row1.removeFromLeft (btnW)); row1.removeFromLeft (btnGap);
        importButton.setBounds (row1.removeFromLeft (btnW)); row1.removeFromLeft (btnGap);
        exportButton.setBounds (row1.removeFromLeft (btnW)); row1.removeFromLeft (btnGap);
        deleteButton.setBounds (row1.removeFromLeft (btnW)); row1.removeFromLeft (btnGap);
        panicButton.setBounds  (row1.removeFromLeft (btnW)); row1.removeFromLeft (btnGap);
        abButton.setBounds     (row1);
    }

    // ─── Row 2: Engine, Region, MIDI mode, VRC6, Scale, MIDI info ───────
    auto row2 = bounds.removeFromTop (rowH).reduced (0, 1);

    engineModeCombo.setBounds (row2.removeFromLeft (78));
    row2.removeFromLeft (4);
    regionCombo.setBounds (row2.removeFromLeft (65));
    row2.removeFromLeft (4);
    midiModeCombo.setBounds (row2.removeFromLeft (65));
    row2.removeFromLeft (6);
    vrc6Toggle.setBounds (row2.removeFromLeft (60));

    // Right side of row 2
    if (audioSettingsButton.isVisible())
    {
        audioSettingsButton.setBounds (row2.removeFromRight (62));
        row2.removeFromRight (4);
    }
    midiInfoButton.setBounds (row2.removeFromRight (42));
    row2.removeFromRight (4);
    scaleCombo.setBounds (row2.removeFromRight (65));

    // ─── Row 3: Sliders ─────────────────────────────────────────────────
    auto row3 = bounds.reduced (0, 1);

    int labelSpace = 28 + 34 + 26 + 34;
    int gaps = 8 * 3;
    int sliderW = (row3.getWidth() - labelSpace - gaps) / 4;

    masterVolLabel.setBounds (row3.removeFromLeft (28));
    masterVolSlider.setBounds (row3.removeFromLeft (juce::jmax (sliderW, 40)));
    row3.removeFromLeft (8);

    masterTuneLabel.setBounds (row3.removeFromLeft (34));
    masterTuneSlider.setBounds (row3.removeFromLeft (juce::jmax (sliderW, 40)));
    row3.removeFromLeft (8);

    velocitySensLabel.setBounds (row3.removeFromLeft (26));
    velocitySensSlider.setBounds (row3.removeFromLeft (juce::jmax (sliderW, 40)));
    row3.removeFromLeft (8);

    pitchBendLabel.setBounds (row3.removeFromLeft (34));
    pitchBendSlider.setBounds (row3.removeFromLeft (juce::jmax (sliderW, 40)));
}

} // namespace cart
