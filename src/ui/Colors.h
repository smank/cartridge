#pragma once

// Compatibility shim — forwards the legacy cart::Colors:: names to the
// new cart::ui::Palette in Theme.h. Existing components reference these
// names; Phase 4 of the UI redesign will sweep them to Palette directly
// and delete this file.

#include "Theme.h"

namespace cart::Colors {

// Surface tiers
inline const juce::Colour& bgDark       = cart::ui::Palette::background;
inline const juce::Colour& bgMid        = cart::ui::Palette::surface;
inline const juce::Colour& bgLight      = cart::ui::Palette::surfaceHi;
inline const juce::Colour& bgStrip      = cart::ui::Palette::surfaceAlt;
inline const juce::Colour  bgStripAlt   { 0xFF303030 };
inline const juce::Colour& bgElevated   = cart::ui::Palette::surfaceElev;

// Primary accent — NES cartridge red
inline const juce::Colour& accentActive = cart::ui::Palette::primary;
inline const juce::Colour& accentDim    = cart::ui::Palette::primaryDim;
inline const juce::Colour& accentDark   = cart::ui::Palette::primaryDark;

// Secondary accent — bright highlights
inline const juce::Colour& fxAccent     = cart::ui::Palette::hot;
inline const juce::Colour& fxBright     = cart::ui::Palette::hotBright;

// VRC6 accent — Konami orange
inline const juce::Colour& orangeAccent = cart::ui::Palette::vrc6Accent;
inline const juce::Colour& orangeDim    = cart::ui::Palette::vrc6Dim;

// Text tiers
inline const juce::Colour& textPrimary  = cart::ui::Palette::textPrimary;
inline const juce::Colour& textSecondary = cart::ui::Palette::textSecondary;
inline const juce::Colour& textDark     = cart::ui::Palette::textDim;

// Keyboard
inline const juce::Colour& keyWhite     = cart::ui::Palette::keyWhite;
inline const juce::Colour& keyBlack     = cart::ui::Palette::keyBlack;
inline const juce::Colour& keyDown      = cart::ui::Palette::keyDown;

// Controls
inline const juce::Colour  knobFill     { 0xFF242424 };
inline const juce::Colour& knobOutline  = cart::ui::Palette::outline;
inline const juce::Colour  faderTrack   { 0xFF181818 };
inline const juce::Colour& faderThumb   = cart::ui::Palette::primary;

// Dividers
inline const juce::Colour& divider      = cart::ui::Palette::outlineDim;
inline const juce::Colour& vrc6Divider  = cart::ui::Palette::vrc6Accent;

} // namespace cart::Colors
