#include "DpcmSampleManager.h"
#include "DpcmSamples.h"
#include <algorithm>

namespace cart {

DpcmSampleManager::DpcmSampleManager() = default;

juce::File DpcmSampleManager::getUserSamplesDir() const
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
               .getChildFile("Cartridge")
               .getChildFile("Samples");
}

int DpcmSampleManager::loadFromFile(const juce::File& file)
{
    if (userSampleCount >= kMaxUserSamples)
        return -1;

    std::vector<uint8_t> data;

    if (file.getFileExtension().equalsIgnoreCase(".dmc"))
    {
        // Raw DPCM data
        juce::MemoryBlock mb;
        if (!file.loadFileAsData(mb))
            return -1;
        data.assign(static_cast<const uint8_t*>(mb.getData()),
                    static_cast<const uint8_t*>(mb.getData()) + mb.getSize());
    }
    else if (file.getFileExtension().equalsIgnoreCase(".wav"))
    {
        // Convert WAV to DPCM
        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();

        std::unique_ptr<juce::AudioFormatReader> reader(
            formatManager.createReaderFor(file));

        if (reader == nullptr)
            return -1;

        // Read all samples as mono float
        int numSamples = static_cast<int>(reader->lengthInSamples);
        juce::AudioBuffer<float> buffer(1, numSamples);
        reader->read(&buffer, 0, numSamples, 0, true, false);

        data = wavToDpcm(buffer.getReadPointer(0), numSamples);
    }
    else
    {
        return -1;
    }

    if (data.empty())
        return -1;

    int slot = userSampleCount;
    userSamples[slot].name = file.getFileNameWithoutExtension();
    userSamples[slot].data = std::move(data);
    ++userSampleCount;

    return kFactorySamples + slot;
}

std::vector<uint8_t> DpcmSampleManager::wavToDpcm(const float* samples, int numSamples)
{
    // NES DPCM: 1-bit delta, LSB first, 8 bits per byte
    // Level range: 0-127 (7-bit), starts at 64
    // bit=1: level += 2, bit=0: level -= 2

    int level = 64;
    std::vector<uint8_t> result;
    result.reserve(static_cast<size_t>((numSamples + 7) / 8));

    uint8_t currentByte = 0;
    int bitIndex = 0;

    for (int i = 0; i < numSamples; ++i)
    {
        // Map float [-1, 1] to [0, 127]
        int target = static_cast<int>((samples[i] + 1.0f) * 63.5f);
        target = std::clamp(target, 0, 127);

        if (target > level)
        {
            currentByte |= static_cast<uint8_t>(1 << bitIndex);
            level = std::min(127, level + 2);
        }
        else
        {
            level = std::max(0, level - 2);
        }

        ++bitIndex;
        if (bitIndex == 8)
        {
            result.push_back(currentByte);
            currentByte = 0;
            bitIndex = 0;
        }
    }

    // Flush remaining bits
    if (bitIndex > 0)
        result.push_back(currentByte);

    return result;
}

std::vector<uint8_t> DpcmSampleManager::getSample(int index) const
{
    if (index < kFactorySamples)
        return getDpcmSample(index);

    int userIdx = index - kFactorySamples;
    if (userIdx >= 0 && userIdx < userSampleCount)
        return userSamples[userIdx].data;

    return getDpcmSample(0); // Fallback to kick
}

juce::String DpcmSampleManager::getSampleName(int index) const
{
    if (index < kFactorySamples)
    {
        static const char* names[] = { "Kick", "Snare", "Hi-Hat", "Tom" };
        return names[index];
    }

    int userIdx = index - kFactorySamples;
    if (userIdx >= 0 && userIdx < userSampleCount)
        return userSamples[userIdx].name;

    return "---";
}

void DpcmSampleManager::saveToXml(juce::XmlElement& xml) const
{
    for (int i = 0; i < userSampleCount; ++i)
    {
        auto* sampleEl = xml.createNewChildElement("UserDpcmSample");
        sampleEl->setAttribute("name", userSamples[i].name);
        sampleEl->setAttribute("data", juce::Base64::toBase64(
            userSamples[i].data.data(), userSamples[i].data.size()));
    }
}

void DpcmSampleManager::loadFromXml(const juce::XmlElement& xml)
{
    userSampleCount = 0;

    for (auto* child : xml.getChildIterator())
    {
        if (child->getTagName() != "UserDpcmSample")
            continue;
        if (userSampleCount >= kMaxUserSamples)
            break;

        auto name = child->getStringAttribute("name");
        auto b64 = child->getStringAttribute("data");

        juce::MemoryOutputStream decoded;
        if (juce::Base64::convertFromBase64(decoded, b64))
        {
            auto* rawData = static_cast<const uint8_t*>(decoded.getData());
            userSamples[userSampleCount].name = name;
            userSamples[userSampleCount].data.assign(rawData, rawData + decoded.getDataSize());
            ++userSampleCount;
        }
    }
}

} // namespace cart
