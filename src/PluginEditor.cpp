#include "PluginEditor.h"

CartridgeEditor::CartridgeEditor (CartridgeProcessor& p)
    : AudioProcessorEditor (p),
      processorRef (p),
      topBar (p, p.getApvts()),
      channelStrips (p.getApvts()),
      effectsBar (p.getApvts()),
      modulationBar (p.getApvts()),
      keyboard (p.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    // Wire up VRC6 toggle from top bar to channel strip area
    topBar.onVrc6Toggle = [this] (bool visible)
    {
        channelStrips.setVrc6Visible (visible);
    };

    // Collapse expanded FX/mod panels on preset change
    topBar.onPresetChanged = [this]
    {
        effectsBar.collapseAll();
        modulationBar.collapseAll();
    };

    // Wire up FX panel height changes
    effectsBar.onHeightChanged = [this] { resized(); };
    modulationBar.onHeightChanged = [this] { resized(); };

    // Style the keyboard
    keyboard.setColour (juce::MidiKeyboardComponent::whiteNoteColourId,
                        cart::Colors::keyWhite);
    keyboard.setColour (juce::MidiKeyboardComponent::blackNoteColourId,
                        cart::Colors::keyBlack);
    keyboard.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId,
                        cart::Colors::bgDark);
    keyboard.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId,
                        cart::Colors::keyDown.withAlpha (0.6f));
    keyboard.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId,
                        cart::Colors::accentDim.withAlpha (0.3f));
    keyboard.setColour (juce::MidiKeyboardComponent::shadowColourId,
                        cart::Colors::bgDark);
    keyboard.setColour (juce::MidiKeyboardComponent::upDownButtonBackgroundColourId,
                        cart::Colors::bgMid);
    keyboard.setColour (juce::MidiKeyboardComponent::upDownButtonArrowColourId,
                        cart::Colors::textPrimary);
    keyboard.setKeyWidth (24.0f);
    keyboard.setMidiChannel (1);
    keyboard.setAvailableRange (36, 96); // C2 to C7 (~5 octaves)
    keyboard.setKeyPressBaseOctave (5);  // QWERTY keys start at C4 (middle C)
    keyboard.setVelocity (0.8f, true);   // QWERTY velocity; mouse uses click position

    addAndMakeVisible (topBar);
    addAndMakeVisible (channelStrips);
    addAndMakeVisible (effectsBar);
    addAndMakeVisible (modulationBar);
    addAndMakeVisible (keyboard);

    // Sync initial VRC6 state
    auto* vrc6Param = p.getApvts().getRawParameterValue (cart::ParamIDs::Vrc6Enabled);
    if (vrc6Param != nullptr)
        channelStrips.setVrc6Visible (vrc6Param->load() >= 0.5f);

    setSize (defaultWidth, defaultHeight);
    setResizable (true, true);
    setResizeLimits (700, 560, 1600, 1100);
    setWantsKeyboardFocus (true);

    // Give keyboard focus so QWERTY-to-MIDI works immediately
    keyboard.grabKeyboardFocus();
}

CartridgeEditor::~CartridgeEditor() = default;

void CartridgeEditor::paint (juce::Graphics& g)
{
    g.fillAll (cart::Colors::bgDark);
}

void CartridgeEditor::resized()
{
    auto area = getLocalBounds();

    topBar.setBounds (area.removeFromTop (topBarHeight));
    keyboard.setBounds (area.removeFromBottom (keyboardHeight));
    effectsBar.setBounds (area.removeFromBottom (effectsBar.getDesiredHeight()));
    modulationBar.setBounds (area.removeFromBottom (modulationBar.getDesiredHeight()));
    channelStrips.setBounds (area);
}

bool CartridgeEditor::keyPressed (const juce::KeyPress& key)
{
    auto& apvts = processorRef.getApvts();

    auto toggleParam = [&] (const char* paramID)
    {
        if (auto* param = apvts.getParameter (paramID))
        {
            float current = param->getValue();
            param->setValueNotifyingHost (current < 0.5f ? 1.0f : 0.0f);
        }
    };

    // Preset navigation
    if (key == juce::KeyPress::leftKey)
    {
        topBar.navigatePreset (-1);
        return true;
    }
    if (key == juce::KeyPress::rightKey)
    {
        topBar.navigatePreset (1);
        return true;
    }

    int keyCode = key.getKeyCode();

    // Channel toggles: 1-5 = APU channels
    if (keyCode == '1') { toggleParam (cart::ParamIDs::P1Enabled);    return true; }
    if (keyCode == '2') { toggleParam (cart::ParamIDs::P2Enabled);    return true; }
    if (keyCode == '3') { toggleParam (cart::ParamIDs::TriEnabled);   return true; }
    if (keyCode == '4') { toggleParam (cart::ParamIDs::NoiseEnabled); return true; }
    if (keyCode == '5') { toggleParam (cart::ParamIDs::DpcmEnabled);  return true; }

    // VRC6 channel toggles: 6-8
    if (keyCode == '6')
    {
        // Toggle VRC6 Pulse 1 mix (0 or 1)
        if (auto* param = apvts.getParameter (cart::ParamIDs::Vrc6P1Mix))
        {
            float current = param->getValue();
            param->setValueNotifyingHost (current < 0.5f ? 1.0f : 0.0f);
        }
        return true;
    }
    if (keyCode == '7')
    {
        if (auto* param = apvts.getParameter (cart::ParamIDs::Vrc6P2Mix))
        {
            float current = param->getValue();
            param->setValueNotifyingHost (current < 0.5f ? 1.0f : 0.0f);
        }
        return true;
    }
    if (keyCode == '8')
    {
        if (auto* param = apvts.getParameter (cart::ParamIDs::Vrc6SawMix))
        {
            float current = param->getValue();
            param->setValueNotifyingHost (current < 0.5f ? 1.0f : 0.0f);
        }
        return true;
    }

    // Ctrl+V = Toggle VRC6 expansion (plain V is a piano key)
    if ((keyCode == 'V' || keyCode == 'v') && key.getModifiers().isCommandDown())
    {
        toggleParam (cart::ParamIDs::Vrc6Enabled);
        return true;
    }

    // Space = All notes off (panic)
    if (keyCode == juce::KeyPress::spaceKey)
    {
        processorRef.getKeyboardState().allNotesOff (0);
        return true;
    }

    // [ = Octave down
    if (keyCode == '[')
    {
        if (currentOctaveOffset > -3)
        {
            currentOctaveOffset--;
            int low  = 36 + currentOctaveOffset * 12;
            int high = 96 + currentOctaveOffset * 12;
            keyboard.setAvailableRange (juce::jmax (0, low), juce::jmin (127, high));
            keyboard.setKeyPressBaseOctave (5 + currentOctaveOffset);
        }
        return true;
    }

    // ] = Octave up
    if (keyCode == ']')
    {
        if (currentOctaveOffset < 2)
        {
            currentOctaveOffset++;
            int low  = 36 + currentOctaveOffset * 12;
            int high = 96 + currentOctaveOffset * 12;
            keyboard.setAvailableRange (juce::jmax (0, low), juce::jmin (127, high));
            keyboard.setKeyPressBaseOctave (5 + currentOctaveOffset);
        }
        return true;
    }

    // Tab = cycle focused channel + switch MIDI channel for QWERTY
    if (keyCode == juce::KeyPress::tabKey)
    {
        int maxCh = channelStrips.isVrc6Visible() ? 7 : 4;
        int fc = channelStrips.getFocusedChannel();
        fc = (fc < maxCh) ? fc + 1 : 0;
        channelStrips.setFocusedChannel (fc);

        // Map channel index → MIDI channel (matches split-mode routing)
        static constexpr int chToMidi[] = { 1, 2, 3, 10, 4, 5, 6, 7 };
        keyboard.setMidiChannel (chToMidi[fc]);
        return true;
    }

    // \ = toggle hold/latch mode (notes ring until toggled off)
    if (keyCode == '\\')
    {
        bool current = processorRef.getHoldMode();
        processorRef.setHoldMode (!current);
        return true;
    }

    // - = velocity down
    if (keyCode == '-')
    {
        currentVelocity = juce::jmax (0.1f, currentVelocity - 0.1f);
        keyboard.setVelocity (currentVelocity, true);
        return true;
    }

    // = = velocity up
    if (keyCode == '=')
    {
        currentVelocity = juce::jmin (1.0f, currentVelocity + 0.1f);
        keyboard.setVelocity (currentVelocity, true);
        return true;
    }

    // Ctrl+S = Solo focused channel (toggle mix of focused vs all others)
    if ((keyCode == 'S' || keyCode == 's') && key.getModifiers().isCommandDown())
    {
        int fc = channelStrips.getFocusedChannel();
        if (fc >= 0)
        {
            const char* mixParams[] = {
                cart::ParamIDs::P1Mix, cart::ParamIDs::P2Mix,
                cart::ParamIDs::TriMix, cart::ParamIDs::NoiseMix,
                cart::ParamIDs::DpcmMix, cart::ParamIDs::Vrc6P1Mix,
                cart::ParamIDs::Vrc6P2Mix, cart::ParamIDs::Vrc6SawMix
            };
            int numMixes = 8;
            for (int i = 0; i < numMixes; ++i)
            {
                if (auto* param = apvts.getParameter (mixParams[i]))
                    param->setValueNotifyingHost (i == fc ? 1.0f : 0.0f);
            }
        }
        return true;
    }

    // Ctrl+M = Mute focused channel
    if ((keyCode == 'M' || keyCode == 'm') && key.getModifiers().isCommandDown())
    {
        int fc = channelStrips.getFocusedChannel();
        if (fc >= 0)
        {
            const char* mixParams[] = {
                cart::ParamIDs::P1Mix, cart::ParamIDs::P2Mix,
                cart::ParamIDs::TriMix, cart::ParamIDs::NoiseMix,
                cart::ParamIDs::DpcmMix, cart::ParamIDs::Vrc6P1Mix,
                cart::ParamIDs::Vrc6P2Mix, cart::ParamIDs::Vrc6SawMix
            };
            if (fc < 8)
            {
                if (auto* param = apvts.getParameter (mixParams[fc]))
                {
                    float current = param->getValue();
                    param->setValueNotifyingHost (current < 0.5f ? 1.0f : 0.0f);
                }
            }
        }
        return true;
    }

    // Forward unhandled keys to keyboard for QWERTY-to-MIDI
    return keyboard.keyPressed (key);
}

bool CartridgeEditor::keyStateChanged (bool isKeyDown)
{
    // Forward key-up events to keyboard for MIDI note-off
    return keyboard.keyStateChanged (isKeyDown);
}
