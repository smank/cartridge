#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace cart::Colors {

// Backgrounds — wider spread for better depth
inline const juce::Colour bgDark       { 0xFF141414 };  // Deepest background
inline const juce::Colour bgMid        { 0xFF222222 };  // Main panels
inline const juce::Colour bgLight      { 0xFF2E2E2E };  // Raised surfaces
inline const juce::Colour bgStrip      { 0xFF282828 };  // Channel strip fill
inline const juce::Colour bgStripAlt   { 0xFF303030 };  // Alternating strip
inline const juce::Colour bgElevated   { 0xFF383838 };  // Buttons, combo boxes

// Primary accent — NES cartridge red
inline const juce::Colour accentActive  { 0xFFB82030 };  // Primary interactive
inline const juce::Colour accentDim     { 0xFF801828 };  // Pressed / track fill
inline const juce::Colour accentDark    { 0xFF501018 };  // Deep shadow

// Secondary accent — bright highlights
inline const juce::Colour fxAccent   { 0xFFD43040 };  // FX / emphasis
inline const juce::Colour fxBright   { 0xFFE84050 };  // Bright hover / alert

// VRC6 accent — warm orange-red
inline const juce::Colour orangeAccent { 0xFFD04828 };
inline const juce::Colour orangeDim    { 0xFF903018 };

// Text — clear hierarchy
inline const juce::Colour textPrimary  { 0xFFD8D4D0 };  // Primary labels
inline const juce::Colour textSecondary{ 0xFF909090 };  // Secondary / dimmed
inline const juce::Colour textDark     { 0xFF606060 };  // Subtle hints

// Keyboard
inline const juce::Colour keyWhite     { 0xFFE8E4DF };
inline const juce::Colour keyBlack     { 0xFF181818 };
inline const juce::Colour keyDown      { 0xFFB82030 };

// Controls
inline const juce::Colour knobFill     { 0xFF242424 };
inline const juce::Colour knobOutline  { 0xFF505050 };
inline const juce::Colour faderTrack   { 0xFF181818 };
inline const juce::Colour faderThumb   { 0xFFB82030 };

// Divider
inline const juce::Colour divider      { 0xFF404040 };
inline const juce::Colour vrc6Divider  { 0xFFD04828 };

} // namespace cart::Colors
