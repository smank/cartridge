#include "VgmParser.h"

namespace cart {

uint32_t VgmParser::readU32 (const uint8_t* p)
{
    return static_cast<uint32_t> (p[0])
         | (static_cast<uint32_t> (p[1]) << 8)
         | (static_cast<uint32_t> (p[2]) << 16)
         | (static_cast<uint32_t> (p[3]) << 24);
}

uint16_t VgmParser::readU16 (const uint8_t* p)
{
    return static_cast<uint16_t> (p[0] | (p[1] << 8));
}

bool VgmParser::parse (const juce::File& file)
{
    juce::MemoryBlock raw;
    if (!file.loadFileAsData (raw) || raw.getSize() < 64)
        return false;

    auto* data = static_cast<const uint8_t*> (raw.getData());
    auto size = raw.getSize();

    // Check for gzip magic (VGZ)
    if (data[0] == 0x1F && data[1] == 0x8B)
    {
        juce::MemoryInputStream compressedStream (raw, false);
        juce::GZIPDecompressorInputStream decompressor (
            &compressedStream, false,
            juce::GZIPDecompressorInputStream::gzipFormat);

        juce::MemoryBlock decompressed;
        decompressor.readIntoMemoryBlock (decompressed);

        if (decompressed.getSize() < 64)
        {
            // Try deflateFormat as fallback
            juce::MemoryInputStream compressedStream2 (raw, false);
            juce::GZIPDecompressorInputStream decompressor2 (
                &compressedStream2, false,
                juce::GZIPDecompressorInputStream::deflateFormat);

            decompressed.reset();
            decompressor2.readIntoMemoryBlock (decompressed);
        }

        if (decompressed.getSize() < 64)
            return false;

        return parseData (static_cast<const uint8_t*> (decompressed.getData()),
                          decompressed.getSize());
    }

    return parseData (data, size);
}

bool VgmParser::parseData (const uint8_t* data, size_t size)
{
    if (size < 64)
        return false;

    // Check "Vgm " magic
    if (data[0] != 'V' || data[1] != 'g' || data[2] != 'm' || data[3] != ' ')
        return false;

    if (!parseHeader (data, size))
        return false;

    if (header.gd3Offset > 0)
        parseGd3 (data, size);

    parseCommands (data, size);
    return true;
}

bool VgmParser::parseHeader (const uint8_t* data, size_t size)
{
    header = {};
    header.version      = readU32 (data + 0x08);
    header.totalSamples = readU32 (data + 0x18);
    header.loopSamples  = readU32 (data + 0x20);

    // GD3 offset is relative to position 0x14
    uint32_t gd3Rel = readU32 (data + 0x14);
    header.gd3Offset = (gd3Rel != 0) ? (0x14 + gd3Rel) : 0;

    // Loop offset is relative to position 0x1C
    uint32_t loopRel = readU32 (data + 0x1C);
    header.loopOffset = (loopRel != 0) ? (0x1C + loopRel) : 0;

    // Data offset (relative to 0x34, version >= 1.50 uses this field)
    if (header.version >= 0x150 && size > 0x38)
    {
        uint32_t dataRel = readU32 (data + 0x34);
        header.dataOffset = (dataRel != 0) ? (0x34 + dataRel) : 0x40;
    }
    else
    {
        header.dataOffset = 0x40;
    }

    // NES APU clock at offset 0x84 (version >= 1.61)
    if (size > 0x88 && header.version >= 0x161)
        header.nesApuClock = readU32 (data + 0x84);

    // Also check offset 0x84 unconditionally for files with incorrect version fields
    if (header.nesApuClock == 0 && size > 0x88)
    {
        uint32_t maybeClock = readU32 (data + 0x84);
        // NES NTSC = 1789773, PAL = 1662607 — check if plausible
        if (maybeClock >= 1000000 && maybeClock <= 2000000)
            header.nesApuClock = maybeClock;
    }

    return true;
}

void VgmParser::parseGd3 (const uint8_t* data, size_t size)
{
    gd3 = {};
    if (header.gd3Offset == 0 || header.gd3Offset + 12 > size)
        return;

    const uint8_t* gd3Data = data + header.gd3Offset;

    // Check "Gd3 " magic
    if (gd3Data[0] != 'G' || gd3Data[1] != 'd' || gd3Data[2] != '3' || gd3Data[3] != ' ')
        return;

    uint32_t gd3Size = readU32 (gd3Data + 8);
    const uint8_t* strData = gd3Data + 12;
    size_t strEnd = header.gd3Offset + 12 + gd3Size;
    if (strEnd > size) strEnd = size;

    // GD3 strings are null-terminated UTF-16 LE in order:
    // Track EN, Track JP, Game EN, Game JP, System EN, System JP,
    // Author EN, Author JP, Date, Converter, Notes
    auto readUtf16 = [&] () -> juce::String
    {
        juce::String result;
        while (strData + 1 < data + strEnd)
        {
            uint16_t ch = readU16 (strData);
            strData += 2;
            if (ch == 0) break;
            result += juce::String::charToString (static_cast<juce::juce_wchar> (ch));
        }
        return result;
    };

    gd3.trackName  = readUtf16();
    readUtf16(); // Track JP
    gd3.gameName   = readUtf16();
    readUtf16(); // Game JP
    gd3.systemName = readUtf16();
    readUtf16(); // System JP
    gd3.author     = readUtf16();
}

void VgmParser::parseCommands (const uint8_t* data, size_t size)
{
    commands.clear();
    commands.reserve (1024);
    hasNesCommands = false;

    size_t pos = header.dataOffset;
    uint64_t sampleTime = 0;

    while (pos < size)
    {
        uint8_t cmd = data[pos];

        if (cmd == 0x66) // End of stream
        {
            commands.push_back ({ VgmCommand::EndOfStream, 0, 0, sampleTime });
            break;
        }
        else if (cmd == 0xB4) // NES APU write
        {
            if (pos + 2 >= size) break;
            uint8_t reg  = data[pos + 1];
            uint8_t val  = data[pos + 2];
            commands.push_back ({ VgmCommand::NesApuWrite, reg, val, sampleTime });
            hasNesCommands = true;
            pos += 3;
        }
        else if (cmd == 0x61) // Wait N samples
        {
            if (pos + 2 >= size) break;
            uint16_t wait = readU16 (data + pos + 1);
            sampleTime += wait;
            pos += 3;
        }
        else if (cmd == 0x62) // Wait 735 (NTSC frame)
        {
            sampleTime += 735;
            pos += 1;
        }
        else if (cmd == 0x63) // Wait 882 (PAL frame)
        {
            sampleTime += 882;
            pos += 1;
        }
        else if (cmd >= 0x70 && cmd <= 0x7F) // Wait 1-16 samples
        {
            sampleTime += (cmd & 0x0F) + 1;
            pos += 1;
        }
        else if (cmd >= 0x80 && cmd <= 0x8F) // YM2612 DAC + wait (skip)
        {
            sampleTime += (cmd & 0x0F);
            pos += 1;
        }
        // Skip other chip commands by their known sizes
        else if (cmd == 0x67) // data block — skip
        {
            if (pos + 6 >= size) break;
            uint32_t blockSize = readU32 (data + pos + 3);
            pos += 7 + blockSize;
        }
        else if (cmd >= 0x30 && cmd <= 0x3F) { pos += 2; } // 1-byte arg
        else if (cmd >= 0x40 && cmd <= 0x4E) { pos += 3; } // 2-byte arg
        else if (cmd == 0x4F || cmd == 0x50) { pos += 2; } // Game Gear PSG / SN76489
        else if (cmd >= 0x51 && cmd <= 0x5F) { pos += 3; } // Various chip writes
        else if (cmd >= 0xA0 && cmd <= 0xBF) { pos += 3; } // Various chip writes
        else if (cmd >= 0xC0 && cmd <= 0xDF) { pos += 4; } // Various chip writes
        else if (cmd >= 0xE0 && cmd <= 0xFF) { pos += 5; } // Various chip writes
        else
        {
            // Unknown command — stop to avoid misparse
            break;
        }

        // Stop at loop point to avoid extracting looped repetitions
        if (header.loopOffset > 0 && pos >= header.loopOffset && sampleTime > 0)
            break;
    }
}

} // namespace cart
