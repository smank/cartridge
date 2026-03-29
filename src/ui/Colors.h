#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace cart::Colors {

// Backgrounds
inline const juce::Colour bgDark       { 0xFF1A1A1A };  // Black (vents, d-pad)
inline const juce::Colour bgMid        { 0xFF2D2D2D };  // Dark charcoal (console lower half)
inline const juce::Colour bgLight      { 0xFF3A3A3A };  // Charcoal (controller border)
inline const juce::Colour bgStrip      { 0xFF333333 };  // Strip background
inline const juce::Colour bgStripAlt   { 0xFF383838 };  // Alternating strip

// Primary accent red
inline const juce::Colour accentActive  { 0xFFA01020 };  // Active (maroon red)
inline const juce::Colour accentDim     { 0xFF701018 };  // Dimmed
inline const juce::Colour accentDark    { 0xFF400810 };  // Dark

// Secondary accent — Power LED red (brighter)
inline const juce::Colour fxAccent   { 0xFFCC2030 };  // FX accent
inline const juce::Colour fxBright   { 0xFFE83040 };  // Bright highlight

// VRC6 accent — warm red-orange
inline const juce::Colour orangeAccent { 0xFFC83020 };
inline const juce::Colour orangeDim    { 0xFF802018 };

// Text — Console label colors
inline const juce::Colour textPrimary  { 0xFFC8C4BF };  // Off-white (label area plastic)
inline const juce::Colour textSecondary{ 0xFF8C8C8C };  // Mid grey (console deck)
inline const juce::Colour textDark     { 0xFF5A5A5A };  // Dark grey

// Keyboard
inline const juce::Colour keyWhite     { 0xFFE8E4DF };  // Cream white
inline const juce::Colour keyBlack     { 0xFF1A1A1A };  // Black
inline const juce::Colour keyDown      { 0xFFA01020 };  // Red when pressed

// Controls
inline const juce::Colour knobFill     { 0xFF2A2A2A };  // Dark grey
inline const juce::Colour knobOutline  { 0xFF5A5A5A };  // Grey (plastic seams)
inline const juce::Colour faderTrack   { 0xFF1A1A1A };  // Black
inline const juce::Colour faderThumb   { 0xFFA01020 };  // Red

// Divider
inline const juce::Colour divider      { 0xFF4A4A4A };  // Subtle grey
inline const juce::Colour vrc6Divider  { 0xFFC83020 };  // VRC6 warm red-orange

} // namespace cart::Colors
