#include <catch2/catch_test_macros.hpp>
#include "import/VgmParser.h"
#include <vector>
#include <cstring>

using namespace cart;

// ─── Helper to build a minimal VGM buffer ───────────────────────────────────

static std::vector<uint8_t> makeMinimalVgm()
{
    std::vector<uint8_t> data (0xC0, 0); // minimum header size for v1.61

    // "Vgm " magic
    data[0] = 'V'; data[1] = 'g'; data[2] = 'm'; data[3] = ' ';

    // Version 1.61
    data[0x08] = 0x61; data[0x09] = 0x01;

    // Data offset relative to 0x34 = 0x8C (so data starts at 0xC0)
    data[0x34] = 0x8C;

    // NES APU clock = 1789773 (NTSC)
    uint32_t clock = 1789773;
    std::memcpy (&data[0x84], &clock, 4);

    return data;
}

static void appendByte (std::vector<uint8_t>& buf, uint8_t b)
{
    buf.push_back (b);
}

static void appendNesWrite (std::vector<uint8_t>& buf, uint8_t reg, uint8_t val)
{
    buf.push_back (0xB4);
    buf.push_back (reg);
    buf.push_back (val);
}

static void appendWaitNtsc (std::vector<uint8_t>& buf)
{
    buf.push_back (0x62);  // wait 735 samples
}

static void appendEnd (std::vector<uint8_t>& buf)
{
    buf.push_back (0x66);
}

// ─── Tests ──────────────────────────────────────────────────────────────────

TEST_CASE ("VgmParser: parse minimal valid VGM", "[VgmParser]")
{
    auto vgm = makeMinimalVgm();
    appendNesWrite (vgm, 0x00, 0xBF);  // Pulse1 reg0 = duty 50%, vol 15
    appendWaitNtsc (vgm);
    appendEnd (vgm);

    VgmParser parser;
    REQUIRE (parser.parseData (vgm.data(), vgm.size()));
}

TEST_CASE ("VgmParser: header fields are correct", "[VgmParser]")
{
    auto vgm = makeMinimalVgm();
    appendEnd (vgm);

    VgmParser parser;
    REQUIRE (parser.parseData (vgm.data(), vgm.size()));

    auto& hdr = parser.getHeader();
    CHECK (hdr.version == 0x161);
    CHECK (hdr.nesApuClock == 1789773u);
    CHECK (hdr.dataOffset == 0xC0u);
}

TEST_CASE ("VgmParser: NES APU write commands are extracted", "[VgmParser]")
{
    auto vgm = makeMinimalVgm();
    appendNesWrite (vgm, 0x00, 0xBF);
    appendNesWrite (vgm, 0x02, 0xFD);
    appendNesWrite (vgm, 0x03, 0x00);
    appendEnd (vgm);

    VgmParser parser;
    REQUIRE (parser.parseData (vgm.data(), vgm.size()));

    auto& cmds = parser.getCommands();

    // Should have 3 NES writes + 1 EndOfStream
    int nesCount = 0;
    for (auto& c : cmds)
        if (c.type == VgmCommand::NesApuWrite)
            ++nesCount;

    CHECK (nesCount == 3);

    // Verify first command
    CHECK (cmds[0].type == VgmCommand::NesApuWrite);
    CHECK (cmds[0].reg == 0x00);
    CHECK (cmds[0].data == 0xBF);

    // Verify second command
    CHECK (cmds[1].reg == 0x02);
    CHECK (cmds[1].data == 0xFD);
}

TEST_CASE ("VgmParser: wait commands accumulate sample time", "[VgmParser]")
{
    auto vgm = makeMinimalVgm();

    appendNesWrite (vgm, 0x00, 0xBF);  // sampleTime = 0
    appendWaitNtsc (vgm);               // wait 735
    appendNesWrite (vgm, 0x00, 0xB0);  // sampleTime = 735
    appendWaitNtsc (vgm);               // wait 735
    appendNesWrite (vgm, 0x00, 0x00);  // sampleTime = 1470
    appendEnd (vgm);

    VgmParser parser;
    REQUIRE (parser.parseData (vgm.data(), vgm.size()));

    auto& cmds = parser.getCommands();
    int nesIdx = 0;
    for (auto& c : cmds)
    {
        if (c.type == VgmCommand::NesApuWrite)
        {
            if (nesIdx == 0) CHECK (c.sampleTime == 0);
            if (nesIdx == 1) CHECK (c.sampleTime == 735);
            if (nesIdx == 2) CHECK (c.sampleTime == 1470);
            ++nesIdx;
        }
    }
    CHECK (nesIdx == 3);
}

TEST_CASE ("VgmParser: hasNesApu returns true for NES data", "[VgmParser]")
{
    auto vgm = makeMinimalVgm();
    appendNesWrite (vgm, 0x00, 0xBF);
    appendEnd (vgm);

    VgmParser parser;
    REQUIRE (parser.parseData (vgm.data(), vgm.size()));
    CHECK (parser.hasNesApu());
}

TEST_CASE ("VgmParser: too-small buffer returns false", "[VgmParser]")
{
    std::vector<uint8_t> tiny (32, 0);
    VgmParser parser;
    CHECK_FALSE (parser.parseData (tiny.data(), tiny.size()));
}

TEST_CASE ("VgmParser: non-VGM data returns false", "[VgmParser]")
{
    std::vector<uint8_t> garbage (128, 0x42);
    VgmParser parser;
    CHECK_FALSE (parser.parseData (garbage.data(), garbage.size()));
}

TEST_CASE ("VgmParser: wait N samples command (0x61)", "[VgmParser]")
{
    auto vgm = makeMinimalVgm();

    appendNesWrite (vgm, 0x00, 0xBF);  // sampleTime = 0
    // 0x61 wait N: little-endian 16-bit
    vgm.push_back (0x61);
    vgm.push_back (0xE8); vgm.push_back (0x03);  // 1000 samples
    appendNesWrite (vgm, 0x00, 0xB0);  // sampleTime = 1000
    appendEnd (vgm);

    VgmParser parser;
    REQUIRE (parser.parseData (vgm.data(), vgm.size()));

    auto& cmds = parser.getCommands();
    // Second NES write should be at sample time 1000
    int nesIdx = 0;
    for (auto& c : cmds)
    {
        if (c.type == VgmCommand::NesApuWrite)
        {
            if (nesIdx == 1)
                CHECK (c.sampleTime == 1000);
            ++nesIdx;
        }
    }
}

TEST_CASE ("VgmParser: short wait commands 0x70-0x7F", "[VgmParser]")
{
    auto vgm = makeMinimalVgm();

    appendNesWrite (vgm, 0x00, 0xBF);  // time = 0
    vgm.push_back (0x73);              // wait 4 samples (0x03 + 1)
    appendNesWrite (vgm, 0x00, 0xB0);  // time = 4
    appendEnd (vgm);

    VgmParser parser;
    REQUIRE (parser.parseData (vgm.data(), vgm.size()));

    auto& cmds = parser.getCommands();
    int nesIdx = 0;
    for (auto& c : cmds)
    {
        if (c.type == VgmCommand::NesApuWrite && nesIdx++ == 1)
            CHECK (c.sampleTime == 4);
    }
}

TEST_CASE ("VgmParser: end of stream command present", "[VgmParser]")
{
    auto vgm = makeMinimalVgm();
    appendEnd (vgm);

    VgmParser parser;
    REQUIRE (parser.parseData (vgm.data(), vgm.size()));

    auto& cmds = parser.getCommands();
    REQUIRE_FALSE (cmds.empty());
    CHECK (cmds.back().type == VgmCommand::EndOfStream);
}
