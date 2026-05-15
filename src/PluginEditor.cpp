#include "PluginEditor.h"
#if JUCE_STANDALONE_APPLICATION || JucePlugin_Build_Standalone
 #include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>
#endif

CartridgeEditor::CartridgeEditor (CartridgeProcessor& p)
    : AudioProcessorEditor (p),
      processorRef (p),
      topBar (p, p.getApvts()),
      waveformDisplay (p.waveformBuffer),
      channelStrips (p.getApvts()),
      modernPanel (p.getApvts()),
      effectsBar (p.getApvts()),
      modulationBar (p.getApvts(), p),
      statusBar (p),
      keyboard (p.getKeyboardState(), juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&lookAndFeel);

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

    // Wire up engine mode toggle
    topBar.onEngineToggle = [this] (bool modern)
    {
        modernModeActive = modern;
        channelStrips.setVisible (!modern);
        modernPanel.setVisible (modern);

        // Refresh VRC6 strip visibility when returning to Classic mode
        if (!modern)
        {
            auto* vrc6Param = processorRef.getApvts().getRawParameterValue (cart::ParamIDs::Vrc6Enabled);
            if (vrc6Param != nullptr)
                channelStrips.setVrc6Visible (vrc6Param->load() >= 0.5f);
        }

        resized();
    };

    // Wire up UI scaling from top bar
    topBar.onScaleChanged = [this] (float s) { applyScale (s); };

    // Wire up audio settings button (standalone only)
#if JUCE_STANDALONE_APPLICATION || JucePlugin_Build_Standalone
    if (auto* holder = juce::StandalonePluginHolder::getInstance())
    {
        topBar.onAudioSettings = []
        {
            auto* h = juce::StandalonePluginHolder::getInstance();
            if (h == nullptr) return;

            // Build our own dialog so we can hide the Bluetooth MIDI button
            // (it crashes on unsigned builds without BT entitlements)
            auto content = std::make_unique<juce::AudioDeviceSelectorComponent> (
                h->deviceManager, 0, 0, 0, 2, true, false, true, false);
            content->setSize (500, 550);

            // Walk children and hide any Bluetooth-related button
            for (auto* child : content->getChildren())
            {
                if (auto* btn = dynamic_cast<juce::TextButton*> (child))
                {
                    if (btn->getButtonText().containsIgnoreCase ("bluetooth"))
                        btn->setVisible (false);
                }
            }

            juce::DialogWindow::LaunchOptions o;
            o.content.setOwned (content.release());
            o.dialogTitle = "Audio/MIDI Settings";
            o.dialogBackgroundColour = cart::ui::Palette::surface;
            o.escapeKeyTriggersCloseButton = true;
            o.useNativeTitleBar = true;
            o.resizable = false;
            o.launchAsync();
        };
        topBar.showAudioSettingsButton (true);
    }
#endif

    // Wire up DPCM sample loading from modulation bar
    modulationBar.onDpcmLoad = [this] (const juce::File& file)
    {
        int slot = processorRef.getDpcmSampleManager().loadFromFile (file);
        if (slot >= 0)
        {
            // Select the newly loaded sample
            if (auto* param = processorRef.getApvts().getParameter (cart::ParamIDs::DpcmSample))
                param->setValueNotifyingHost (param->convertTo0to1 (static_cast<float> (slot)));
        }
    };

    // Wire up FX/Mod panel height changes — mutually exclusive expansion
    effectsBar.onHeightChanged = [this]
    {
        if (effectsBar.getDesiredHeight() > effectsBar.headerHeight)
            modulationBar.collapseAll();
        resized();
    };
    modulationBar.onHeightChanged = [this]
    {
        if (modulationBar.getDesiredHeight() > modulationBar.headerHeight)
            effectsBar.collapseAll();
        resized();
    };

    // Style the keyboard
    keyboard.setColour (juce::MidiKeyboardComponent::whiteNoteColourId,
                        cart::ui::Palette::keyWhite);
    keyboard.setColour (juce::MidiKeyboardComponent::blackNoteColourId,
                        cart::ui::Palette::keyBlack);
    keyboard.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId,
                        cart::ui::Palette::background);
    keyboard.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId,
                        cart::ui::Palette::keyDown.withAlpha (0.6f));
    keyboard.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId,
                        cart::ui::Palette::primaryDim.withAlpha (0.3f));
    keyboard.setColour (juce::MidiKeyboardComponent::shadowColourId,
                        cart::ui::Palette::background);
    keyboard.setColour (juce::MidiKeyboardComponent::upDownButtonBackgroundColourId,
                        cart::ui::Palette::surface);
    keyboard.setColour (juce::MidiKeyboardComponent::upDownButtonArrowColourId,
                        cart::ui::Palette::textPrimary);
    keyboard.setKeyWidth (24.0f);
    keyboard.setMidiChannel (1);
    keyboard.setAvailableRange (36, 96); // C2 to C7 (~5 octaves)
    keyboard.setKeyPressBaseOctave (5);  // QWERTY keys start at C4 (middle C)
    keyboard.setVelocity (0.8f, true);   // QWERTY velocity; mouse uses click position

    // Wire activity LED flags from processor to channel strips
    channelStrips.setActivityFlags (p.channelActive);

    addAndMakeVisible (topBar);
    addAndMakeVisible (waveformDisplay);
    addAndMakeVisible (channelStrips);
    addChildComponent (modernPanel);  // Hidden by default (Classic mode)
    addAndMakeVisible (effectsBar);
    addAndMakeVisible (modulationBar);
    addAndMakeVisible (statusBar);
    addAndMakeVisible (keyboard);

    // Sync initial VRC6 state
    auto* vrc6Param = p.getApvts().getRawParameterValue (cart::ParamIDs::Vrc6Enabled);
    if (vrc6Param != nullptr)
        channelStrips.setVrc6Visible (vrc6Param->load() >= 0.5f);

    // Sync initial engine mode
    auto* engineModeParam = p.getApvts().getRawParameterValue (cart::ParamIDs::EngineMode);
    if (engineModeParam != nullptr)
    {
        modernModeActive = (static_cast<int> (engineModeParam->load()) == 1);
        channelStrips.setVisible (!modernModeActive);
        modernPanel.setVisible (modernModeActive);
    }

    // Load saved scale (just the value, don't resize yet)
    loadScalePreference();

    int initW = juce::roundToInt (baseWidth * currentScale);
    int initH = juce::roundToInt (baseHeight * currentScale);

    setSize (initW, initH);
    setResizable (true, true);
    setResizeLimits (baseWidth, baseHeight,
                     baseWidth * maxScalePercent / 100, baseHeight * maxScalePercent / 100);
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
                                  cart::ui::Palette::background);
            docWindow->setUsingNativeTitleBar (true);

            if (auto* peer = docWindow->getPeer())
                cart::enableTitleBarFullscreen (peer->getNativeHandle());
        }

        // Re-apply scaled size after native title bar recreates the window peer
        safeThis->setSize (juce::roundToInt (baseWidth * safeThis->currentScale),
                           juce::roundToInt (baseHeight * safeThis->currentScale));

        // Centre the window on screen
        if (auto* topLevel = safeThis->getTopLevelComponent())
            topLevel->centreWithSize (topLevel->getWidth(), topLevel->getHeight());
    });
}

CartridgeEditor::~CartridgeEditor()
{
    setLookAndFeel (nullptr);
}

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
    setResizeLimits (baseWidth, baseHeight,
                     baseWidth * maxScalePercent / 100, baseHeight * maxScalePercent / 100);
    setSize (w, h);
    saveScalePreference();
}

void CartridgeEditor::loadScalePreference()
{
    auto file = getSettingsFile();
    if (! file.existsAsFile())
    {
        // First launch: auto-detect best scale for the primary display
        auto display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay();
        if (display != nullptr)
        {
            auto screenArea = display->userArea;
            float scaleW = static_cast<float> (screenArea.getWidth()) / static_cast<float> (baseWidth);
            float scaleH = static_cast<float> (screenArea.getHeight()) / static_cast<float> (baseHeight);
            float fit = juce::jmin (scaleW, scaleH) * 0.9f;  // 90% of screen

            // Snap to nearest scale option
            if      (fit >= 1.75f) { currentScale = 2.0f;  topBar.setScaleIndex (4); }
            else if (fit >= 1.45f) { currentScale = 1.5f;  topBar.setScaleIndex (3); }
            else if (fit >= 1.2f)  { currentScale = 1.25f; topBar.setScaleIndex (2); }
            else                   { currentScale = 1.0f;  topBar.setScaleIndex (1); }
        }
        return;
    }

    if (auto xml = juce::parseXML (file))
    {
        float scale = static_cast<float> (xml->getDoubleAttribute ("uiScale", 1.0));

        if (scale < 1.0f) scale = 1.0f;
        int comboId = 1;
        if      (scale <= 1.01f) comboId = 1;
        else if (scale <= 1.26f) comboId = 2;
        else if (scale <= 1.51f) comboId = 3;
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
    using namespace cart::ui;
    auto bounds = getLocalBounds().toFloat();

    // Subtle vertical gradient — slightly lighter at the top so the
    // top bar feels like a "control surface" and the keyboard reads
    // as a foot rest rather than identical-flat.
    g.setGradientFill (juce::ColourGradient (
        Palette::background.brighter (0.04f), 0.0f, 0.0f,
        Palette::background.darker (0.10f),  0.0f, bounds.getHeight(),
        false));
    g.fillRect (bounds);

    // Hairline divider under the top bar
    g.setColour (Palette::primary.withAlpha (0.35f));
    g.fillRect (0, topBarHeight - 1, getWidth(), 1);
}

void CartridgeEditor::resized()
{
    // Use full window bounds — the layout stretches to fill
    auto area = getLocalBounds();

    topBar.setBounds (area.removeFromTop (topBarHeight));
    waveformDisplay.setBounds (area.removeFromTop (waveformHeight).reduced (4, 2));

    // Scale keyboard height proportionally (20% of window, clamped)
    int kbHeight = juce::jlimit (100, 220, getHeight() / 5);
    keyboard.setBounds (area.removeFromBottom (kbHeight));

    // Scale key width to fill the keyboard width (C2–C7 = 36 white keys)
    static constexpr int numWhiteKeys = 36;
    float kw = static_cast<float> (keyboard.getWidth()) / static_cast<float> (numWhiteKeys);
    keyboard.setKeyWidth (juce::jmax (kw, 14.0f));

    statusBar.setBounds (area.removeFromBottom (statusBarHeight));

    // Calculate panel heights, capping them to preserve minimum channel strip space
    static constexpr int minStripHeight = 240;
    int fxDesired  = effectsBar.getDesiredHeight();
    int modDesired = modulationBar.getDesiredHeight();
    int panelsTotal = fxDesired + modDesired;
    int available = area.getHeight();

    if (available - panelsTotal < minStripHeight)
    {
        // Not enough room — shrink the expanded panel's detail area
        int budget = juce::jmax (0, available - minStripHeight);
        int fxHeader  = effectsBar.headerHeight;
        int modHeader = modulationBar.headerHeight;

        if (fxDesired > fxHeader && modDesired <= modHeader)
        {
            // Only FX is expanded
            fxDesired = juce::jmax (fxHeader, budget - modHeader);
            modDesired = modHeader;
        }
        else if (modDesired > modHeader && fxDesired <= fxHeader)
        {
            // Only Mod is expanded
            modDesired = juce::jmax (modHeader, budget - fxHeader);
            fxDesired = fxHeader;
        }
        else
        {
            // Both collapsed or both somehow open — just use headers
            fxDesired = fxHeader;
            modDesired = modHeader;
        }
    }

    effectsBar.setBounds (area.removeFromBottom (fxDesired));
    modulationBar.setBounds (area.removeFromBottom (modDesired));

    if (modernModeActive)
    {
        modernPanel.setBounds (area);
        channelStrips.setVisible (false);
        modernPanel.setVisible (true);
    }
    else
    {
        channelStrips.setBounds (area);
        channelStrips.setVisible (true);
        modernPanel.setVisible (false);
    }
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
        statusBar.setVelocity (currentVelocity);
        statusBar.setOctaveOffset (currentOctaveOffset);
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
        statusBar.setVelocity (currentVelocity);
        statusBar.setOctaveOffset (currentOctaveOffset);
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
        statusBar.setVelocity (currentVelocity);
        statusBar.setOctaveOffset (currentOctaveOffset);
        return true;
    }

    // = = velocity up
    if (keyCode == '=')
    {
        currentVelocity = juce::jmin (1.0f, currentVelocity + 0.1f);
        keyboard.setVelocity (currentVelocity, true);
        statusBar.setVelocity (currentVelocity);
        statusBar.setOctaveOffset (currentOctaveOffset);
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
