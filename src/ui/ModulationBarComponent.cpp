#include "ModulationBarComponent.h"
#include "Parameters.h"
#include "../PluginProcessor.h"

namespace cart {

namespace
{
    void makeKnobLabel (juce::Label& label, const juce::String& text)
    {
        label.setText (text, juce::dontSendNotification);
        label.setFont (juce::FontOptions (11.0f));
        label.setColour (juce::Label::textColourId, Colors::textSecondary);
        label.setJustificationType (juce::Justification::centred);
    }
}

ModulationBarComponent::ModulationBarComponent (juce::AudioProcessorValueTreeState& apvts, CartridgeProcessor& proc)
{
    // ─── LFO ────────────────────────────────────────────────────────────
    lfoEnable.setClickingTogglesState (true);
    addAndMakeVisible (lfoEnable);
    lfoEnableAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::LfoEnabled, lfoEnable);
    lfoEnable.setSize (0, 0);

    styleKnob (lfoRate);
    lfoRate.setTooltip ("LFO rate in Hz");
    addChildComponent (lfoRate);
    lfoRateAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::LfoRate, lfoRate);
    makeKnobLabel (lfoRateLabel, "Rate");
    addChildComponent (lfoRateLabel);

    styleKnob (lfoVibrato);
    lfoVibrato.setTooltip ("Vibrato depth (pitch modulation)");
    addChildComponent (lfoVibrato);
    lfoVibratoAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::LfoVibratoDepth, lfoVibrato);
    makeKnobLabel (lfoVibratoLabel, "Vibrato");
    addChildComponent (lfoVibratoLabel);

    styleKnob (lfoTremolo);
    lfoTremolo.setTooltip ("Tremolo depth (volume modulation)");
    addChildComponent (lfoTremolo);
    lfoTremoloAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::LfoTremoloDepth, lfoTremolo);
    makeKnobLabel (lfoTremoloLabel, "Tremolo");
    addChildComponent (lfoTremoloLabel);

    // ─── Portamento ─────────────────────────────────────────────────────
    portaEnable.setClickingTogglesState (true);
    addAndMakeVisible (portaEnable);
    portaEnableAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::PortaEnabled, portaEnable);
    portaEnable.setSize (0, 0);

    styleKnob (portaTime);
    portaTime.setTooltip ("Portamento glide time in seconds");
    addChildComponent (portaTime);
    portaTimeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::PortaTime, portaTime);
    makeKnobLabel (portaTimeLabel, "Time");
    addChildComponent (portaTimeLabel);

    // ─── Arpeggiator ────────────────────────────────────────────────────
    arpEnable.setClickingTogglesState (true);
    addAndMakeVisible (arpEnable);
    arpEnableAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::ArpEnabled, arpEnable);
    arpEnable.setSize (0, 0);

    arpPattern.addItemList ({ "Up", "Down", "Up-Down", "Random", "As Played" }, 1);
    arpPattern.setTooltip ("Arpeggiator note order pattern");
    arpPattern.setColour (juce::ComboBox::backgroundColourId, Colors::bgLight);
    arpPattern.setColour (juce::ComboBox::textColourId, Colors::textPrimary);
    arpPattern.setColour (juce::ComboBox::outlineColourId, Colors::knobOutline);
    addChildComponent (arpPattern);
    arpPatternAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        apvts, ParamIDs::ArpPattern, arpPattern);
    makeKnobLabel (arpPatternLabel, "Pattern");
    addChildComponent (arpPatternLabel);

    styleKnob (arpRate);
    arpRate.setTooltip ("Arpeggiator speed in Hz");
    addChildComponent (arpRate);
    arpRateAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::ArpRate, arpRate);
    makeKnobLabel (arpRateLabel, "Rate");
    addChildComponent (arpRateLabel);

    styleKnob (arpOctaves);
    arpOctaves.setTooltip ("Arpeggiator octave range (1-4)");
    addChildComponent (arpOctaves);
    arpOctavesAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::ArpOctaves, arpOctaves);
    makeKnobLabel (arpOctavesLabel, "Octaves");
    addChildComponent (arpOctavesLabel);

    styleKnob (arpGate);
    arpGate.setTooltip ("Arpeggiator gate length (note duration)");
    addChildComponent (arpGate);
    arpGateAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::ArpGate, arpGate);
    makeKnobLabel (arpGateLabel, "Gate");
    addChildComponent (arpGateLabel);

    // Arp tempo sync
    arpSyncToggle.setButtonText ("Sync");
    arpSyncToggle.setColour (juce::ToggleButton::textColourId, Colors::textSecondary);
    arpSyncToggle.setColour (juce::ToggleButton::tickColourId, Colors::accentActive);
    arpSyncToggle.setTooltip ("Sync arp rate to host tempo");
    addChildComponent (arpSyncToggle);
    arpSyncAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::ArpSyncEnabled, arpSyncToggle);

    arpSyncDiv.addItemList ({ "1/1", "1/2", "1/4", "1/8", "1/16", "1/32", "1/4T", "1/8T", "1/16T" }, 1);
    arpSyncDiv.setColour (juce::ComboBox::backgroundColourId, Colors::bgLight);
    arpSyncDiv.setColour (juce::ComboBox::textColourId, Colors::textPrimary);
    arpSyncDiv.setColour (juce::ComboBox::outlineColourId, Colors::knobOutline);
    arpSyncDiv.setTooltip ("Arp tempo sync note division");
    addChildComponent (arpSyncDiv);
    arpSyncDivAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        apvts, ParamIDs::ArpSyncDiv, arpSyncDiv);

    // ─── DPCM ───────────────────────────────────────────────────────────
    {
        juce::StringArray dpcmNames { "Kick", "Snare", "Hi-Hat", "Tom" };
        for (int i = 0; i < 16; ++i)
            dpcmNames.add ("User " + juce::String (i + 1));
        dpcmSample.addItemList (dpcmNames, 1);
    }
    dpcmSample.setTooltip ("Select DPCM sample to play");
    dpcmSample.setColour (juce::ComboBox::backgroundColourId, Colors::bgLight);
    dpcmSample.setColour (juce::ComboBox::textColourId, Colors::textPrimary);
    dpcmSample.setColour (juce::ComboBox::outlineColourId, Colors::knobOutline);
    addChildComponent (dpcmSample);
    dpcmSampleAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        apvts, ParamIDs::DpcmSample, dpcmSample);
    makeKnobLabel (dpcmSampleLabel, "Sample");
    addChildComponent (dpcmSampleLabel);

    // DPCM Load button
    dpcmLoadButton.setColour (juce::TextButton::buttonColourId, Colors::bgLight);
    dpcmLoadButton.setColour (juce::TextButton::textColourOffId, Colors::textSecondary);
    dpcmLoadButton.setTooltip ("Load custom DPCM sample (.dmc or .wav)");
    dpcmLoadButton.onClick = [this]
    {
        dpcmFileChooser = std::make_shared<juce::FileChooser> (
            "Load DPCM Sample",
            juce::File::getSpecialLocation (juce::File::userDesktopDirectory),
            "*.dmc;*.wav");

        dpcmFileChooser->launchAsync (
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this] (const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file.existsAsFile() && onDpcmLoad)
                    onDpcmLoad (file);
            });
    };
    addChildComponent (dpcmLoadButton);

    // ─── Step Sequencer ─────────────────────────────────────────────────
    seqEnable.setClickingTogglesState (true);
    addAndMakeVisible (seqEnable);
    seqEnableAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::SeqEnabled, seqEnable);
    seqEnable.setSize (0, 0);

    styleKnob (seqRate);
    seqRate.setTooltip ("Step sequencer rate in Hz");
    addChildComponent (seqRate);
    seqRateAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::SeqRate, seqRate);
    makeKnobLabel (seqRateLabel, "Rate");
    addChildComponent (seqRateLabel);

    seqSyncToggle.setButtonText ("Sync");
    seqSyncToggle.setColour (juce::ToggleButton::textColourId, Colors::textSecondary);
    seqSyncToggle.setColour (juce::ToggleButton::tickColourId, Colors::accentActive);
    seqSyncToggle.setTooltip ("Sync step rate to host tempo");
    addChildComponent (seqSyncToggle);
    seqSyncAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::SeqSyncEnabled, seqSyncToggle);

    seqSyncDiv.addItemList ({ "1/1", "1/2", "1/4", "1/8", "1/16", "1/32", "1/4T", "1/8T", "1/16T" }, 1);
    seqSyncDiv.setColour (juce::ComboBox::backgroundColourId, Colors::bgLight);
    seqSyncDiv.setColour (juce::ComboBox::textColourId, Colors::textPrimary);
    seqSyncDiv.setColour (juce::ComboBox::outlineColourId, Colors::knobOutline);
    seqSyncDiv.setTooltip ("Step sequencer tempo sync note division");
    addChildComponent (seqSyncDiv);
    seqSyncDivAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        apvts, ParamIDs::SeqSyncDiv, seqSyncDiv);

    stepSeqGrid = std::make_unique<StepSequencerComponent> (proc);
    addChildComponent (*stepSeqGrid);

    startTimerHz (15);
}

ModulationBarComponent::~ModulationBarComponent()
{
    stopTimer();
}

void ModulationBarComponent::timerCallback()
{
    repaint();
}

int ModulationBarComponent::getDesiredHeight() const
{
    if (expandedMod < 0) return headerHeight;
    return headerHeight + (expandedMod == MOD_SEQ ? seqDetailHeight : detailHeight);
}

void ModulationBarComponent::collapseAll()
{
    if (expandedMod >= 0)
    {
        setDetailVisible (expandedMod, false);
        expandedMod = -1;

        if (onHeightChanged)
            onHeightChanged();

        resized();
        repaint();
    }
}

void ModulationBarComponent::styleKnob (juce::Slider& knob)
{
    knob.setSliderStyle (juce::Slider::LinearHorizontal);
    knob.setTextBoxStyle (juce::Slider::TextBoxRight, false, 48, 16);
    knob.setColour (juce::Slider::trackColourId, Colors::accentActive);
    knob.setColour (juce::Slider::thumbColourId, Colors::accentActive);
    knob.setColour (juce::Slider::backgroundColourId, Colors::knobOutline);
    knob.setColour (juce::Slider::textBoxTextColourId, Colors::textPrimary);
    knob.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    knob.setPopupMenuEnabled (true);
}

void ModulationBarComponent::setDetailVisible (int modIndex, bool visible)
{
    auto setVis = [visible] (std::initializer_list<juce::Component*> comps)
    {
        for (auto* c : comps)
            c->setVisible (visible);
    };

    switch (modIndex)
    {
        case MOD_LFO:
            setVis ({ &lfoRate, &lfoVibrato, &lfoTremolo, &lfoRateLabel, &lfoVibratoLabel, &lfoTremoloLabel });
            break;
        case MOD_PORTA:
            setVis ({ &portaTime, &portaTimeLabel });
            break;
        case MOD_ARP:
            setVis ({ &arpPattern, &arpRate, &arpOctaves, &arpGate, &arpPatternLabel, &arpRateLabel, &arpOctavesLabel, &arpGateLabel, &arpSyncToggle, &arpSyncDiv });
            break;
        case MOD_DPCM:
            setVis ({ &dpcmSample, &dpcmSampleLabel, &dpcmLoadButton });
            break;
        case MOD_SEQ:
            setVis ({ &seqRate, &seqRateLabel, &seqSyncToggle, &seqSyncDiv, stepSeqGrid.get() });
            break;
        default: break;
    }
}

void ModulationBarComponent::toggleExpand (int modIndex)
{
    if (expandedMod >= 0)
        setDetailVisible (expandedMod, false);

    if (expandedMod == modIndex)
        expandedMod = -1; // collapse
    else
        expandedMod = modIndex;

    if (expandedMod >= 0)
        setDetailVisible (expandedMod, true);

    if (onHeightChanged)
        onHeightChanged();

    resized();
    repaint();
}

void ModulationBarComponent::mouseDown (const juce::MouseEvent& e)
{
    auto pos = e.getPosition();

    // Only handle clicks in the header area
    if (pos.getY() >= headerHeight)
        return;

    int sectionW = getWidth() / NUM_MOD;
    int modIndex = juce::jmin (pos.getX() / sectionW, (int) NUM_MOD - 1);

    // Check if click is on the LED area (left 28px of section) — only for sections with enable buttons
    int localX = pos.getX() - modIndex * sectionW;
    if (localX < 28 && modIndex != MOD_DPCM)
    {
        // Toggle the enable button for this mod
        juce::ToggleButton* enableButtons[] = { &lfoEnable, &portaEnable, &arpEnable, nullptr, &seqEnable };
        if (modIndex < NUM_MOD && enableButtons[modIndex] != nullptr)
        {
            enableButtons[modIndex]->setToggleState (! enableButtons[modIndex]->getToggleState(),
                                                      juce::sendNotification);
            repaint();
        }
        return;
    }

    // Click elsewhere in header → expand/collapse
    toggleExpand (modIndex);
}

void ModulationBarComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    int sectionW = bounds.getWidth() / NUM_MOD;

    // ─── Header background ─────────────────────────────────────────────
    auto headerArea = bounds.removeFromTop (headerHeight);
    g.setColour (Colors::bgMid);
    g.fillRect (headerArea);

    // Draw each header section
    const juce::String modNames[] = { "LFO", "PORTA", "ARP", "DPCM", "SEQ" };
    const juce::ToggleButton* enableButtons[] = { &lfoEnable, &portaEnable, &arpEnable, nullptr, &seqEnable };

    for (int i = 0; i < NUM_MOD; ++i)
    {
        auto section = headerArea.withX (i * sectionW).withWidth (sectionW);
        bool enabled = enableButtons[i] != nullptr && enableButtons[i]->getToggleState();
        bool expanded = (expandedMod == i);

        // Highlight expanded section
        if (expanded)
        {
            g.setColour (Colors::bgLight);
            g.fillRect (section);
        }

        if (enableButtons[i] != nullptr)
        {
            // LED circle
            auto ledBounds = section.withWidth (28).withSizeKeepingCentre (12, 12).toFloat();
            if (enabled)
            {
                g.setColour (Colors::accentActive);
                g.fillEllipse (ledBounds);
            }
            else
            {
                g.setColour (Colors::knobOutline);
                g.drawEllipse (ledBounds, 1.5f);
            }
        }

        // Mod name
        auto textArea = (enableButtons[i] != nullptr) ? section.withTrimmedLeft (28) : section;
        g.setColour (enabled ? Colors::textPrimary : Colors::textSecondary);
        g.setFont (juce::FontOptions (12.0f));

        juce::String suffix = expanded ? juce::String::charToString (0x25BC) : "";
        // DPCM uses a right-arrow since it has no enable toggle
        if (i == MOD_DPCM)
        {
            g.setColour (Colors::textSecondary);
            suffix = expanded ? juce::String::charToString (0x25BC) : juce::String::charToString (0x25B8);
        }
        g.drawText (modNames[i] + " " + suffix, textArea, juce::Justification::centredLeft);

        // Vertical divider (except after last)
        if (i < NUM_MOD - 1)
        {
            g.setColour (Colors::divider);
            g.drawVerticalLine ((i + 1) * sectionW, (float) headerArea.getY() + 4.0f,
                                (float) headerArea.getBottom() - 4.0f);
        }
    }

    // ─── Detail area background ────────────────────────────────────────
    if (expandedMod >= 0)
    {
        auto detailArea = getLocalBounds().withTop (headerHeight);
        g.setColour (Colors::bgLight);
        g.fillRect (detailArea);

        // Top border line
        g.setColour (Colors::accentActive.withAlpha (0.4f));
        g.drawHorizontalLine (headerHeight, 0.0f, (float) getWidth());
    }
}

void ModulationBarComponent::layoutDetailKnobs (juce::Rectangle<int> area, int modIndex)
{
    const int labelW = 65;
    const int comboH = 22;

    area = area.reduced (8, 4);

    auto layoutRow = [&] (juce::Rectangle<int>& a, juce::Slider& slider, juce::Label& label, int rowH)
    {
        auto row = a.removeFromTop (rowH);
        label.setJustificationType (juce::Justification::centredRight);
        label.setBounds (row.removeFromLeft (labelW));
        row.removeFromLeft (4);
        slider.setBounds (row);
    };

    switch (modIndex)
    {
        case MOD_LFO:
        {
            int rowH = area.getHeight() / 3;
            layoutRow (area, lfoRate, lfoRateLabel, rowH);
            layoutRow (area, lfoVibrato, lfoVibratoLabel, rowH);
            layoutRow (area, lfoTremolo, lfoTremoloLabel, area.getHeight());
            break;
        }
        case MOD_PORTA:
        {
            // Single slider, vertically centred
            auto centred = area.withHeight (juce::jmin (24, area.getHeight()));
            centred.setCentre (area.getCentreX(), area.getCentreY());
            portaTimeLabel.setJustificationType (juce::Justification::centredRight);
            portaTimeLabel.setBounds (centred.removeFromLeft (labelW));
            centred.removeFromLeft (4);
            portaTime.setBounds (centred);
            break;
        }
        case MOD_ARP:
        {
            int rowH = area.getHeight() / 5;

            // Pattern combo row
            auto comboRow = area.removeFromTop (rowH);
            arpPatternLabel.setJustificationType (juce::Justification::centredRight);
            arpPatternLabel.setBounds (comboRow.removeFromLeft (labelW));
            comboRow.removeFromLeft (4);
            arpPattern.setBounds (comboRow.removeFromLeft (juce::jmin (100, comboRow.getWidth())));

            layoutRow (area, arpRate, arpRateLabel, rowH);
            layoutRow (area, arpOctaves, arpOctavesLabel, rowH);
            layoutRow (area, arpGate, arpGateLabel, rowH);

            // Sync row
            auto syncRow = area;
            arpSyncToggle.setBounds (syncRow.removeFromLeft (60));
            syncRow.removeFromLeft (4);
            arpSyncDiv.setBounds (syncRow.removeFromLeft (juce::jmin (80, syncRow.getWidth()))
                                       .withHeight (juce::jmin (comboH, syncRow.getHeight())));
            break;
        }
        case MOD_DPCM:
        {
            int labelH = 16;
            auto centred = area.withSizeKeepingCentre (area.getWidth() / 2, area.getHeight());
            dpcmSampleLabel.setJustificationType (juce::Justification::centred);
            dpcmSampleLabel.setBounds (centred.removeFromTop (labelH));
            auto row = centred.removeFromTop (comboH + 4);
            int cw = juce::jmax (80, row.getWidth() - 60);
            dpcmSample.setBounds (row.removeFromLeft (cw));
            row.removeFromLeft (4);
            dpcmLoadButton.setBounds (row.removeFromLeft (50).withHeight (comboH));
            break;
        }
        case MOD_SEQ:
        {
            // Top controls row: Rate slider + Sync toggle + Sync division
            auto ctrlRow = area.removeFromTop (24);
            seqRateLabel.setJustificationType (juce::Justification::centredRight);
            seqRateLabel.setBounds (ctrlRow.removeFromLeft (labelW));
            ctrlRow.removeFromLeft (4);
            seqRate.setBounds (ctrlRow.removeFromLeft (juce::jmin (200, ctrlRow.getWidth())));
            ctrlRow.removeFromLeft (8);
            seqSyncToggle.setBounds (ctrlRow.removeFromLeft (60));
            ctrlRow.removeFromLeft (4);
            seqSyncDiv.setBounds (ctrlRow.removeFromLeft (juce::jmin (80, ctrlRow.getWidth()))
                                        .withHeight (juce::jmin (comboH, ctrlRow.getHeight())));

            // Step grid fills remaining space
            area.removeFromTop (4);
            stepSeqGrid->setBounds (area);
            break;
        }
        default: break;
    }
}

void ModulationBarComponent::resized()
{
    // Enable buttons are invisible but need valid bounds for APVTS attachment
    lfoEnable.setBounds (0, 0, 0, 0);
    portaEnable.setBounds (0, 0, 0, 0);
    arpEnable.setBounds (0, 0, 0, 0);
    seqEnable.setBounds (0, 0, 0, 0);

    if (expandedMod >= 0)
    {
        auto detailArea = getLocalBounds().withTop (headerHeight);
        layoutDetailKnobs (detailArea, expandedMod);
    }
}

} // namespace cart
