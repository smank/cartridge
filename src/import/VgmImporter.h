#pragma once

#include <juce_core/juce_core.h>

namespace cart { class PresetManager; }

namespace cart {

/// Imports VGM/VGZ files and saves extracted instruments as user presets
/// in a subfolder of the user preset directory.
class VgmImporter
{
public:
    struct Result
    {
        bool success = false;
        int numInstruments = 0;
        juce::String gameName;
        juce::String error;
    };

    /// Import a VGM/VGZ file. Extracts per-channel instruments and saves
    /// each as a .xml preset in a subfolder named after the game/track.
    /// Returns the result with count of imported instruments.
    static Result importFile (const juce::File& file, PresetManager& presetManager);
};

} // namespace cart
