#pragma once

#include <juce_graphics/juce_graphics.h>

// Cartridge UI design system. Palette tiers, metrics, and font helpers
// shared across every component. Forked from the smank-plugins toolkit
// but tuned to Cartridge's NES-cartridge red identity with VRC6 orange.

namespace cart::ui
{

namespace Palette
{
    // Surface tiers — black -> charcoal -> graphite, for layered depth
    inline const juce::Colour background    { 0xFF141414 };
    inline const juce::Colour surface       { 0xFF222222 };
    inline const juce::Colour surfaceHi     { 0xFF2E2E2E };
    inline const juce::Colour surfaceAlt    { 0xFF282828 };
    inline const juce::Colour surfaceElev   { 0xFF383838 };

    inline const juce::Colour outline       { 0xFF505050 };
    inline const juce::Colour outlineDim    { 0xFF404040 };

    // Primary — NES cartridge red
    inline const juce::Colour primary       { 0xFFB82030 };
    inline const juce::Colour primaryDim    { 0xFF801828 };
    inline const juce::Colour primaryDark   { 0xFF501018 };

    // Secondary — bone / cream, for thumb dots, pointer accents
    inline const juce::Colour secondary     { 0xFFD8D4D0 };
    inline const juce::Colour secondaryDim  { 0xFF909090 };

    // Hot — bright red for hover / alert / emphasis
    inline const juce::Colour hot           { 0xFFD43040 };
    inline const juce::Colour hotBright    { 0xFFE84050 };

    // VRC6 — Konami expansion accent
    inline const juce::Colour vrc6Accent    { 0xFFD04828 };
    inline const juce::Colour vrc6Dim       { 0xFF903018 };

    // Text tiers
    inline const juce::Colour textPrimary   { 0xFFD8D4D0 };
    inline const juce::Colour textSecondary { 0xFF909090 };
    inline const juce::Colour textDim       { 0xFF606060 };

    // Keyboard
    inline const juce::Colour keyWhite      { 0xFFE8E4DF };
    inline const juce::Colour keyBlack      { 0xFF181818 };
    inline const juce::Colour keyDown       { 0xFFB82030 };

    // Waveform glow — used by WaveformDisplay's radial gradient
    inline const juce::Colour waveGlow      { 0xFFFF4858 };
    inline const juce::Colour waveDark      { 0xFF200810 };
}

namespace Metrics
{
    constexpr int windowDefaultWidth  = 940;
    constexpr int windowDefaultHeight = 720;
    constexpr int windowMinWidth      = 880;
    constexpr int windowMinHeight     = 660;

    constexpr int topBarHeight        = 96;
    constexpr int waveformHeight      = 60;
    constexpr int statusBarHeight     = 28;

    constexpr int sectionGap          = 6;
    constexpr int sectionInset        = 8;
    constexpr int cornerRadius        = 6;

    constexpr float knobStrokeWidth   = 2.5f;
    constexpr float outlineWidth      = 1.0f;

    constexpr int hitTargetMin        = 32;

    // Channel strip & accordion layout
    constexpr int channelHeaderHeight   = 38;
    constexpr int channelHeaderLedHit   = 32;
    constexpr int accordionDetailHeight = 140;

    // Common control dimensions used by strips and bars
    constexpr int sliderHeight          = 22;
    constexpr int comboHeight           = 24;
    constexpr int rotaryKnobDiameter    = 56;

    // Typography (px heights consumed by Theme.h's font helpers)
    constexpr float fontHeading         = 13.0f;
    constexpr float fontLabel           = 13.0f;
    constexpr float fontLabelSmall      = 12.0f;
    constexpr float fontValue           = 11.0f;
    constexpr float fontTiny            = 10.0f;
}

inline juce::Font displayFont (float size)
{
    juce::Font f (juce::FontOptions {}.withName ("Menlo").withHeight (size));
    f.setStyleFlags (juce::Font::bold);
    return f;
}

inline juce::Font labelFont (float size)
{
    return juce::Font (juce::FontOptions {}.withHeight (size));
}

inline juce::Font monoFont (float size)
{
    return juce::Font (juce::FontOptions {}.withName ("Menlo").withHeight (size));
}

} // namespace cart::ui
