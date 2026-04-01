#include "ModulationBarComponent.h"
#include "Parameters.h"

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

ModulationBarComponent::ModulationBarComponent (juce::AudioProcessorValueTreeState& apvts)
{
    // ─── LFO ────────────────────────────────────────────────────────────
    lfoEnable.setClickingTogglesState (true);
    addAndMakeVisible (lfoEnable);
    lfoEnableAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::LfoEnabled, lfoEnable);
    lfoEnable.setSize (0, 0);

    styleKnob (lfoRate);
    addChildComponent (lfoRate);
    lfoRateAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::LfoRate, lfoRate);
    makeKnobLabel (lfoRateLabel, "Rate");
    addChildComponent (lfoRateLabel);

    styleKnob (lfoVibrato);
    addChildComponent (lfoVibrato);
    lfoVibratoAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::LfoVibratoDepth, lfoVibrato);
    makeKnobLabel (lfoVibratoLabel, "Vibrato");
    addChildComponent (lfoVibratoLabel);

    styleKnob (lfoTremolo);
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
    arpPattern.setColour (juce::ComboBox::backgroundColourId, Colors::bgLight);
    arpPattern.setColour (juce::ComboBox::textColourId, Colors::textPrimary);
    arpPattern.setColour (juce::ComboBox::outlineColourId, Colors::knobOutline);
    addChildComponent (arpPattern);
    arpPatternAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        apvts, ParamIDs::ArpPattern, arpPattern);
    makeKnobLabel (arpPatternLabel, "Pattern");
    addChildComponent (arpPatternLabel);

    styleKnob (arpRate);
    addChildComponent (arpRate);
    arpRateAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::ArpRate, arpRate);
    makeKnobLabel (arpRateLabel, "Rate");
    addChildComponent (arpRateLabel);

    styleKnob (arpOctaves);
    addChildComponent (arpOctaves);
    arpOctavesAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::ArpOctaves, arpOctaves);
    makeKnobLabel (arpOctavesLabel, "Octaves");
    addChildComponent (arpOctavesLabel);

    styleKnob (arpGate);
    addChildComponent (arpGate);
    arpGateAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::ArpGate, arpGate);
    makeKnobLabel (arpGateLabel, "Gate");
    addChildComponent (arpGateLabel);

    // ─── DPCM ───────────────────────────────────────────────────────────
    {
        juce::StringArray dpcmNames { "Kick", "Snare", "Hi-Hat", "Tom" };
        for (int i = 0; i < 16; ++i)
            dpcmNames.add ("User " + juce::String (i + 1));
        dpcmSample.addItemList (dpcmNames, 1);
    }
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
    return headerHeight + (expandedMod >= 0 ? detailHeight : 0);
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
    knob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 56, 14);
    knob.setColour (juce::Slider::rotarySliderFillColourId, Colors::accentActive);
    knob.setColour (juce::Slider::rotarySliderOutlineColourId, Colors::knobOutline);
    knob.setColour (juce::Slider::thumbColourId, Colors::accentActive);
    knob.setColour (juce::Slider::textBoxTextColourId, Colors::textPrimary);
    knob.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
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
            setVis ({ &arpPattern, &arpRate, &arpOctaves, &arpGate, &arpPatternLabel, &arpRateLabel, &arpOctavesLabel, &arpGateLabel });
            break;
        case MOD_DPCM:
            setVis ({ &dpcmSample, &dpcmSampleLabel, &dpcmLoadButton });
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
        juce::ToggleButton* enableButtons[] = { &lfoEnable, &portaEnable, &arpEnable };
        if (modIndex < 3)
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
    const juce::String modNames[] = { "LFO", "PORTA", "ARP", "DPCM" };
    const juce::ToggleButton* enableButtons[] = { &lfoEnable, &portaEnable, &arpEnable, nullptr };

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
    const int knobSize = 48;
    const int labelH = 16;
    const int comboH = 24;

    auto layoutKnobColumn = [&] (juce::Rectangle<int> col, juce::Slider& knob, juce::Label& label)
    {
        auto labelArea = col.removeFromTop (labelH);
        label.setBounds (labelArea);
        knob.setBounds (col.withSizeKeepingCentre (knobSize, juce::jmin (knobSize + 14, col.getHeight())));
    };

    area = area.reduced (8, 4);

    switch (modIndex)
    {
        case MOD_LFO:
        {
            int colW = area.getWidth() / 3;
            layoutKnobColumn (area.removeFromLeft (colW), lfoRate, lfoRateLabel);
            layoutKnobColumn (area.removeFromLeft (colW), lfoVibrato, lfoVibratoLabel);
            layoutKnobColumn (area, lfoTremolo, lfoTremoloLabel);
            break;
        }
        case MOD_PORTA:
        {
            // Center the single knob
            layoutKnobColumn (area.withSizeKeepingCentre (area.getWidth() / 3, area.getHeight()),
                              portaTime, portaTimeLabel);
            break;
        }
        case MOD_ARP:
        {
            int colW = area.getWidth() / 4;
            // Combo gets a column — use most of the column width
            auto comboCol = area.removeFromLeft (colW);
            arpPatternLabel.setBounds (comboCol.removeFromTop (labelH));
            int cw = juce::jmax (80, comboCol.getWidth() - 12);
            arpPattern.setBounds (comboCol.withSizeKeepingCentre (cw, comboH));

            layoutKnobColumn (area.removeFromLeft (colW), arpRate, arpRateLabel);
            layoutKnobColumn (area.removeFromLeft (colW), arpOctaves, arpOctavesLabel);
            layoutKnobColumn (area, arpGate, arpGateLabel);
            break;
        }
        case MOD_DPCM:
        {
            auto comboCol = area.withSizeKeepingCentre (area.getWidth() / 2, area.getHeight());
            dpcmSampleLabel.setBounds (comboCol.removeFromTop (labelH));
            auto row = comboCol.removeFromTop (comboH + 4);
            int cw = juce::jmax (80, row.getWidth() - 60);
            dpcmSample.setBounds (row.removeFromLeft (cw));
            row.removeFromLeft (4);
            dpcmLoadButton.setBounds (row.removeFromLeft (50).withHeight (comboH));
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

    if (expandedMod >= 0)
    {
        auto detailArea = getLocalBounds().withTop (headerHeight);
        layoutDetailKnobs (detailArea, expandedMod);
    }
}

} // namespace cart
