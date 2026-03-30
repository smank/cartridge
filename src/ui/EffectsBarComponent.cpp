#include "EffectsBarComponent.h"
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

EffectsBarComponent::EffectsBarComponent (juce::AudioProcessorValueTreeState& apvts)
{
    // ─── BitCrush ──────────────────────────────────────────────────────
    bcEnable.setClickingTogglesState (true);
    addAndMakeVisible (bcEnable);
    bcEnableAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::BcEnabled, bcEnable);
    bcEnable.setSize (0, 0); // invisible; we draw LED manually

    styleKnob (bcBitDepth);
    addChildComponent (bcBitDepth);
    bcBitDepthAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::BcBitDepth, bcBitDepth);
    makeKnobLabel (bcBitDepthLabel, "Bit Depth");
    addChildComponent (bcBitDepthLabel);

    styleKnob (bcRate);
    addChildComponent (bcRate);
    bcRateAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::BcRateReduce, bcRate);
    makeKnobLabel (bcRateLabel, "Rate");
    addChildComponent (bcRateLabel);

    styleKnob (bcMix);
    addChildComponent (bcMix);
    bcMixAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::BcMix, bcMix);
    makeKnobLabel (bcMixLabel, "Mix");
    addChildComponent (bcMixLabel);

    // ─── Filter ────────────────────────────────────────────────────────
    fltEnable.setClickingTogglesState (true);
    addAndMakeVisible (fltEnable);
    fltEnableAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::FltEnabled, fltEnable);
    fltEnable.setSize (0, 0);

    fltType.addItemList ({ "LP", "BP", "HP" }, 1);
    fltType.setColour (juce::ComboBox::backgroundColourId, Colors::bgLight);
    fltType.setColour (juce::ComboBox::textColourId, Colors::textPrimary);
    fltType.setColour (juce::ComboBox::outlineColourId, Colors::knobOutline);
    addChildComponent (fltType);
    fltTypeAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        apvts, ParamIDs::FltType, fltType);
    makeKnobLabel (fltTypeLabel, "Type");
    addChildComponent (fltTypeLabel);

    styleKnob (fltCutoff);
    addChildComponent (fltCutoff);
    fltCutoffAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::FltCutoff, fltCutoff);
    makeKnobLabel (fltCutoffLabel, "Cutoff");
    addChildComponent (fltCutoffLabel);

    styleKnob (fltResonance);
    addChildComponent (fltResonance);
    fltResonanceAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::FltResonance, fltResonance);
    makeKnobLabel (fltResonanceLabel, "Resonance");
    addChildComponent (fltResonanceLabel);

    // ─── Chorus ────────────────────────────────────────────────────────
    chEnable.setClickingTogglesState (true);
    addAndMakeVisible (chEnable);
    chEnableAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::ChEnabled, chEnable);
    chEnable.setSize (0, 0);

    styleKnob (chRate);
    addChildComponent (chRate);
    chRateAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::ChRate, chRate);
    makeKnobLabel (chRateLabel, "Rate");
    addChildComponent (chRateLabel);

    styleKnob (chDepth);
    addChildComponent (chDepth);
    chDepthAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::ChDepth, chDepth);
    makeKnobLabel (chDepthLabel, "Depth");
    addChildComponent (chDepthLabel);

    styleKnob (chMix);
    addChildComponent (chMix);
    chMixAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::ChMix, chMix);
    makeKnobLabel (chMixLabel, "Mix");
    addChildComponent (chMixLabel);

    // ─── Delay ─────────────────────────────────────────────────────────
    dlEnable.setClickingTogglesState (true);
    addAndMakeVisible (dlEnable);
    dlEnableAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::DlEnabled, dlEnable);
    dlEnable.setSize (0, 0);

    styleKnob (dlTime);
    addChildComponent (dlTime);
    dlTimeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::DlTime, dlTime);
    makeKnobLabel (dlTimeLabel, "Time");
    addChildComponent (dlTimeLabel);

    styleKnob (dlFeedback);
    addChildComponent (dlFeedback);
    dlFeedbackAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::DlFeedback, dlFeedback);
    makeKnobLabel (dlFeedbackLabel, "Feedback");
    addChildComponent (dlFeedbackLabel);

    styleKnob (dlMix);
    addChildComponent (dlMix);
    dlMixAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::DlMix, dlMix);
    makeKnobLabel (dlMixLabel, "Mix");
    addChildComponent (dlMixLabel);

    // ─── Reverb ────────────────────────────────────────────────────────
    rvEnable.setClickingTogglesState (true);
    addAndMakeVisible (rvEnable);
    rvEnableAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::RvEnabled, rvEnable);
    rvEnable.setSize (0, 0);

    styleKnob (rvSize);
    addChildComponent (rvSize);
    rvSizeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::RvSize, rvSize);
    makeKnobLabel (rvSizeLabel, "Size");
    addChildComponent (rvSizeLabel);

    styleKnob (rvDamping);
    addChildComponent (rvDamping);
    rvDampingAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::RvDamping, rvDamping);
    makeKnobLabel (rvDampingLabel, "Damp");
    addChildComponent (rvDampingLabel);

    styleKnob (rvWidth);
    addChildComponent (rvWidth);
    rvWidthAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::RvWidth, rvWidth);
    makeKnobLabel (rvWidthLabel, "Width");
    addChildComponent (rvWidthLabel);

    styleKnob (rvMix);
    addChildComponent (rvMix);
    rvMixAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::RvMix, rvMix);
    makeKnobLabel (rvMixLabel, "Mix");
    addChildComponent (rvMixLabel);

    startTimerHz (15);
}

EffectsBarComponent::~EffectsBarComponent()
{
    stopTimer();
}

void EffectsBarComponent::timerCallback()
{
    repaint();
}

int EffectsBarComponent::getDesiredHeight() const
{
    return headerHeight + (expandedEffect >= 0 ? detailHeight : 0);
}

void EffectsBarComponent::collapseAll()
{
    if (expandedEffect >= 0)
    {
        setDetailVisible (expandedEffect, false);
        expandedEffect = -1;

        if (onHeightChanged)
            onHeightChanged();

        resized();
        repaint();
    }
}

void EffectsBarComponent::styleKnob (juce::Slider& knob)
{
    knob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    knob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 56, 14);
    knob.setColour (juce::Slider::rotarySliderFillColourId, Colors::fxAccent);
    knob.setColour (juce::Slider::rotarySliderOutlineColourId, Colors::knobOutline);
    knob.setColour (juce::Slider::thumbColourId, Colors::fxAccent);
    knob.setColour (juce::Slider::textBoxTextColourId, Colors::textPrimary);
    knob.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
}

void EffectsBarComponent::setDetailVisible (int fxIndex, bool visible)
{
    auto setVis = [visible] (std::initializer_list<juce::Component*> comps)
    {
        for (auto* c : comps)
            c->setVisible (visible);
    };

    switch (fxIndex)
    {
        case FX_CRUSH:
            setVis ({ &bcBitDepth, &bcRate, &bcMix, &bcBitDepthLabel, &bcRateLabel, &bcMixLabel });
            break;
        case FX_FILTER:
            setVis ({ &fltType, &fltCutoff, &fltResonance, &fltTypeLabel, &fltCutoffLabel, &fltResonanceLabel });
            break;
        case FX_CHORUS:
            setVis ({ &chRate, &chDepth, &chMix, &chRateLabel, &chDepthLabel, &chMixLabel });
            break;
        case FX_DELAY:
            setVis ({ &dlTime, &dlFeedback, &dlMix, &dlTimeLabel, &dlFeedbackLabel, &dlMixLabel });
            break;
        case FX_REVERB:
            setVis ({ &rvSize, &rvDamping, &rvWidth, &rvMix, &rvSizeLabel, &rvDampingLabel, &rvWidthLabel, &rvMixLabel });
            break;
        default: break;
    }
}

void EffectsBarComponent::toggleExpand (int fxIndex)
{
    if (expandedEffect >= 0)
        setDetailVisible (expandedEffect, false);

    if (expandedEffect == fxIndex)
        expandedEffect = -1; // collapse
    else
        expandedEffect = fxIndex;

    if (expandedEffect >= 0)
        setDetailVisible (expandedEffect, true);

    if (onHeightChanged)
        onHeightChanged();

    resized();
    repaint();
}

void EffectsBarComponent::mouseDown (const juce::MouseEvent& e)
{
    auto pos = e.getPosition();

    // Only handle clicks in the header area
    if (pos.getY() >= headerHeight)
        return;

    int sectionW = getWidth() / NUM_FX;
    int fxIndex = juce::jmin (pos.getX() / sectionW, (int) NUM_FX - 1);

    // Check if click is on the LED area (left 28px of section)
    int localX = pos.getX() - fxIndex * sectionW;
    if (localX < 28)
    {
        // Toggle the enable button for this effect
        juce::ToggleButton* enableButtons[] = { &bcEnable, &fltEnable, &chEnable, &dlEnable, &rvEnable };
        enableButtons[fxIndex]->setToggleState (! enableButtons[fxIndex]->getToggleState(),
                                                 juce::sendNotification);
        repaint();
        return;
    }

    // Click elsewhere in header → expand/collapse
    toggleExpand (fxIndex);
}

void EffectsBarComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    int sectionW = bounds.getWidth() / NUM_FX;

    // ─── Header background ─────────────────────────────────────────────
    auto headerArea = bounds.removeFromTop (headerHeight);
    g.setColour (Colors::bgMid);
    g.fillRect (headerArea);

    // Draw each header section
    const juce::String fxNames[] = { "CRUSH", "FILTER", "CHORUS", "DELAY", "REVERB" };
    const juce::ToggleButton* enableButtons[] = { &bcEnable, &fltEnable, &chEnable, &dlEnable, &rvEnable };

    for (int i = 0; i < NUM_FX; ++i)
    {
        auto section = headerArea.withX (i * sectionW).withWidth (sectionW);
        bool enabled = enableButtons[i]->getToggleState();
        bool expanded = (expandedEffect == i);

        // Highlight expanded section
        if (expanded)
        {
            g.setColour (Colors::bgLight);
            g.fillRect (section);
        }

        // LED circle
        auto ledBounds = section.withWidth (28).withSizeKeepingCentre (12, 12).toFloat();
        if (enabled)
        {
            g.setColour (Colors::fxAccent);
            g.fillEllipse (ledBounds);
        }
        else
        {
            g.setColour (Colors::knobOutline);
            g.drawEllipse (ledBounds, 1.5f);
        }

        // Effect name
        auto textArea = section.withTrimmedLeft (28);
        g.setColour (enabled ? Colors::textPrimary : Colors::textSecondary);
        g.setFont (juce::FontOptions (12.0f));
        g.drawText (fxNames[i] + (expanded ? juce::String::charToString (0x25BC) : ""),
                    textArea, juce::Justification::centredLeft);

        // Vertical divider (except after last)
        if (i < NUM_FX - 1)
        {
            g.setColour (Colors::divider);
            g.drawVerticalLine ((i + 1) * sectionW, (float) headerArea.getY() + 4.0f,
                                (float) headerArea.getBottom() - 4.0f);
        }
    }

    // ─── Detail area background ────────────────────────────────────────
    if (expandedEffect >= 0)
    {
        auto detailArea = getLocalBounds().withTop (headerHeight);
        g.setColour (Colors::bgLight);
        g.fillRect (detailArea);

        // Top border line
        g.setColour (Colors::fxAccent.withAlpha (0.4f));
        g.drawHorizontalLine (headerHeight, 0.0f, (float) getWidth());
    }
}

void EffectsBarComponent::layoutDetailKnobs (juce::Rectangle<int> area, int fxIndex)
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

    switch (fxIndex)
    {
        case FX_CRUSH:
        {
            int colW = area.getWidth() / 3;
            layoutKnobColumn (area.removeFromLeft (colW), bcBitDepth, bcBitDepthLabel);
            layoutKnobColumn (area.removeFromLeft (colW), bcRate, bcRateLabel);
            layoutKnobColumn (area, bcMix, bcMixLabel);
            break;
        }
        case FX_FILTER:
        {
            int colW = area.getWidth() / 3;
            // Combo gets a column — use most of the column width
            auto comboCol = area.removeFromLeft (colW);
            fltTypeLabel.setBounds (comboCol.removeFromTop (labelH));
            int cw = juce::jmax (60, comboCol.getWidth() - 12);
            fltType.setBounds (comboCol.withSizeKeepingCentre (cw, comboH));

            layoutKnobColumn (area.removeFromLeft (colW), fltCutoff, fltCutoffLabel);
            layoutKnobColumn (area, fltResonance, fltResonanceLabel);
            break;
        }
        case FX_CHORUS:
        {
            int colW = area.getWidth() / 3;
            layoutKnobColumn (area.removeFromLeft (colW), chRate, chRateLabel);
            layoutKnobColumn (area.removeFromLeft (colW), chDepth, chDepthLabel);
            layoutKnobColumn (area, chMix, chMixLabel);
            break;
        }
        case FX_DELAY:
        {
            int colW = area.getWidth() / 3;
            layoutKnobColumn (area.removeFromLeft (colW), dlTime, dlTimeLabel);
            layoutKnobColumn (area.removeFromLeft (colW), dlFeedback, dlFeedbackLabel);
            layoutKnobColumn (area, dlMix, dlMixLabel);
            break;
        }
        case FX_REVERB:
        {
            int colW = area.getWidth() / 4;
            layoutKnobColumn (area.removeFromLeft (colW), rvSize, rvSizeLabel);
            layoutKnobColumn (area.removeFromLeft (colW), rvDamping, rvDampingLabel);
            layoutKnobColumn (area.removeFromLeft (colW), rvWidth, rvWidthLabel);
            layoutKnobColumn (area, rvMix, rvMixLabel);
            break;
        }
        default: break;
    }
}

void EffectsBarComponent::resized()
{
    // Enable buttons are invisible but need valid bounds for APVTS attachment
    bcEnable.setBounds (0, 0, 0, 0);
    fltEnable.setBounds (0, 0, 0, 0);
    chEnable.setBounds (0, 0, 0, 0);
    dlEnable.setBounds (0, 0, 0, 0);
    rvEnable.setBounds (0, 0, 0, 0);

    if (expandedEffect >= 0)
    {
        auto detailArea = getLocalBounds().withTop (headerHeight);
        layoutDetailKnobs (detailArea, expandedEffect);
    }
}

} // namespace cart
