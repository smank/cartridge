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
    bcBitDepth.setTooltip ("Bit depth reduction (lower = crunchier)");
    addChildComponent (bcBitDepth);
    bcBitDepthAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::BcBitDepth, bcBitDepth);
    makeKnobLabel (bcBitDepthLabel, "Bit Depth");
    addChildComponent (bcBitDepthLabel);

    styleKnob (bcRate);
    bcRate.setTooltip ("Sample rate reduction factor");
    addChildComponent (bcRate);
    bcRateAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::BcRateReduce, bcRate);
    makeKnobLabel (bcRateLabel, "Rate");
    addChildComponent (bcRateLabel);

    styleKnob (bcMix);
    bcMix.setTooltip ("BitCrush dry/wet mix");
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
    fltType.setTooltip ("Filter type: Low-pass, Band-pass, High-pass");
    fltType.setColour (juce::ComboBox::backgroundColourId, Colors::bgLight);
    fltType.setColour (juce::ComboBox::textColourId, Colors::textPrimary);
    fltType.setColour (juce::ComboBox::outlineColourId, Colors::knobOutline);
    addChildComponent (fltType);
    fltTypeAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        apvts, ParamIDs::FltType, fltType);
    makeKnobLabel (fltTypeLabel, "Type");
    addChildComponent (fltTypeLabel);

    styleKnob (fltCutoff);
    fltCutoff.setTooltip ("Filter cutoff frequency");
    addChildComponent (fltCutoff);
    fltCutoffAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::FltCutoff, fltCutoff);
    makeKnobLabel (fltCutoffLabel, "Cutoff");
    addChildComponent (fltCutoffLabel);

    styleKnob (fltResonance);
    fltResonance.setTooltip ("Filter resonance (peak at cutoff)");
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
    chRate.setTooltip ("Chorus modulation rate");
    addChildComponent (chRate);
    chRateAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::ChRate, chRate);
    makeKnobLabel (chRateLabel, "Rate");
    addChildComponent (chRateLabel);

    styleKnob (chDepth);
    chDepth.setTooltip ("Chorus modulation depth");
    addChildComponent (chDepth);
    chDepthAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::ChDepth, chDepth);
    makeKnobLabel (chDepthLabel, "Depth");
    addChildComponent (chDepthLabel);

    styleKnob (chMix);
    chMix.setTooltip ("Chorus dry/wet mix");
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
    dlTime.setTooltip ("Delay time in milliseconds");
    addChildComponent (dlTime);
    dlTimeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::DlTime, dlTime);
    makeKnobLabel (dlTimeLabel, "Time");
    addChildComponent (dlTimeLabel);

    styleKnob (dlFeedback);
    dlFeedback.setTooltip ("Delay feedback (echo repeats)");
    addChildComponent (dlFeedback);
    dlFeedbackAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::DlFeedback, dlFeedback);
    makeKnobLabel (dlFeedbackLabel, "Feedback");
    addChildComponent (dlFeedbackLabel);

    styleKnob (dlMix);
    dlMix.setTooltip ("Delay dry/wet mix");
    addChildComponent (dlMix);
    dlMixAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::DlMix, dlMix);
    makeKnobLabel (dlMixLabel, "Mix");
    addChildComponent (dlMixLabel);

    // Delay tempo sync
    dlSyncToggle.setButtonText ("Sync");
    dlSyncToggle.setColour (juce::ToggleButton::textColourId, Colors::textSecondary);
    dlSyncToggle.setColour (juce::ToggleButton::tickColourId, Colors::fxAccent);
    dlSyncToggle.setTooltip ("Sync delay time to host tempo");
    addChildComponent (dlSyncToggle);
    dlSyncAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::DlSyncEnabled, dlSyncToggle);

    dlSyncDiv.addItemList ({ "1/1", "1/2", "1/4", "1/8", "1/16", "1/32", "1/4T", "1/8T", "1/16T" }, 1);
    dlSyncDiv.setColour (juce::ComboBox::backgroundColourId, Colors::bgLight);
    dlSyncDiv.setColour (juce::ComboBox::textColourId, Colors::textPrimary);
    dlSyncDiv.setColour (juce::ComboBox::outlineColourId, Colors::knobOutline);
    dlSyncDiv.setTooltip ("Delay tempo sync note division");
    addChildComponent (dlSyncDiv);
    dlSyncDivAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        apvts, ParamIDs::DlSyncDiv, dlSyncDiv);

    // ─── Reverb ────────────────────────────────────────────────────────
    rvEnable.setClickingTogglesState (true);
    addAndMakeVisible (rvEnable);
    rvEnableAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::RvEnabled, rvEnable);
    rvEnable.setSize (0, 0);

    styleKnob (rvSize);
    rvSize.setTooltip ("Reverb room size");
    addChildComponent (rvSize);
    rvSizeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::RvSize, rvSize);
    makeKnobLabel (rvSizeLabel, "Size");
    addChildComponent (rvSizeLabel);

    styleKnob (rvDamping);
    rvDamping.setTooltip ("Reverb high-frequency damping");
    addChildComponent (rvDamping);
    rvDampingAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::RvDamping, rvDamping);
    makeKnobLabel (rvDampingLabel, "Damp");
    addChildComponent (rvDampingLabel);

    styleKnob (rvWidth);
    rvWidth.setTooltip ("Reverb stereo width");
    addChildComponent (rvWidth);
    rvWidthAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::RvWidth, rvWidth);
    makeKnobLabel (rvWidthLabel, "Width");
    addChildComponent (rvWidthLabel);

    styleKnob (rvMix);
    rvMix.setTooltip ("Reverb dry/wet mix");
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
    knob.setRotaryParameters (juce::MathConstants<float>::pi * 1.2f,
                              juce::MathConstants<float>::pi * 2.8f,
                              true);
    knob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 64, 14);
    knob.setPopupMenuEnabled (true);
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
            setVis ({ &dlTime, &dlFeedback, &dlMix, &dlTimeLabel, &dlFeedbackLabel, &dlMixLabel, &dlSyncToggle, &dlSyncDiv });
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

    // Top edge accent
    g.setColour (Colors::divider.withAlpha (0.3f));
    g.fillRect (headerArea.getX(), headerArea.getY(), headerArea.getWidth(), 1);

    const juce::String fxNames[] = { "CRUSH", "FILTER", "CHORUS", "DELAY", "REVERB" };
    const juce::ToggleButton* enableButtons[] = { &bcEnable, &fltEnable, &chEnable, &dlEnable, &rvEnable };

    for (int i = 0; i < NUM_FX; ++i)
    {
        auto section = headerArea.withX (i * sectionW).withWidth (sectionW);
        bool enabled = enableButtons[i]->getToggleState();
        bool expanded = (expandedEffect == i);

        if (expanded)
        {
            g.setColour (Colors::bgLight);
            g.fillRect (section);
        }

        // LED — smaller, cleaner
        auto ledBounds = section.withWidth (26).withSizeKeepingCentre (8, 8).toFloat();
        if (enabled)
        {
            g.setColour (Colors::fxAccent);
            g.fillEllipse (ledBounds);
            // Glow
            g.setColour (Colors::fxAccent.withAlpha (0.2f));
            g.fillEllipse (ledBounds.expanded (3.0f));
        }
        else
        {
            g.setColour (Colors::knobOutline.withAlpha (0.6f));
            g.drawEllipse (ledBounds, 1.0f);
        }

        auto textArea = section.withTrimmedLeft (26);
        g.setColour (enabled ? Colors::textPrimary : Colors::textSecondary);
        g.setFont (juce::FontOptions (11.0f));

        juce::String label = fxNames[i];
        if (expanded) label += " " + juce::String::charToString (0x25BE);
        g.drawText (label, textArea, juce::Justification::centredLeft);

        if (i < NUM_FX - 1)
        {
            g.setColour (Colors::divider.withAlpha (0.25f));
            g.drawVerticalLine ((i + 1) * sectionW, (float) headerArea.getY() + 6.0f,
                                (float) headerArea.getBottom() - 6.0f);
        }
    }

    // ─── Detail area ────────────────────────────────────────────────────
    if (expandedEffect >= 0)
    {
        auto detailArea = getLocalBounds().withTop (headerHeight);
        g.setColour (Colors::bgLight);
        g.fillRect (detailArea);

        g.setColour (Colors::fxAccent.withAlpha (0.3f));
        g.fillRect (0, headerHeight, getWidth(), 1);
    }
}

void EffectsBarComponent::layoutDetailKnobs (juce::Rectangle<int> area, int fxIndex)
{
    constexpr int colW   = 84;
    constexpr int colGap = 8;
    constexpr int labelH = 14;
    constexpr int comboH = 22;

    area = area.reduced (10, 8);

    // Centre the column row horizontally
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

    switch (fxIndex)
    {
        case FX_CRUSH:
            centreColumns (3);
            layoutKnobCol (bcBitDepth,   bcBitDepthLabel);
            layoutKnobCol (bcRate,       bcRateLabel);
            layoutKnobCol (bcMix,        bcMixLabel);
            break;
        case FX_FILTER:
            centreColumns (3);
            layoutComboCol (fltType,    fltTypeLabel);
            layoutKnobCol  (fltCutoff,  fltCutoffLabel);
            layoutKnobCol  (fltResonance, fltResonanceLabel);
            break;
        case FX_CHORUS:
            centreColumns (3);
            layoutKnobCol (chRate,  chRateLabel);
            layoutKnobCol (chDepth, chDepthLabel);
            layoutKnobCol (chMix,   chMixLabel);
            break;
        case FX_DELAY:
        {
            // 3 knobs + sync column (toggle stacked over combo)
            centreColumns (4);
            layoutKnobCol (dlTime,     dlTimeLabel);
            layoutKnobCol (dlFeedback, dlFeedbackLabel);
            layoutKnobCol (dlMix,      dlMixLabel);

            auto syncCol = area.removeFromLeft (colW);
            syncCol.removeFromTop (labelH + 4);
            dlSyncToggle.setBounds (syncCol.removeFromTop (24));
            syncCol.removeFromTop (4);
            dlSyncDiv.setBounds (syncCol.removeFromTop (comboH));
            break;
        }
        case FX_REVERB:
            centreColumns (4);
            layoutKnobCol (rvSize,    rvSizeLabel);
            layoutKnobCol (rvDamping, rvDampingLabel);
            layoutKnobCol (rvWidth,   rvWidthLabel);
            layoutKnobCol (rvMix,     rvMixLabel);
            break;
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
