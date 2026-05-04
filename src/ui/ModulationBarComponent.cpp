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
    knob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    knob.setRotaryParameters (juce::MathConstants<float>::pi * 1.2f,
                              juce::MathConstants<float>::pi * 2.8f,
                              true);
    knob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 64, 14);
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

    // Check if click is on the LED area (left 36px of section) — only for sections with enable buttons
    int localX = pos.getX() - modIndex * sectionW;
    if (localX < 36 && modIndex != MOD_DPCM)
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
    using namespace cart::ui;
    auto bounds = getLocalBounds();
    int sectionW = bounds.getWidth() / NUM_MOD;

    // ─── Header background — gradient, slightly cooler than FX bar ─────
    auto headerArea = bounds.removeFromTop (headerHeight);
    g.setGradientFill (juce::ColourGradient (
        Palette::surface, 0.0f, (float) headerArea.getY(),
        Palette::background.brighter (0.05f), 0.0f, (float) headerArea.getBottom(),
        false));
    g.fillRect (headerArea);

    // Top edge — primary (cooler red than FX's hot)
    g.setColour (Palette::primary.withAlpha (0.5f));
    g.fillRect (headerArea.getX(), headerArea.getY(), headerArea.getWidth(), 2);

    const juce::String modNames[] = { "LFO", "PORTA", "ARP", "DPCM", "SEQ" };
    const juce::ToggleButton* enableButtons[] = { &lfoEnable, &portaEnable, &arpEnable, nullptr, &seqEnable };

    for (int i = 0; i < NUM_MOD; ++i)
    {
        auto section = headerArea.withX (i * sectionW).withWidth (sectionW);
        const bool hasLED   = (enableButtons[i] != nullptr);
        const bool enabled  = hasLED && enableButtons[i]->getToggleState();
        const bool expanded = (expandedMod == i);
        const bool hovered  = (hoveredSection == i);

        if (expanded)
        {
            g.setColour (Palette::surfaceHi);
            g.fillRect (section);
            g.setColour (Palette::primary);
            g.fillRect (section.getX(), section.getY(), section.getWidth(), 2);
        }
        else if (hovered)
        {
            g.setColour (Palette::surfaceHi.withAlpha (0.55f));
            g.fillRect (section);
        }

        // ─── LED ────────────────────────────────────────────────────────
        if (hasLED)
        {
            const float ledSize = 12.0f;
            auto ledBounds = section.withWidth (36).withSizeKeepingCentre ((int) ledSize, (int) ledSize).toFloat();
            if (enabled)
            {
                g.setColour (Palette::primary.withAlpha (0.18f));
                g.fillEllipse (ledBounds.expanded (5.0f));
                g.setColour (Palette::primary.withAlpha (0.32f));
                g.fillEllipse (ledBounds.expanded (2.5f));
                g.setColour (Palette::primary);
                g.fillEllipse (ledBounds);
                g.setColour (Palette::secondary.withAlpha (0.55f));
                g.fillEllipse (ledBounds.getX() + ledSize * 0.18f,
                               ledBounds.getY() + ledSize * 0.15f,
                               ledSize * 0.32f, ledSize * 0.32f);
            }
            else
            {
                g.setColour (Palette::outline);
                g.drawEllipse (ledBounds, 1.2f);
                g.setColour (Palette::surface.darker (0.3f));
                g.fillEllipse (ledBounds.reduced (1.2f));
            }
        }

        const auto textArea = (hasLED ? section.withTrimmedLeft (36)
                                      : section.withTrimmedLeft (12))
                              .withTrimmedRight (24);
        g.setColour (enabled || ! hasLED ? Palette::textPrimary : Palette::textSecondary);
        g.setFont (displayFont (12.0f));
        g.drawText (modNames[i], textArea, juce::Justification::centredLeft);

        // Chevron
        const float chevR = 4.0f;
        const float chevX = (float) section.getRight() - 16.0f;
        const float chevY = (float) section.getCentreY();
        juce::Path chev;
        if (expanded)
        {
            chev.addTriangle (chevX - chevR, chevY - chevR * 0.4f,
                              chevX + chevR, chevY - chevR * 0.4f,
                              chevX,         chevY + chevR * 0.6f);
        }
        else
        {
            chev.addTriangle (chevX - chevR * 0.4f, chevY - chevR,
                              chevX - chevR * 0.4f, chevY + chevR,
                              chevX + chevR * 0.6f, chevY);
        }
        g.setColour (expanded ? Palette::primary
                              : (hovered ? Palette::secondary : Palette::textSecondary));
        g.fillPath (chev);

        if (i < NUM_MOD - 1)
        {
            g.setColour (Palette::outlineDim.withAlpha (0.5f));
            g.drawVerticalLine ((i + 1) * sectionW, (float) headerArea.getY() + 8.0f,
                                (float) headerArea.getBottom() - 8.0f);
        }
    }

    // ─── Detail area ────────────────────────────────────────────────────
    if (expandedMod >= 0)
    {
        auto detailArea = getLocalBounds().withTop (headerHeight);
        g.setColour (Palette::surfaceHi);
        g.fillRect (detailArea);
        g.setColour (Palette::primary.withAlpha (0.7f));
        g.fillRect (0, headerHeight, getWidth(), 1);
        g.setColour (Palette::primary.withAlpha (0.18f));
        g.fillRect (0, headerHeight + 1, getWidth(), 4);
    }
}

void ModulationBarComponent::mouseMove (const juce::MouseEvent& e)
{
    int newHover = -1;
    if (e.getPosition().getY() < headerHeight)
    {
        const int sectionW = getWidth() / NUM_MOD;
        if (sectionW > 0)
            newHover = juce::jmin (e.getPosition().getX() / sectionW, (int) NUM_MOD - 1);
    }
    if (newHover != hoveredSection)
    {
        hoveredSection = newHover;
        repaint (0, 0, getWidth(), headerHeight);
    }
}

void ModulationBarComponent::mouseExit (const juce::MouseEvent&)
{
    if (hoveredSection != -1)
    {
        hoveredSection = -1;
        repaint (0, 0, getWidth(), headerHeight);
    }
}

void ModulationBarComponent::layoutDetailKnobs (juce::Rectangle<int> area, int modIndex)
{
    constexpr int colW   = 100;
    constexpr int colGap = 12;
    constexpr int labelH = 14;
    constexpr int comboH = 24;

    area = area.reduced (12, 10);

    auto centreColumns = [&] (int numColumns)
    {
        const int needed = numColumns * colW + (numColumns - 1) * colGap;
        const int offset = juce::jmax (0, (area.getWidth() - needed) / 2);
        area.removeFromLeft (offset);
    };

    auto layoutKnobCol = [&] (juce::Slider& slider, juce::Label& label)
    {
        auto col = area.removeFromLeft (colW);
        label.setJustificationType (juce::Justification::centred);
        label.setBounds (col.removeFromTop (labelH));
        slider.setBounds (col);
        if (area.getWidth() > 0) area.removeFromLeft (colGap);
    };

    auto layoutComboCol = [&] (juce::ComboBox& combo, juce::Label& label)
    {
        auto col = area.removeFromLeft (colW);
        label.setJustificationType (juce::Justification::centred);
        label.setBounds (col.removeFromTop (labelH));
        col.removeFromTop ((col.getHeight() - comboH) / 2);
        combo.setBounds (col.removeFromTop (comboH));
        if (area.getWidth() > 0) area.removeFromLeft (colGap);
    };

    switch (modIndex)
    {
        case MOD_LFO:
            centreColumns (3);
            layoutKnobCol (lfoRate,    lfoRateLabel);
            layoutKnobCol (lfoVibrato, lfoVibratoLabel);
            layoutKnobCol (lfoTremolo, lfoTremoloLabel);
            break;
        case MOD_PORTA:
            centreColumns (1);
            layoutKnobCol (portaTime, portaTimeLabel);
            break;
        case MOD_ARP:
        {
            // Pattern combo + 3 knobs + sync column
            centreColumns (5);
            layoutComboCol (arpPattern, arpPatternLabel);
            layoutKnobCol  (arpRate,    arpRateLabel);
            layoutKnobCol  (arpOctaves, arpOctavesLabel);
            layoutKnobCol  (arpGate,    arpGateLabel);

            auto syncCol = area.removeFromLeft (colW);
            syncCol.removeFromTop (labelH + 4);
            arpSyncToggle.setBounds (syncCol.removeFromTop (24));
            syncCol.removeFromTop (4);
            arpSyncDiv.setBounds (syncCol.removeFromTop (comboH));
            break;
        }
        case MOD_DPCM:
        {
            // Sample combo + Load button column
            centreColumns (1);
            auto col = area.removeFromLeft (colW * 2 + colGap);
            dpcmSampleLabel.setJustificationType (juce::Justification::centred);
            dpcmSampleLabel.setBounds (col.removeFromTop (labelH));
            col.removeFromTop ((col.getHeight() - comboH) / 2);
            auto row = col.removeFromTop (comboH);
            dpcmLoadButton.setBounds (row.removeFromRight (60));
            row.removeFromRight (8);
            dpcmSample.setBounds (row);
            break;
        }
        case MOD_SEQ:
        {
            // Top row: Rate knob, sync column on the left; step grid below fills the rest
            auto topRow = area.removeFromTop (labelH + 50);

            // Rate knob column on the left
            auto rateCol = topRow.removeFromLeft (colW);
            seqRateLabel.setJustificationType (juce::Justification::centred);
            seqRateLabel.setBounds (rateCol.removeFromTop (labelH));
            seqRate.setBounds (rateCol);
            topRow.removeFromLeft (colGap);

            // Sync toggle + combo as a stacked column
            auto syncCol = topRow.removeFromLeft (colW);
            syncCol.removeFromTop (labelH + 4);
            seqSyncToggle.setBounds (syncCol.removeFromTop (22));
            syncCol.removeFromTop (4);
            seqSyncDiv.setBounds (syncCol.removeFromTop (comboH));

            // Step grid takes the rest
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
