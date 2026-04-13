#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Parameters.h"
#include "dsp/StepSequencer.h"
#include <vector>
#include <optional>
#include <array>

namespace cart {

struct Preset
{
    juce::String name;
    juce::String category;  // Empty for uncategorized user presets
    std::vector<std::pair<juce::String, float>> values;  // paramID → value

    // Optional step sequence data (populated from VGM imports or presets with embedded seqs)
    bool hasStepSeqData = false;
    std::array<StepSequenceData, 8> stepSeqData;
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

    /// Export current state as a single .cartpreset file
    void exportPreset (const juce::String& name,
                       juce::AudioProcessorValueTreeState& apvts,
                       const juce::File& destFile);

    /// Export a bank of presets as a .cartbank file
    void exportBank (const juce::String& bankName,
                     const std::vector<int>& presetIndices,
                     const juce::File& destFile) const;

    /// Import a .cartpreset file — returns the index of the newly added preset, or -1
    int importPreset (const juce::File& file);

    /// Import a .cartbank file — returns number of presets imported
    int importBank (const juce::File& file);

    /// Get ordered list of unique user preset categories (for PresetComboBox sections)
    std::vector<juce::String> getUserCategories() const;

    /// Get the Preset struct at index (for category info)
    const Preset* getPreset (int index) const;

    /// Returns 0 for Classic, 1 for Modern engine mode preset
    int getPresetEngineMode (int index) const;

    /// Returns true if the preset at index has embedded step sequence data
    bool presetHasStepSeqData (int index) const;

    /// Copy step sequence data from preset into dest array. Returns true if data was present.
    bool getPresetStepSeqData (int index, std::array<StepSequenceData, 8>& dest) const;

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
