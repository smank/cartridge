#include "ChannelStripComponent.h"
#include "Parameters.h"

namespace cart {

namespace
{
    // Horizontal sliders for narrow rows (transpose, sweep, linear counter)
    void styleKnob (juce::Slider& knob)
    {
        knob.setSliderStyle (juce::Slider::LinearHorizontal);
        knob.setTextBoxStyle (juce::Slider::TextBoxRight, false, 36, 14);
        knob.setPopupMenuEnabled (true);
    }

    // Rotary used for the prominent strip controls (Volume, Pan, Main knob)
    void styleRotaryKnob (juce::Slider& knob)
    {
        knob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        knob.setRotaryParameters (juce::MathConstants<float>::pi * 1.2f,
                                  juce::MathConstants<float>::pi * 2.8f,
                                  true);
        knob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 40, 12);
        knob.setPopupMenuEnabled (true);
    }

    void styleDetailKnob (juce::Slider& knob) { styleKnob (knob); }
    void styleFader      (juce::Slider& fader) { styleKnob (fader); }

    void styleToggle (juce::ToggleButton& toggle, const juce::String& text)
    {
        toggle.setButtonText (text);
    }

    void styleLabel (juce::Label& label, const juce::String& text, float fontSize,
                     juce::Colour colour = Colors::textPrimary)
    {
        label.setText (text, juce::dontSendNotification);
        label.setFont (juce::FontOptions (fontSize));
        label.setColour (juce::Label::textColourId, colour);
        label.setJustificationType (juce::Justification::centred);
    }

    juce::String getChannelName (ChannelType type)
    {
        switch (type)
        {
            case ChannelType::Pulse1:     return "PLS 1";
            case ChannelType::Pulse2:     return "PLS 2";
            case ChannelType::Triangle:   return "TRI";
            case ChannelType::Noise:      return "NOISE";
            case ChannelType::Dpcm:       return "DPCM";
            case ChannelType::Vrc6Pulse1: return "V.PLS1";
            case ChannelType::Vrc6Pulse2: return "V.PLS2";
            case ChannelType::Vrc6Saw:    return "V.SAW";
        }
        return {};
    }
} // anonymous

ChannelStripComponent::ChannelStripComponent (ChannelType type,
                                              juce::AudioProcessorValueTreeState& apvts)
    : channelType (type)
{
    auto nameColour = isVrc6() ? Colors::orangeAccent : Colors::accentActive;
    nameLabel.setText (getChannelName (type), juce::dontSendNotification);
    nameLabel.setFont (cart::ui::displayFont (13.0f));
    nameLabel.setColour (juce::Label::textColourId, nameColour);
    nameLabel.setJustificationType (juce::Justification::centred);
    nameLabel.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (nameLabel);

    // Pan knob — every channel has one (rotary, sits next to Volume)
    styleRotaryKnob (panKnob);
    panKnob.setTooltip ("Stereo pan position (L/R)");
    addAndMakeVisible (panKnob);

    // Mix fader — every channel has one
    // Mix uses LinearBar style — fills with primary as a coloured bar so
    // it reads as "level" at a glance and is visually distinct from the
    // thumb-on-track sliders used for Volume/Pan/etc.
    styleFader (mixFader);
    mixFader.setSliderStyle (juce::Slider::LinearBar);
    mixFader.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    mixFader.setTooltip ("Channel mix level (0-100%)");
    addAndMakeVisible (mixFader);

    // Details state — toggled by clicking the strip header chevron.
    // detailsButton stays invisible (size 0) but holds the toggle state so
    // any external code that polls it still works.
    detailsButton.setClickingTogglesState (true);
    detailsButton.onClick = [this]
    {
        detailsVisible = detailsButton.getToggleState();
        resized();
        repaint();
    };

    // Wire up pan knob attachment based on channel type
    {
        const char* panID = nullptr;
        switch (type)
        {
            case ChannelType::Pulse1:     panID = ParamIDs::P1Pan; break;
            case ChannelType::Pulse2:     panID = ParamIDs::P2Pan; break;
            case ChannelType::Triangle:   panID = ParamIDs::TriPan; break;
            case ChannelType::Noise:      panID = ParamIDs::NoisePan; break;
            case ChannelType::Dpcm:       panID = ParamIDs::DpcmPan; break;
            case ChannelType::Vrc6Pulse1: panID = ParamIDs::Vrc6P1Pan; break;
            case ChannelType::Vrc6Pulse2: panID = ParamIDs::Vrc6P2Pan; break;
            case ChannelType::Vrc6Saw:    panID = ParamIDs::Vrc6SawPan; break;
        }
        if (panID != nullptr)
            panAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
                apvts, panID, panKnob);
    }

    switch (type)
    {
        case ChannelType::Pulse1:     setupPulseControls ("p1", apvts); break;
        case ChannelType::Pulse2:     setupPulseControls ("p2", apvts); break;
        case ChannelType::Triangle:   setupTriangleControls (apvts); break;
        case ChannelType::Noise:      setupNoiseControls (apvts); break;
        case ChannelType::Dpcm:       setupDpcmControls (apvts); break;
        case ChannelType::Vrc6Pulse1: setupVrc6PulseControls ("vrc6P1", apvts); break;
        case ChannelType::Vrc6Pulse2: setupVrc6PulseControls ("vrc6P2", apvts); break;
        case ChannelType::Vrc6Saw:    setupVrc6SawControls (apvts); break;
    }

    startTimerHz (15);
}

ChannelStripComponent::~ChannelStripComponent()
{
    stopTimer();
}

void ChannelStripComponent::timerCallback()
{
    bool newState = (activityFlag != nullptr) && activityFlag->load (std::memory_order_relaxed);
    if (newState != ledState)
    {
        ledState = newState;
        repaint();  // Outline + accent stripe + LED all change with active state
    }
    else if (ledState && enableAttach != nullptr && enableToggle.getToggleState())
    {
        // While the LED is active, repaint the header so the breathing halo animates
        repaint (0, 0, getWidth(), headerHeight + 4);
    }
}

void ChannelStripComponent::mouseDown (const juce::MouseEvent& e)
{
    if (e.getPosition().getY() >= headerHeight)
        return;

    if (e.getPosition().getX() < ledClickWidth)
    {
        // Toggle channel enable
        if (enableAttach != nullptr)
        {
            enableToggle.setToggleState (! enableToggle.getToggleState(),
                                          juce::sendNotification);
            repaint();
        }
        return;
    }

    // Anywhere else in the header → toggle the details accordion
    if (hasDetails)
    {
        detailsButton.setToggleState (! detailsButton.getToggleState(),
                                       juce::sendNotification);
        // detailsButton.onClick triggers resized() + repaint()
    }
}

void ChannelStripComponent::mouseMove (const juce::MouseEvent& e)
{
    const bool wasOver = headerHovered;
    headerHovered = (e.getPosition().getY() < headerHeight);
    if (wasOver != headerHovered)
        repaint (0, 0, getWidth(), headerHeight + 4);
}

void ChannelStripComponent::mouseExit (const juce::MouseEvent&)
{
    if (headerHovered)
    {
        headerHovered = false;
        repaint (0, 0, getWidth(), headerHeight + 4);
    }
}

bool ChannelStripComponent::isVrc6() const
{
    return channelType == ChannelType::Vrc6Pulse1
        || channelType == ChannelType::Vrc6Pulse2
        || channelType == ChannelType::Vrc6Saw;
}

void ChannelStripComponent::setupPulseControls (const juce::String& prefix,
                                                juce::AudioProcessorValueTreeState& apvts)
{
    // Enable
    styleToggle (enableToggle, "On");
    enableToggle.setTooltip ("Enable/disable this channel");
    addAndMakeVisible (enableToggle);
    enableAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, prefix + "Enabled", enableToggle);

    // Duty combo
    hasMainCombo = true;
    mainCombo.addItem ("12.5%", 1);
    mainCombo.addItem ("25%", 2);
    mainCombo.addItem ("50%", 3);
    mainCombo.addItem ("75%", 4);
    mainCombo.setColour (juce::ComboBox::backgroundColourId, Colors::bgLight);
    mainCombo.setColour (juce::ComboBox::textColourId, Colors::textPrimary);
    mainCombo.setColour (juce::ComboBox::outlineColourId, Colors::knobOutline);
    mainCombo.setTooltip ("Pulse wave duty cycle (12.5%, 25%, 50%, 75%)");
    addAndMakeVisible (mainCombo);
    mainComboAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        apvts, prefix + "Duty", mainCombo);

    // Transpose knob
    hasTranspose = true;
    styleKnob (transposeKnob);
    transposeKnob.setTextValueSuffix (" st");
    transposeKnob.setTooltip ("Transpose this channel in semitones (-24 to +24)");
    addAndMakeVisible (transposeKnob);
    transposeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, prefix + "Transpose", transposeKnob);

    // Volume knob
    hasVolume = true;
    styleRotaryKnob (volumeKnob);
    volumeKnob.setTooltip ("Channel volume (0-15)");
    addAndMakeVisible (volumeKnob);
    volumeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, prefix + "Volume", volumeKnob);

    // Details (sweep/envelope)
    hasDetails = true;
    detailsButton.setTooltip ("Show/hide envelope and sweep controls");
    addAndMakeVisible (detailsButton);

    styleToggle (constVolToggle, "ConstV");
    constVolToggle.setTooltip ("Constant volume (bypass envelope decay)");
    addChildComponent (constVolToggle);
    constVolAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, prefix + "ConstVol", constVolToggle);

    styleToggle (envLoopToggle, "EnvLp");
    envLoopToggle.setTooltip ("Loop envelope (restart decay when it reaches 0)");
    addChildComponent (envLoopToggle);
    envLoopAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, prefix + "EnvLoop", envLoopToggle);

    styleToggle (sweepEnableToggle, "Sweep");
    sweepEnableToggle.setTooltip ("Enable frequency sweep (pitch slides up/down)");
    addChildComponent (sweepEnableToggle);
    sweepEnableAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, prefix + "SweepEnable", sweepEnableToggle);

    styleDetailKnob (sweepPeriodKnob);
    sweepPeriodKnob.setTooltip ("Sweep period (speed of pitch change)");
    addChildComponent (sweepPeriodKnob);
    sweepPeriodAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, prefix + "SweepPeriod", sweepPeriodKnob);

    styleToggle (sweepNegateToggle, "Neg");
    sweepNegateToggle.setTooltip ("Sweep negate (pitch slides down instead of up)");
    addChildComponent (sweepNegateToggle);
    sweepNegateAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, prefix + "SweepNegate", sweepNegateToggle);

    styleDetailKnob (sweepShiftKnob);
    sweepShiftKnob.setTooltip ("Sweep shift (amount of pitch change per step)");
    addChildComponent (sweepShiftKnob);
    sweepShiftAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, prefix + "SweepShift", sweepShiftKnob);

    // Mix
    mixAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, prefix + "Mix", mixFader);
}

void ChannelStripComponent::setupTriangleControls (juce::AudioProcessorValueTreeState& apvts)
{
    // Enable
    styleToggle (enableToggle, "On");
    enableToggle.setTooltip ("Enable/disable triangle channel");
    addAndMakeVisible (enableToggle);
    enableAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::TriEnabled, enableToggle);

    // No main combo or knob — just a label
    hasMainCombo = false;
    hasMainKnob = false;
    styleLabel (mainLabel, "32-Step", 11.0f, Colors::textSecondary);
    addAndMakeVisible (mainLabel);

    // Transpose knob
    hasTranspose = true;
    styleKnob (transposeKnob);
    transposeKnob.setTextValueSuffix (" st");
    transposeKnob.setTooltip ("Transpose this channel in semitones (-24 to +24)");
    addAndMakeVisible (transposeKnob);
    transposeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::TriTranspose, transposeKnob);

    // No volume knob — fixed volume
    hasVolume = false;
    styleLabel (volumeLabel, "Fixed", 11.0f, Colors::textSecondary);
    addAndMakeVisible (volumeLabel);

    // Details (linear counter)
    hasDetails = true;
    detailsButton.setTooltip ("Show/hide linear counter controls");
    addAndMakeVisible (detailsButton);

    styleDetailKnob (linearReloadKnob);
    linearReloadKnob.setTooltip ("Linear counter reload value (note duration)");
    addChildComponent (linearReloadKnob);
    linearReloadAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::TriLinearReload, linearReloadKnob);

    styleToggle (linearControlToggle, "LinCtrl");
    linearControlToggle.setTooltip ("Linear counter control (halt length counter)");
    addChildComponent (linearControlToggle);
    linearControlAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::TriLinearControl, linearControlToggle);

    // Mix
    mixAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::TriMix, mixFader);
}

void ChannelStripComponent::setupNoiseControls (juce::AudioProcessorValueTreeState& apvts)
{
    // Enable
    styleToggle (enableToggle, "On");
    enableToggle.setTooltip ("Enable/disable noise channel");
    addAndMakeVisible (enableToggle);
    enableAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::NoiseEnabled, enableToggle);

    // Mode combo
    hasMainCombo = true;
    mainCombo.addItem ("Long", 1);
    mainCombo.addItem ("Short", 2);
    mainCombo.setColour (juce::ComboBox::backgroundColourId, Colors::bgLight);
    mainCombo.setColour (juce::ComboBox::textColourId, Colors::textPrimary);
    mainCombo.setColour (juce::ComboBox::outlineColourId, Colors::knobOutline);
    mainCombo.setTooltip ("Short = metallic/pitched, Long = white noise");
    addAndMakeVisible (mainCombo);
    mainComboAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        apvts, ParamIDs::NoiseMode, mainCombo);

    // Period knob
    hasMainKnob = true;
    styleKnob (noisePeriodKnob);
    noisePeriodKnob.setTooltip ("Noise period index (pitch of noise)");
    addAndMakeVisible (noisePeriodKnob);
    noisePeriodAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::NoisePeriod, noisePeriodKnob);

    // Volume knob
    hasVolume = true;
    styleRotaryKnob (volumeKnob);
    volumeKnob.setTooltip ("Noise volume (0-15)");
    addAndMakeVisible (volumeKnob);
    volumeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::NoiseVolume, volumeKnob);

    // Details (envelope)
    hasDetails = true;
    detailsButton.setTooltip ("Show/hide envelope controls");
    addAndMakeVisible (detailsButton);

    styleToggle (constVolToggle, "ConstV");
    constVolToggle.setTooltip ("Constant volume (bypass envelope decay)");
    addChildComponent (constVolToggle);
    constVolAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::NoiseConstVol, constVolToggle);

    styleToggle (envLoopToggle, "EnvLp");
    envLoopToggle.setTooltip ("Loop envelope (restart decay when it reaches 0)");
    addChildComponent (envLoopToggle);
    envLoopAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::NoiseEnvLoop, envLoopToggle);

    // Mix
    mixAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::NoiseMix, mixFader);
}

void ChannelStripComponent::setupDpcmControls (juce::AudioProcessorValueTreeState& apvts)
{
    // Enable
    styleToggle (enableToggle, "On");
    enableToggle.setTooltip ("Enable/disable DPCM sample channel");
    addAndMakeVisible (enableToggle);
    enableAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::DpcmEnabled, enableToggle);

    // Rate knob
    hasMainKnob = true;
    hasMainCombo = false;
    styleRotaryKnob (mainKnob);
    mainKnob.setTooltip ("DPCM playback rate index");
    addAndMakeVisible (mainKnob);
    mainKnobAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::DpcmRate, mainKnob);

    // No volume — fixed
    hasVolume = false;
    styleLabel (volumeLabel, "Fixed", 11.0f, Colors::textSecondary);
    addAndMakeVisible (volumeLabel);

    // Details (loop toggle)
    hasDetails = true;
    detailsButton.setTooltip ("Show/hide DPCM loop control");
    addAndMakeVisible (detailsButton);

    styleToggle (dpcmLoopToggle, "Loop");
    dpcmLoopToggle.setTooltip ("Loop DPCM sample playback");
    addChildComponent (dpcmLoopToggle);
    dpcmLoopAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::DpcmLoop, dpcmLoopToggle);

    // Mix
    mixAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::DpcmMix, mixFader);
}

void ChannelStripComponent::setupVrc6PulseControls (const juce::String& prefix,
                                                    juce::AudioProcessorValueTreeState& apvts)
{
    // Individual enable toggle for this VRC6 channel
    styleToggle (enableToggle, "On");
    enableToggle.setTooltip ("Enable/disable this VRC6 channel");
    addAndMakeVisible (enableToggle);
    enableAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, prefix + "Enabled", enableToggle);

    // Duty knob (0–7)
    hasMainKnob = true;
    hasMainCombo = false;
    styleRotaryKnob (mainKnob);
    mainKnob.setTooltip ("VRC6 pulse duty cycle (0-7, 8 levels)");
    addAndMakeVisible (mainKnob);
    mainKnobAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, prefix + "Duty", mainKnob);

    // Transpose knob
    hasTranspose = true;
    styleKnob (transposeKnob);
    transposeKnob.setTextValueSuffix (" st");
    transposeKnob.setTooltip ("Transpose this channel in semitones (-24 to +24)");
    addAndMakeVisible (transposeKnob);
    transposeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, prefix + "Transpose", transposeKnob);

    // Volume knob
    hasVolume = true;
    styleRotaryKnob (volumeKnob);
    volumeKnob.setTooltip ("VRC6 pulse volume (0-15)");
    addAndMakeVisible (volumeKnob);
    volumeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, prefix + "Volume", volumeKnob);

    // No details
    hasDetails = false;

    // Mix
    mixAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, prefix + "Mix", mixFader);
}

void ChannelStripComponent::setupVrc6SawControls (juce::AudioProcessorValueTreeState& apvts)
{
    // Individual enable toggle for VRC6 sawtooth
    styleToggle (enableToggle, "On");
    enableToggle.setTooltip ("Enable/disable VRC6 sawtooth channel");
    addAndMakeVisible (enableToggle);
    enableAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::Vrc6SawEnabled, enableToggle);

    // Rate knob
    hasMainKnob = true;
    hasMainCombo = false;
    styleRotaryKnob (mainKnob);
    mainKnob.setTooltip ("VRC6 sawtooth accumulator rate (0-63)");
    addAndMakeVisible (mainKnob);
    mainKnobAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::Vrc6SawRate, mainKnob);

    // Transpose knob
    hasTranspose = true;
    styleKnob (transposeKnob);
    transposeKnob.setTextValueSuffix (" st");
    transposeKnob.setTooltip ("Transpose this channel in semitones (-24 to +24)");
    addAndMakeVisible (transposeKnob);
    transposeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::Vrc6SawTranspose, transposeKnob);

    // No volume — fixed
    hasVolume = false;
    styleLabel (volumeLabel, "Fixed", 11.0f, Colors::textSecondary);
    addAndMakeVisible (volumeLabel);

    // No details
    hasDetails = false;

    // Mix
    mixAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::Vrc6SawMix, mixFader);
}

void ChannelStripComponent::paint (juce::Graphics& g)
{
    using namespace cart::ui;
    auto bounds = getLocalBounds().toFloat().reduced (1.0f);
    const float cornerR = 5.0f;
    const auto accentColour = isVrc6() ? Palette::vrc6Accent : Palette::primary;
    const bool enabled = enableAttach != nullptr ? enableToggle.getToggleState() : true;

    // ─── Strip body (vertical gradient) ────────────────────────────────
    juce::ColourGradient bodyGrad (
        Palette::surfaceAlt.brighter (0.05f), 0.0f, bounds.getY(),
        Palette::surfaceAlt.darker (0.10f),   0.0f, bounds.getBottom(),
        false);
    g.setGradientFill (bodyGrad);
    g.fillRoundedRectangle (bounds, cornerR);

    // Outline — brighter when activity LED is lit
    g.setColour (ledState
                    ? accentColour.withAlpha (0.60f)
                    : Palette::outlineDim.withAlpha (0.5f));
    g.drawRoundedRectangle (bounds.reduced (0.25f), cornerR, 0.8f);

    // ─── Header (top headerHeight px) ─────────────────────────────────
    g.saveState();
    g.reduceClipRegion (bounds.toNearestIntEdges());

    auto header = bounds.withHeight ((float) headerHeight);

    // Header bg: brighten when hovered or details are open
    if (headerHovered || detailsVisible)
    {
        const float a = detailsVisible ? 0.65f : (headerHovered ? 0.40f : 0.0f);
        g.setColour (Palette::surfaceHi.withAlpha (a));
        g.fillRect (header);
    }

    // Accent stripe BELOW the header — separates header from body
    {
        const float baseAlpha = (enabled && ledState) ? 1.0f : (enabled ? 0.85f : 0.45f);
        const float stripeY = header.getBottom();
        g.setColour (accentColour.withAlpha (baseAlpha));
        g.fillRect (bounds.getX(), stripeY - 1.0f, bounds.getWidth(), 2.0f);

        // Soft glow trailing down into the body
        juce::ColourGradient glow (
            accentColour.withAlpha (ledState ? 0.22f : 0.08f),
            bounds.getCentreX(), stripeY,
            accentColour.withAlpha (0.0f),
            bounds.getCentreX(), stripeY + 14.0f,
            false);
        g.setGradientFill (glow);
        g.fillRect (bounds.getX(), stripeY, bounds.getWidth(), 14.0f);
    }

    // ─── Header LED — toggles enable on click, breathes when active ───
    {
        const float ledSize = 12.0f;
        const float ledX = header.getX() + (ledClickWidth - ledSize) * 0.5f + 4.0f;
        const float ledY = header.getCentreY() - ledSize * 0.5f;
        const auto ledBounds = juce::Rectangle<float> (ledX, ledY, ledSize, ledSize);

        if (enabled)
        {
            // Breathing halo when channel is currently sounding
            if (ledState)
            {
                const float t = (float) (juce::Time::getMillisecondCounter() % 3600) / 3600.0f;
                const float breathe = 0.5f + 0.5f * std::sin (t * juce::MathConstants<float>::twoPi);
                const float haloA  = 0.18f + 0.18f * breathe;
                const float haloA2 = 0.30f + 0.14f * breathe;
                g.setColour (accentColour.withAlpha (haloA));
                g.fillEllipse (ledBounds.expanded (5.5f));
                g.setColour (accentColour.withAlpha (haloA2));
                g.fillEllipse (ledBounds.expanded (2.5f));
            }
            g.setColour (accentColour);
            g.fillEllipse (ledBounds);
            // Specular highlight
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

    // ─── Chevron at right of header — only if this channel has details ─
    if (hasDetails)
    {
        const float chevR = 4.0f;
        const float chevX = header.getRight() - 12.0f;
        const float chevY = header.getCentreY();
        juce::Path chev;
        if (detailsVisible)
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
        g.setColour (detailsVisible ? accentColour
                                    : (headerHovered ? Palette::secondary : Palette::textSecondary));
        g.fillPath (chev);
    }

    g.restoreState();

    // Draw labels above controls
    g.setFont (juce::FontOptions (10.0f));
    g.setColour (Colors::textSecondary);

    auto drawLabel = [&] (const juce::String& text, const juce::Component& comp)
    {
        if (comp.isVisible())
        {
            auto b = comp.getBounds();
            g.drawText (text, b.getX() + 2, b.getY() - 13, b.getWidth(), 12,
                        juce::Justification::centredLeft);
        }
    };

    if (hasMainCombo)
    {
        switch (channelType)
        {
            case ChannelType::Pulse1:
            case ChannelType::Pulse2:     drawLabel ("DUTY", mainCombo); break;
            case ChannelType::Noise:      drawLabel ("MODE", mainCombo); break;
            default: break;
        }
    }

    if (hasMainKnob)
    {
        auto& knob = hasMainCombo ? noisePeriodKnob : mainKnob;
        switch (channelType)
        {
            case ChannelType::Noise:      drawLabel ("PERIOD", knob); break;
            case ChannelType::Dpcm:       drawLabel ("RATE", knob); break;
            case ChannelType::Vrc6Pulse1:
            case ChannelType::Vrc6Pulse2: drawLabel ("DUTY", knob); break;
            case ChannelType::Vrc6Saw:    drawLabel ("RATE", knob); break;
            default: break;
        }
    }

    if (hasTranspose)  drawLabel ("TRANS", transposeKnob);
    if (hasVolume)     drawLabel ("VOL", volumeKnob);
    drawLabel ("PAN", panKnob);
    drawLabel ("MIX", mixFader);

    // Detail section labels
    if (hasDetails && detailsVisible)
    {
        if (sweepPeriodKnob.isVisible())  drawLabel ("SW.PRD", sweepPeriodKnob);
        if (sweepShiftKnob.isVisible())   drawLabel ("SW.SHF", sweepShiftKnob);
        if (linearReloadKnob.isVisible()) drawLabel ("LIN.RLD", linearReloadKnob);
    }
}

void ChannelStripComponent::resized()
{
    auto area = getLocalBounds().reduced (3);
    const int pad = 3;
    const int sliderH = 22;
    const int labelH = 13;
    const int rotaryH = 54;   // total slider height for vol/pan/main rotaries

    // ─── Header (replaces the old name row + enable row + details row) ─
    auto header = area.removeFromTop (headerHeight);
    nameLabel.setBounds (header.reduced (ledClickWidth, 0).withTrimmedRight (16));

    // Enable toggle + details button stay in the component tree (APVTS
    // attachment + state) but are invisible — clicks come from mouseDown.
    enableToggle.setBounds (0, 0, 0, 0);
    detailsButton.setBounds (0, 0, 0, 0);

    // 2px stripe + small gap before body controls
    area.removeFromTop (6);

    // ─── Bottom-up: Mix LinearBar + Vol/Pan side-by-side rotaries ────
    // Reserved first so they always sit at the bottom of the strip.
    auto mixSliderArea = area.removeFromBottom (sliderH);
    area.removeFromBottom (labelH);  // "MIX" label space
    mixFader.setBounds (mixSliderArea.reduced (2, 0));
    area.removeFromBottom (pad);

    if (hasVolume)
    {
        auto vpRow = area.removeFromBottom (rotaryH);
        area.removeFromBottom (labelH);  // "VOL" / "PAN" labels
        const int half = vpRow.getWidth() / 2;
        volumeKnob.setBounds (vpRow.removeFromLeft (half).reduced (2, 0));
        vpRow.removeFromLeft (2);
        panKnob.setBounds (vpRow.reduced (2, 0));
    }
    else
    {
        // Triangle: no volume; "Fixed" label + Pan rotary centred under it
        auto panRow = area.removeFromBottom (rotaryH);
        area.removeFromBottom (labelH);
        const int knobW = juce::jmin (50, panRow.getWidth());
        panKnob.setBounds (panRow.withSizeKeepingCentre (knobW, rotaryH));
        volumeLabel.setBounds (area.removeFromBottom (16));
    }
    area.removeFromBottom (pad);

    // ─── Top-down: main combo / main knob / transpose ─────────────────
    if (hasMainCombo)
    {
        area.removeFromTop (labelH);
        mainCombo.setBounds (area.removeFromTop (24).reduced (2, 0));
        area.removeFromTop (pad);
    }

    if (hasMainKnob)
    {
        area.removeFromTop (labelH);
        if (hasMainCombo)
        {
            // Noise period — narrow horizontal under the Mode combo
            noisePeriodKnob.setBounds (area.removeFromTop (sliderH).reduced (2, 0));
        }
        else
        {
            // Solo main knob (DPCM, VRC6 Pulse/Saw) — rotary centred
            const int knobW = juce::jmin (60, area.getWidth() - 8);
            mainKnob.setBounds (area.removeFromTop (rotaryH).withSizeKeepingCentre (knobW, rotaryH));
        }
        area.removeFromTop (pad);
    }
    else if (! hasMainCombo)
    {
        mainLabel.setBounds (area.removeFromTop (16));
        area.removeFromTop (pad);
    }

    if (hasTranspose)
    {
        area.removeFromTop (labelH);
        transposeKnob.setBounds (area.removeFromTop (sliderH).reduced (2, 0));
        area.removeFromTop (pad);
    }

    // ─── Details panel — fills any remaining mid-area when expanded ───
    if (hasDetails)
    {
        if (detailsVisible)
        {
            int detailH = juce::jmax (0, area.getHeight());
            auto detailArea = area.removeFromTop (detailH);
            const int rowH = 20;

            switch (channelType)
            {
                case ChannelType::Pulse1:
                case ChannelType::Pulse2:
                {
                    constVolToggle.setVisible (true);
                    envLoopToggle.setVisible (true);
                    sweepEnableToggle.setVisible (true);
                    sweepPeriodKnob.setVisible (true);
                    sweepNegateToggle.setVisible (true);
                    sweepShiftKnob.setVisible (true);

                    constVolToggle.setBounds (detailArea.removeFromTop (rowH));
                    envLoopToggle.setBounds (detailArea.removeFromTop (rowH));
                    sweepEnableToggle.setBounds (detailArea.removeFromTop (rowH));
                    detailArea.removeFromTop (labelH);
                    sweepPeriodKnob.setBounds (detailArea.removeFromTop (sliderH));
                    sweepNegateToggle.setBounds (detailArea.removeFromTop (rowH));
                    detailArea.removeFromTop (labelH);
                    sweepShiftKnob.setBounds (detailArea.removeFromTop (sliderH));
                    break;
                }
                case ChannelType::Triangle:
                {
                    linearReloadKnob.setVisible (true);
                    linearControlToggle.setVisible (true);

                    detailArea.removeFromTop (labelH);
                    linearReloadKnob.setBounds (detailArea.removeFromTop (sliderH));
                    linearControlToggle.setBounds (detailArea.removeFromTop (rowH));
                    break;
                }
                case ChannelType::Noise:
                {
                    constVolToggle.setVisible (true);
                    envLoopToggle.setVisible (true);

                    constVolToggle.setBounds (detailArea.removeFromTop (rowH));
                    envLoopToggle.setBounds (detailArea.removeFromTop (rowH));
                    break;
                }
                case ChannelType::Dpcm:
                {
                    dpcmLoopToggle.setVisible (true);
                    dpcmLoopToggle.setBounds (detailArea.removeFromTop (rowH));
                    break;
                }
                default:
                    break;
            }
            area.removeFromTop (pad);
        }
        else
        {
            constVolToggle.setVisible (false);
            envLoopToggle.setVisible (false);
            sweepEnableToggle.setVisible (false);
            sweepPeriodKnob.setVisible (false);
            sweepNegateToggle.setVisible (false);
            sweepShiftKnob.setVisible (false);
            linearReloadKnob.setVisible (false);
            linearControlToggle.setVisible (false);
            dpcmLoopToggle.setVisible (false);
        }
    }
    // Pan, Vol, and Mix were laid out at the start of this function from
    // the bottom up; nothing else needs placement here.
}

} // namespace cart
