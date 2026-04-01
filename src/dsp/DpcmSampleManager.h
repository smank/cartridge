#pragma once
#include <juce_core/juce_core.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <vector>
#include <array>
#include <cstdint>

namespace cart {

class DpcmSampleManager
{
public:
    static constexpr int kMaxUserSamples = 16;
    static constexpr int kFactorySamples = 4;

    DpcmSampleManager();

    /// Load a .dmc or .wav file. Returns user slot index (4-19) or -1 on failure.
    int loadFromFile(const juce::File& file);

    /// Get sample data for index 0-3 (factory) or 4-19 (user)
    std::vector<uint8_t> getSample(int index) const;

    /// Get display name for a sample
    juce::String getSampleName(int index) const;

    /// Total number of available samples (factory + loaded user)
    int getSampleCount() const { return kFactorySamples + userSampleCount; }

    /// Serialize user samples to XML (base64 encoded)
    void saveToXml(juce::XmlElement& xml) const;

    /// Restore user samples from XML
    void loadFromXml(const juce::XmlElement& xml);

    /// Convert WAV audio data to DPCM format
    static std::vector<uint8_t> wavToDpcm(const float* samples, int numSamples);

private:
    struct UserSample
    {
        juce::String name;
        std::vector<uint8_t> data;
    };

    std::array<UserSample, kMaxUserSamples> userSamples;
    int userSampleCount = 0;

    juce::File getUserSamplesDir() const;
};

} // namespace cart
