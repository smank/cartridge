#include "ChannelStripComponent.h"
#include "Parameters.h"

namespace cart {

namespace
{
    void styleKnob (juce::Slider& knob)
    {
        knob.setSliderStyle (juce::Slider::LinearHorizontal);
        knob.setTextBoxStyle (juce::Slider::TextBoxRight, false, 36, 14);
        knob.setColour (juce::Slider::trackColourId, Colors::faderTrack);
        knob.setColour (juce::Slider::thumbColourId, Colors::faderThumb);
        knob.setColour (juce::Slider::backgroundColourId, Colors::bgLight);
        knob.setColour (juce::Slider::textBoxTextColourId, Colors::textSecondary);
        knob.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    }

    void styleDetailKnob (juce::Slider& knob)
    {
        knob.setSliderStyle (juce::Slider::LinearHorizontal);
        knob.setTextBoxStyle (juce::Slider::TextBoxRight, false, 36, 14);
        knob.setColour (juce::Slider::trackColourId, Colors::faderTrack);
        knob.setColour (juce::Slider::thumbColourId, Colors::faderThumb);
        knob.setColour (juce::Slider::backgroundColourId, Colors::bgLight);
        knob.setColour (juce::Slider::textBoxTextColourId, Colors::textSecondary);
        knob.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    }

    void styleFader (juce::Slider& fader)
    {
        fader.setSliderStyle (juce::Slider::LinearHorizontal);
        fader.setTextBoxStyle (juce::Slider::TextBoxRight, false, 36, 14);
        fader.setColour (juce::Slider::trackColourId, Colors::faderTrack);
        fader.setColour (juce::Slider::thumbColourId, Colors::faderThumb);
        fader.setColour (juce::Slider::backgroundColourId, Colors::bgLight);
        fader.setColour (juce::Slider::textBoxTextColourId, Colors::textSecondary);
        fader.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    }

    void styleToggle (juce::ToggleButton& toggle, const juce::String& text)
    {
        toggle.setButtonText (text);
        toggle.setColour (juce::ToggleButton::textColourId, Colors::textSecondary);
        toggle.setColour (juce::ToggleButton::tickColourId, Colors::accentActive);
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
    styleLabel (nameLabel, getChannelName (type), 14.0f, nameColour);
    addAndMakeVisible (nameLabel);

    // Mix fader — every channel has one
    styleFader (mixFader);
    addAndMakeVisible (mixFader);

    // Details button styling
    detailsButton.setColour (juce::TextButton::buttonColourId, Colors::bgLight);
    detailsButton.setColour (juce::TextButton::textColourOffId, Colors::textSecondary);
    detailsButton.setColour (juce::TextButton::textColourOnId, Colors::accentActive);
    detailsButton.setClickingTogglesState (true);
    detailsButton.onClick = [this]
    {
        detailsVisible = detailsButton.getToggleState();
        resized();
    };

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
        // Only repaint the header area (top 22px)
        repaint (0, 0, getWidth(), 22);
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
    addAndMakeVisible (mainCombo);
    mainComboAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        apvts, prefix + "Duty", mainCombo);

    // Transpose knob
    hasTranspose = true;
    styleKnob (transposeKnob);
    transposeKnob.setTextValueSuffix (" st");
    addAndMakeVisible (transposeKnob);
    transposeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, prefix + "Transpose", transposeKnob);

    // Volume knob
    hasVolume = true;
    styleKnob (volumeKnob);
    addAndMakeVisible (volumeKnob);
    volumeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, prefix + "Volume", volumeKnob);

    // Details (sweep/envelope)
    hasDetails = true;
    addAndMakeVisible (detailsButton);

    styleToggle (constVolToggle, "ConstV");
    addChildComponent (constVolToggle);
    constVolAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, prefix + "ConstVol", constVolToggle);

    styleToggle (envLoopToggle, "EnvLp");
    addChildComponent (envLoopToggle);
    envLoopAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, prefix + "EnvLoop", envLoopToggle);

    styleToggle (sweepEnableToggle, "Sweep");
    addChildComponent (sweepEnableToggle);
    sweepEnableAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, prefix + "SweepEnable", sweepEnableToggle);

    styleDetailKnob (sweepPeriodKnob);
    addChildComponent (sweepPeriodKnob);
    sweepPeriodAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, prefix + "SweepPeriod", sweepPeriodKnob);

    styleToggle (sweepNegateToggle, "Neg");
    addChildComponent (sweepNegateToggle);
    sweepNegateAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, prefix + "SweepNegate", sweepNegateToggle);

    styleDetailKnob (sweepShiftKnob);
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
    addAndMakeVisible (transposeKnob);
    transposeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::TriTranspose, transposeKnob);

    // No volume knob — fixed volume
    hasVolume = false;
    styleLabel (volumeLabel, "Fixed", 11.0f, Colors::textSecondary);
    addAndMakeVisible (volumeLabel);

    // Details (linear counter)
    hasDetails = true;
    addAndMakeVisible (detailsButton);

    styleDetailKnob (linearReloadKnob);
    addChildComponent (linearReloadKnob);
    linearReloadAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::TriLinearReload, linearReloadKnob);

    styleToggle (linearControlToggle, "LinCtrl");
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
    addAndMakeVisible (mainCombo);
    mainComboAttach = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        apvts, ParamIDs::NoiseMode, mainCombo);

    // Period knob
    hasMainKnob = true;
    styleKnob (noisePeriodKnob);
    addAndMakeVisible (noisePeriodKnob);
    noisePeriodAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::NoisePeriod, noisePeriodKnob);

    // Volume knob
    hasVolume = true;
    styleKnob (volumeKnob);
    addAndMakeVisible (volumeKnob);
    volumeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::NoiseVolume, volumeKnob);

    // Details (envelope)
    hasDetails = true;
    addAndMakeVisible (detailsButton);

    styleToggle (constVolToggle, "ConstV");
    addChildComponent (constVolToggle);
    constVolAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::NoiseConstVol, constVolToggle);

    styleToggle (envLoopToggle, "EnvLp");
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
    addAndMakeVisible (enableToggle);
    enableAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, ParamIDs::DpcmEnabled, enableToggle);

    // Rate knob
    hasMainKnob = true;
    hasMainCombo = false;
    styleKnob (mainKnob);
    addAndMakeVisible (mainKnob);
    mainKnobAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::DpcmRate, mainKnob);

    // No volume — fixed
    hasVolume = false;
    styleLabel (volumeLabel, "Fixed", 11.0f, Colors::textSecondary);
    addAndMakeVisible (volumeLabel);

    // Details (loop toggle)
    hasDetails = true;
    addAndMakeVisible (detailsButton);

    styleToggle (dpcmLoopToggle, "Loop");
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
    // No enable toggle for VRC6 sub-channels (the whole VRC6 enable is global)

    // Duty knob (0–7)
    hasMainKnob = true;
    hasMainCombo = false;
    styleKnob (mainKnob);
    addAndMakeVisible (mainKnob);
    mainKnobAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, prefix + "Duty", mainKnob);

    // Transpose knob
    hasTranspose = true;
    styleKnob (transposeKnob);
    transposeKnob.setTextValueSuffix (" st");
    addAndMakeVisible (transposeKnob);
    transposeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, prefix + "Transpose", transposeKnob);

    // Volume knob
    hasVolume = true;
    styleKnob (volumeKnob);
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
    // No enable toggle for VRC6 sub-channels

    // Rate knob
    hasMainKnob = true;
    hasMainCombo = false;
    styleKnob (mainKnob);
    addAndMakeVisible (mainKnob);
    mainKnobAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, ParamIDs::Vrc6SawRate, mainKnob);

    // Transpose knob
    hasTranspose = true;
    styleKnob (transposeKnob);
    transposeKnob.setTextValueSuffix (" st");
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
    auto bounds = getLocalBounds().toFloat().reduced (1.0f);
    const float cornerR = 6.0f;

    // Rounded background
    g.setColour (Colors::bgStrip);
    g.fillRoundedRectangle (bounds, cornerR);

    // Subtle outline
    g.setColour (Colors::divider);
    g.drawRoundedRectangle (bounds, cornerR, 0.5f);

    // Accent stripe at top — clipped to panel bounds
    {
        g.saveState();
        g.reduceClipRegion (bounds.toNearestIntEdges());
        g.setColour (isVrc6() ? Colors::orangeAccent : Colors::accentActive);
        g.fillRect (bounds.withHeight (3.0f));
        g.restoreState();
    }

    // Activity LED — small circle to the left of the name label
    {
        auto nameBounds = nameLabel.getBounds();
        float ledSize = 8.0f;
        float ledX = static_cast<float> (nameBounds.getX()) - ledSize - 2.0f;
        float ledY = static_cast<float> (nameBounds.getCentreY()) - ledSize * 0.5f;

        if (ledState)
        {
            g.setColour (isVrc6() ? Colors::orangeAccent : Colors::accentActive);
            g.fillEllipse (ledX, ledY, ledSize, ledSize);
            // Glow effect
            g.setColour ((isVrc6() ? Colors::orangeAccent : Colors::accentActive).withAlpha (0.3f));
            g.fillEllipse (ledX - 2.0f, ledY - 2.0f, ledSize + 4.0f, ledSize + 4.0f);
        }
        else
        {
            g.setColour (Colors::knobOutline);
            g.drawEllipse (ledX, ledY, ledSize, ledSize, 1.0f);
        }
    }

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
    area.removeFromTop (2);  // accent stripe clearance
    const int pad = 3;
    const int sliderH = 22;
    const int labelH = 13;

    // --- Layout top to bottom ---

    auto nameRow = area.removeFromTop (16);
    nameLabel.setBounds (nameRow.withTrimmedLeft (14));  // Leave space for LED
    area.removeFromTop (pad);

    if (enableAttach != nullptr)
    {
        enableToggle.setBounds (area.removeFromTop (20));
        area.removeFromTop (pad);
    }

    // Main combo (if any)
    if (hasMainCombo)
    {
        area.removeFromTop (labelH);
        mainCombo.setBounds (area.removeFromTop (24).reduced (2, 0));
        area.removeFromTop (pad);
    }

    // Main knob/slider (if any)
    if (hasMainKnob)
    {
        area.removeFromTop (labelH);
        auto& knob = hasMainCombo ? noisePeriodKnob : mainKnob;
        knob.setBounds (area.removeFromTop (sliderH).reduced (2, 0));
        area.removeFromTop (pad);
    }
    else if (! hasMainCombo)
    {
        mainLabel.setBounds (area.removeFromTop (16));
        area.removeFromTop (pad);
    }

    // Transpose slider
    if (hasTranspose)
    {
        area.removeFromTop (labelH);
        transposeKnob.setBounds (area.removeFromTop (sliderH).reduced (2, 0));
        area.removeFromTop (pad);
    }

    // Volume slider or label
    if (hasVolume)
    {
        area.removeFromTop (labelH);
        volumeKnob.setBounds (area.removeFromTop (sliderH).reduced (2, 0));
        area.removeFromTop (pad);
    }
    else
    {
        volumeLabel.setBounds (area.removeFromTop (16));
        area.removeFromTop (pad);
    }

    // Details toggle + expandable area
    if (hasDetails)
    {
        // Align Details button across all channel types.
        // Pulse/Noise consume 160px before this point; pad shorter channels.
        static constexpr int targetConsumed = 160;
        int consumed = area.getY() - (getLocalBounds().reduced (3).getY());
        if (consumed < targetConsumed)
            area.removeFromTop (targetConsumed - consumed);

        detailsButton.setBounds (area.removeFromTop (20).reduced (4, 0));
        area.removeFromTop (pad);

        if (detailsVisible)
        {
            int detailH = juce::jmin (140, area.getHeight() - sliderH - labelH - pad);
            detailH = juce::jmax (0, detailH);
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

    // Mix fader — fixed height at bottom, label drawn by paint() above it
    mixFader.setBounds (area.removeFromBottom (sliderH).reduced (2, 0));
}

} // namespace cart
