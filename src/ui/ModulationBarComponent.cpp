#include "ModulationBarComponent.h"
#include "Parameters.h"
#include "../PluginProcessor.h"

namespace cart {

using namespace cart::ui;

namespace
{
    void makeKnobLabel (juce::Label& label, const juce::String& text)
    {
        label.setText (text, juce::dontSendNotification);
        label.setFont (labelFont (Metrics::fontValue));
        label.setColour (juce::Label::textColourId, Palette::textSecondary);
        label.setJustificationType (juce::Justification::centred);
    }
}

void ModulationBarComponent::configureLedToggle (juce::ToggleButton& toggle,
                                                 const juce::String& tooltipText)
{
    addAndMakeVisible (toggle);
    toggle.setClickingTogglesState (true);
    toggle.setLookAndFeel (&invisibleToggleLaf);
    toggle.setWantsKeyboardFocus (true);
    toggle.setTooltip (tooltipText);
}

ModulationBarComponent::ModulationBarComponent (juce::AudioProcessorValueTreeState& apvts, CartridgeProcessor& proc)
{
    // ─── LFO ────────────────────────────────────────────────────────────
    configureLedToggle (lfoEnable, "Enable LFO");
    lfoEnableAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::LfoEnabled, lfoEnable);

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
    configureLedToggle (portaEnable, "Enable Portamento");
    portaEnableAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::PortaEnabled, portaEnable);

    styleKnob (portaTime);
    portaTime.setTooltip ("Portamento glide time in seconds");
    addChildComponent (portaTime);
    portaTimeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::PortaTime, portaTime);
    makeKnobLabel (portaTimeLabel, "Time");
    addChildComponent (portaTimeLabel);

    // ─── Arpeggiator ────────────────────────────────────────────────────
    configureLedToggle (arpEnable, "Enable Arpeggiator");
    arpEnableAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::ArpEnabled, arpEnable);

    arpPattern.addItemList ({ "Up", "Down", "Up-Down", "Random", "As Played" }, 1);
    arpPattern.setTooltip ("Arpeggiator note order pattern");
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
    arpSyncToggle.setColour (juce::ToggleButton::textColourId, Palette::textSecondary);
    arpSyncToggle.setColour (juce::ToggleButton::tickColourId, Palette::primary);
    arpSyncToggle.setTooltip ("Sync arp rate to host tempo");
    addChildComponent (arpSyncToggle);
    arpSyncAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::ArpSyncEnabled, arpSyncToggle);

    arpSyncDiv.addItemList ({ "1/1", "1/2", "1/4", "1/8", "1/16", "1/32", "1/4T", "1/8T", "1/16T" }, 1);
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
    addChildComponent (dpcmSample);
    dpcmSampleAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        apvts, ParamIDs::DpcmSample, dpcmSample);
    makeKnobLabel (dpcmSampleLabel, "Sample");
    addChildComponent (dpcmSampleLabel);

    // DPCM Load button
    dpcmLoadButton.setColour (juce::TextButton::buttonColourId, Palette::surfaceHi);
    dpcmLoadButton.setColour (juce::TextButton::textColourOffId, Palette::textSecondary);
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
    configureLedToggle (seqEnable, "Enable Step Sequencer");
    seqEnableAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::SeqEnabled, seqEnable);

    styleKnob (seqRate);
    seqRate.setTooltip ("Step sequencer rate in Hz");
    addChildComponent (seqRate);
    seqRateAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::SeqRate, seqRate);
    makeKnobLabel (seqRateLabel, "Rate");
    addChildComponent (seqRateLabel);

    seqSyncToggle.setButtonText ("Sync");
    seqSyncToggle.setColour (juce::ToggleButton::textColourId, Palette::textSecondary);
    seqSyncToggle.setColour (juce::ToggleButton::tickColourId, Palette::primary);
    seqSyncToggle.setTooltip ("Sync step rate to host tempo");
    addChildComponent (seqSyncToggle);
    seqSyncAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::SeqSyncEnabled, seqSyncToggle);

    seqSyncDiv.addItemList ({ "1/1", "1/2", "1/4", "1/8", "1/16", "1/32", "1/4T", "1/8T", "1/16T" }, 1);
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

    // LED toggles use our InvisibleToggleLAF — detach before the LAF dies
    lfoEnable.setLookAndFeel (nullptr);
    portaEnable.setLookAndFeel (nullptr);
    arpEnable.setLookAndFeel (nullptr);
    seqEnable.setLookAndFeel (nullptr);
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

    // Only handle clicks in the header area. LED toggles are real children
    // with their own hit testing — JUCE delivers their clicks before this.
    if (pos.getY() >= headerHeight)
        return;

    int sectionW = getWidth() / NUM_MOD;
    int modIndex = juce::jmin (pos.getX() / sectionW, (int) NUM_MOD - 1);

    // Click anywhere else in the header → expand/collapse
    toggleExpand (modIndex);
}

void ModulationBarComponent::paint (juce::Graphics& g)
{
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
                // Mod bar breathes a touch faster than FX so they don't
                // pulse in unison — the eye reads them as separate sources.
                const float t = (float) (juce::Time::getMillisecondCounter() % 3200) / 3200.0f;
                const float breathe = 0.5f + 0.5f * std::sin (t * juce::MathConstants<float>::twoPi);
                const float haloA  = 0.14f + 0.14f * breathe;
                const float haloA2 = 0.26f + 0.10f * breathe;

                g.setColour (Palette::primary.withAlpha (haloA));
                g.fillEllipse (ledBounds.expanded (5.5f));
                g.setColour (Palette::primary.withAlpha (haloA2));
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
        g.setFont (displayFont (Metrics::fontLabelSmall));
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
    // LED toggles get a 32×32 hit target centred over the painted LED in each
    // section header. The painted LED lives inside the leftmost 36px of the
    // section, vertically centred in the header band.
    const int sectionW = juce::jmax (1, getWidth() / NUM_MOD);
    const int hit      = Metrics::channelHeaderLedHit;
    const int ledColX  = 18; // centre of the painted LED within its section
    const int ledCY    = headerHeight / 2;

    auto placeLed = [&] (juce::ToggleButton& t, int sectionIndex)
    {
        const int cx = sectionIndex * sectionW + ledColX;
        t.setBounds (cx - hit / 2, ledCY - hit / 2, hit, hit);
    };

    placeLed (lfoEnable,   MOD_LFO);
    placeLed (portaEnable, MOD_PORTA);
    placeLed (arpEnable,   MOD_ARP);
    placeLed (seqEnable,   MOD_SEQ);

    if (expandedMod >= 0)
    {
        auto detailArea = getLocalBounds().withTop (headerHeight);
        layoutDetailKnobs (detailArea, expandedMod);
    }
}

} // namespace cart
