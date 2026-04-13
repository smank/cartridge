#pragma once

#include <juce_core/juce_core.h>
#include <vector>
#include <cstdint>

namespace cart {

struct VgmHeader
{
    uint32_t version       = 0;
    uint32_t totalSamples  = 0;
    uint32_t loopOffset    = 0;
    uint32_t loopSamples   = 0;
    uint32_t dataOffset    = 0;
    uint32_t nesApuClock   = 0;   // 0 if no NES APU data
    uint32_t gd3Offset     = 0;
};

struct VgmCommand
{
    enum Type { NesApuWrite, Wait, EndOfStream, Unknown };
    Type     type        = Unknown;
    uint8_t  reg         = 0;    // NES APU register offset (0x00-0x17)
    uint8_t  data        = 0;
    uint64_t sampleTime  = 0;    // cumulative sample position
};

struct Gd3Tag
{
    juce::String trackName;
    juce::String gameName;
    juce::String systemName;
    juce::String author;
};

class VgmParser
{
public:
    /// Parse a VGM or VGZ file. Returns true on success.
    bool parse (const juce::File& file);

    /// Parse from raw (decompressed) VGM data.
    bool parseData (const uint8_t* data, size_t size);

    const VgmHeader& getHeader() const { return header; }
    const Gd3Tag& getGd3() const { return gd3; }
    const std::vector<VgmCommand>& getCommands() const { return commands; }

    bool hasNesApu() const { return header.nesApuClock != 0 || hasNesCommands; }

private:
    bool parseHeader (const uint8_t* data, size_t size);
    void parseCommands (const uint8_t* data, size_t size);
    void parseGd3 (const uint8_t* data, size_t size);

    static uint32_t readU32 (const uint8_t* p);
    static uint16_t readU16 (const uint8_t* p);

    VgmHeader header;
    Gd3Tag gd3;
    std::vector<VgmCommand> commands;
    bool hasNesCommands = false;
};

} // namespace cart
