#include "PluginEditor.h"

CartridgeEditor::CartridgeEditor (CartridgeProcessor& p)
    : AudioProcessorEditor (p),
      processorRef (p),
      topBar (p, p.getApvts()),
      channelStrips (p.getApvts()),
      effectsBar (p.getApvts()),
      modulationBar (p.getApvts()),
      statusBar (p),
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

    // Wire up UI scaling from top bar
    topBar.onScaleChanged = [this] (float s) { applyScale (s); };

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
    addAndMakeVisible (statusBar);
    addAndMakeVisible (keyboard);

    // Sync initial VRC6 state
    auto* vrc6Param = p.getApvts().getRawParameterValue (cart::ParamIDs::Vrc6Enabled);
    if (vrc6Param != nullptr)
        channelStrips.setVrc6Visible (vrc6Param->load() >= 0.5f);

    // Load saved scale (just the value, don't resize yet)
    loadScalePreference();

    int initW = juce::roundToInt (baseWidth * currentScale);
    int initH = juce::roundToInt (baseHeight * currentScale);

    setSize (initW, initH);
    setResizable (true, true);
    setResizeLimits (juce::jmin (initW, 700), juce::jmin (initH, 560), 3840, 2160);
    setWantsKeyboardFocus (true);

    // Editor keeps focus so shortcut keys ([, ], -, =, Tab, etc.) are handled
    // first; unhandled QWERTY keys are forwarded to the keyboard at the end of
    // keyPressed() for MIDI note input.
    grabKeyboardFocus();

    // Deferred: switch standalone window to native macOS title bar (traffic lights
    // + double-click fullscreen).  Must run after the window is fully constructed.
    // Re-apply size after the title bar switch to prevent the peer reset from
    // snapping back to a stale size.
    juce::Timer::callAfterDelay (200, [safeThis = juce::Component::SafePointer<CartridgeEditor> (this)]
    {
        if (safeThis == nullptr)
            return;

        if (auto* docWindow = dynamic_cast<juce::DocumentWindow*> (safeThis->getTopLevelComponent()))
        {
            docWindow->setColour (juce::ResizableWindow::backgroundColourId,
                                  cart::Colors::bgDark);
            docWindow->setUsingNativeTitleBar (true);

            if (auto* peer = docWindow->getPeer())
                cart::enableTitleBarFullscreen (peer->getNativeHandle());
        }

        // Re-apply scaled size after native title bar recreates the window peer
        safeThis->setSize (juce::roundToInt (baseWidth * safeThis->currentScale),
                           juce::roundToInt (baseHeight * safeThis->currentScale));
    });
}

CartridgeEditor::~CartridgeEditor() = default;

juce::File CartridgeEditor::getSettingsFile() const
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
               .getChildFile ("Cartridge")
               .getChildFile ("ui-settings.xml");
}

void CartridgeEditor::applyScale (float scale)
{
    currentScale = scale;
    int w = juce::roundToInt (baseWidth * scale);
    int h = juce::roundToInt (baseHeight * scale);
    setResizeLimits (juce::jmin (w, 700), juce::jmin (h, 560), 3840, 2160);
    setSize (w, h);
    saveScalePreference();
}

void CartridgeEditor::loadScalePreference()
{
    auto file = getSettingsFile();
    if (! file.existsAsFile())
        return;

    if (auto xml = juce::parseXML (file))
    {
        float scale = static_cast<float> (xml->getDoubleAttribute ("uiScale", 1.0));

        // Map scale to combo ID: 0.75→1, 1.0→2, 1.25→3, 1.5→4
        int comboId = 2;
        if      (scale <= 0.76f) comboId = 1;
        else if (scale <= 1.01f) comboId = 2;
        else if (scale <= 1.26f) comboId = 3;
        else                     comboId = 4;

        topBar.setScaleIndex (comboId);
        currentScale = scale;
    }
}

void CartridgeEditor::saveScalePreference()
{
    auto file = getSettingsFile();
    file.getParentDirectory().createDirectory();

    juce::XmlElement xml ("CartridgeSettings");
    xml.setAttribute ("uiScale", static_cast<double> (currentScale));
    xml.writeTo (file);
}

void CartridgeEditor::paint (juce::Graphics& g)
{
    g.fillAll (cart::Colors::bgDark);
}

void CartridgeEditor::resized()
{
    // Center all content within a max-width column; bgDark fills the margins
    static constexpr int maxContentWidth = 1600;
    auto area = getLocalBounds();
    if (area.getWidth() > maxContentWidth)
        area = area.withSizeKeepingCentre (maxContentWidth, area.getHeight());

    topBar.setBounds (area.removeFromTop (topBarHeight));

    // Scale keyboard height proportionally (20% of window, clamped)
    int kbHeight = juce::jlimit (100, 220, getHeight() / 5);
    keyboard.setBounds (area.removeFromBottom (kbHeight));

    // Scale key width to fill the keyboard width (C2–C7 = 36 white keys)
    static constexpr int numWhiteKeys = 36;
    float kw = static_cast<float> (keyboard.getWidth()) / static_cast<float> (numWhiteKeys);
    keyboard.setKeyWidth (juce::jmax (kw, 14.0f));

    statusBar.setBounds (area.removeFromBottom (statusBarHeight));
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

    // Ctrl+Cmd+F = Toggle fullscreen (standard macOS shortcut)
    if ((keyCode == 'F' || keyCode == 'f')
        && key.getModifiers().isCommandDown()
        && key.getModifiers().isCtrlDown())
    {
        if (auto* tlc = getTopLevelComponent())
            if (auto* peer = tlc->getPeer())
                peer->setFullScreen (! peer->isFullScreen());
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
        statusBar.update (channelStrips.getFocusedChannel(), currentVelocity, currentOctaveOffset, processorRef.getHoldMode());
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
        statusBar.update (channelStrips.getFocusedChannel(), currentVelocity, currentOctaveOffset, processorRef.getHoldMode());
        return true;
    }

    // Tab / Shift+Tab = cycle focused channel + switch MIDI channel for QWERTY
    if (keyCode == juce::KeyPress::tabKey)
    {
        int maxCh = channelStrips.isVrc6Visible() ? 7 : 4;
        int fc = channelStrips.getFocusedChannel();

        if (key.getModifiers().isShiftDown())
            fc = (fc > 0) ? fc - 1 : maxCh;
        else
            fc = (fc < maxCh) ? fc + 1 : 0;

        channelStrips.setFocusedChannel (fc);

        // Map channel index → MIDI channel (matches split-mode routing)
        static constexpr int chToMidi[] = { 1, 2, 3, 10, 4, 5, 6, 7 };
        keyboard.setMidiChannel (chToMidi[fc]);
        statusBar.update (fc, currentVelocity, currentOctaveOffset, processorRef.getHoldMode());
        return true;
    }

    // \ = toggle hold/latch mode (notes ring until toggled off)
    if (keyCode == '\\')
    {
        bool current = processorRef.getHoldMode();
        processorRef.setHoldMode (!current);
        statusBar.update (channelStrips.getFocusedChannel(), currentVelocity, currentOctaveOffset, !current);
        return true;
    }

    // - = velocity down
    if (keyCode == '-')
    {
        currentVelocity = juce::jmax (0.1f, currentVelocity - 0.1f);
        keyboard.setVelocity (currentVelocity, true);
        statusBar.update (channelStrips.getFocusedChannel(), currentVelocity, currentOctaveOffset, processorRef.getHoldMode());
        return true;
    }

    // = = velocity up
    if (keyCode == '=')
    {
        currentVelocity = juce::jmin (1.0f, currentVelocity + 0.1f);
        keyboard.setVelocity (currentVelocity, true);
        statusBar.update (channelStrips.getFocusedChannel(), currentVelocity, currentOctaveOffset, processorRef.getHoldMode());
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

    // Escape = unsolo (restore all channel mixes to full)
    if (keyCode == juce::KeyPress::escapeKey)
    {
        const char* mixParams[] = {
            cart::ParamIDs::P1Mix, cart::ParamIDs::P2Mix,
            cart::ParamIDs::TriMix, cart::ParamIDs::NoiseMix,
            cart::ParamIDs::DpcmMix, cart::ParamIDs::Vrc6P1Mix,
            cart::ParamIDs::Vrc6P2Mix, cart::ParamIDs::Vrc6SawMix
        };
        for (auto* id : mixParams)
        {
            if (auto* param = apvts.getParameter (id))
                param->setValueNotifyingHost (1.0f);
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
