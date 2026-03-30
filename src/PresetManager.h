#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Parameters.h"
#include <vector>
#include <optional>

namespace cart {

struct Preset
{
    juce::String name;
    std::vector<std::pair<juce::String, float>> values;  // paramID → value
};

/// Factory + user presets for Cartridge.
class PresetManager
{
public:
    PresetManager();

    int getNumPresets() const                   { return static_cast<int> (presets.size()); }
    const juce::String& getPresetName (int i) const;
    void applyPreset (int index, juce::AudioProcessorValueTreeState& apvts) const;

    void saveUserPreset (const juce::String& name, juce::AudioProcessorValueTreeState& apvts);
    void deleteUserPreset (int index);
    void refreshUserPresets();
    bool isFactoryPreset (int index) const { return index < static_cast<int> (factoryCount); }
    int getFactoryPresetCount() const { return static_cast<int> (factoryCount); }

private:
    void buildPresets();

    /// Helper to create a preset with common defaults, then override specific values
    Preset makePreset (const juce::String& name,
                       std::initializer_list<std::pair<juce::String, float>> overrides) const;

    std::vector<Preset> presets;
    std::vector<std::pair<juce::String, float>> defaults;

    size_t factoryCount = 0;
    juce::File getUserPresetDirectory() const;
    void loadUserPresets();
    void savePresetToFile (const Preset& preset, const juce::File& file) const;
    std::optional<Preset> loadPresetFromFile (const juce::File& file) const;
};

} // namespace cart
