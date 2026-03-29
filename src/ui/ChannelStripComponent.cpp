#include "ChannelStripComponent.h"
#include "Parameters.h"

namespace cart {

namespace
{
    void styleKnob (juce::Slider& knob)
    {
        knob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        knob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 48, 14);
        knob.setColour (juce::Slider::rotarySliderFillColourId, Colors::accentActive);
        knob.setColour (juce::Slider::rotarySliderOutlineColourId, Colors::knobOutline);
        knob.setColour (juce::Slider::thumbColourId, Colors::accentActive);
        knob.setColour (juce::Slider::textBoxTextColourId, Colors::textSecondary);
        knob.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    }

    void styleFader (juce::Slider& fader)
    {
        fader.setSliderStyle (juce::Slider::LinearVertical);
        fader.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 48, 14);
        fader.setColour (juce::Slider::trackColourId, Colors::faderTrack);
        fader.setColour (juce::Slider::thumbColourId, Colors::faderThumb);
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
}

ChannelStripComponent::~ChannelStripComponent() = default;

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

    styleKnob (sweepPeriodKnob);
    addChildComponent (sweepPeriodKnob);
    sweepPeriodAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, prefix + "SweepPeriod", sweepPeriodKnob);

    styleToggle (sweepNegateToggle, "Neg");
    addChildComponent (sweepNegateToggle);
    sweepNegateAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        apvts, prefix + "SweepNegate", sweepNegateToggle);

    styleKnob (sweepShiftKnob);
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

    // No volume knob — fixed volume
    hasVolume = false;
    styleLabel (volumeLabel, "Fixed", 11.0f, Colors::textSecondary);
    addAndMakeVisible (volumeLabel);

    // Details (linear counter)
    hasDetails = true;
    addAndMakeVisible (detailsButton);

    styleKnob (linearReloadKnob);
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
    auto bounds = getLocalBounds().toFloat();

    // Rounded background
    g.setColour (Colors::bgStrip);
    g.fillRoundedRectangle (bounds.reduced (1.0f), 4.0f);

    // Subtle outline
    g.setColour (Colors::divider);
    g.drawRoundedRectangle (bounds.reduced (1.0f), 4.0f, 0.5f);
}

void ChannelStripComponent::resized()
{
    auto area = getLocalBounds().reduced (4);
    const int pad = 2;

    // Name label
    nameLabel.setBounds (area.removeFromTop (18));
    area.removeFromTop (pad);

    // Enable toggle (APU channels only, not VRC6 sub-channels)
    if (enableAttach != nullptr)
    {
        enableToggle.setBounds (area.removeFromTop (22));
        area.removeFromTop (pad);
    }

    // Main control area (60px)
    auto mainArea = area.removeFromTop (60);
    if (hasMainCombo && hasMainKnob)
    {
        // Noise: combo on top, period knob below
        mainCombo.setBounds (mainArea.removeFromTop (24).reduced (2, 0));
        noisePeriodKnob.setBounds (mainArea.reduced (2, 0));
    }
    else if (hasMainCombo)
    {
        mainCombo.setBounds (mainArea.removeFromTop (24).reduced (2, 0));
    }
    else if (hasMainKnob)
    {
        mainKnob.setBounds (mainArea.reduced (2, 0));
    }
    else
    {
        mainLabel.setBounds (mainArea);
    }
    area.removeFromTop (pad);

    // Volume knob (48px)
    auto volArea = area.removeFromTop (48);
    if (hasVolume)
        volumeKnob.setBounds (volArea.reduced (2, 0));
    else
        volumeLabel.setBounds (volArea);
    area.removeFromTop (pad);

    // Details toggle + details section
    if (hasDetails)
    {
        detailsButton.setBounds (area.removeFromTop (22).reduced (4, 0));
        area.removeFromTop (pad);

        if (detailsVisible)
        {
            auto detailArea = area.removeFromTop (100);
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
                    auto sweepRow = detailArea.removeFromTop (20);
                    sweepPeriodKnob.setBounds (sweepRow);
                    sweepNegateToggle.setBounds (detailArea.removeFromTop (rowH));
                    sweepShiftKnob.setBounds (detailArea);
                    break;
                }
                case ChannelType::Triangle:
                {
                    linearReloadKnob.setVisible (true);
                    linearControlToggle.setVisible (true);

                    linearReloadKnob.setBounds (detailArea.removeFromTop (50));
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
                case ChannelType::Vrc6Pulse1:
                case ChannelType::Vrc6Pulse2:
                case ChannelType::Vrc6Saw:
                    break;
            }
            area.removeFromTop (pad);
        }
        else
        {
            // Hide all detail controls
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

    // Mix fader (fills remaining space)
    mixFader.setBounds (area.reduced (8, 0));
}

} // namespace cart
