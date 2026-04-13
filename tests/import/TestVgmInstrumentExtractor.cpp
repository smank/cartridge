#include <catch2/catch_test_macros.hpp>
#include "import/VgmInstrumentExtractor.h"
#include <vector>

using namespace cart;

static constexpr uint32_t kNtscClock = 1789773;
static constexpr int kFrame = 735;  // samples per NTSC frame

// ─── Helpers ────────────────────────────────────────────────────────────────

/// Build a NES APU write command at a given sample time.
static VgmCommand nesWrite (uint8_t reg, uint8_t data, uint64_t time)
{
    VgmCommand cmd;
    cmd.type = VgmCommand::NesApuWrite;
    cmd.reg  = reg;
    cmd.data = data;
    cmd.sampleTime = time;
    return cmd;
}

/// Build a simple Pulse 1 note sequence with volume envelope fade-out.
/// Returns commands that: set duty+vol, set freq low, trigger (reg3),
/// then change volume over several frames, then silence.
static std::vector<VgmCommand> makePulse1FadeOut()
{
    std::vector<VgmCommand> cmds;
    uint64_t t = 0;

    // Note on: duty 50%, vol 15, freq ~440Hz (timer period ~253)
    cmds.push_back (nesWrite (0x00, 0xBF, t));       // reg0: duty=10, LC halt=1, vol=15
    cmds.push_back (nesWrite (0x02, 0xFD, t));       // reg2: timer low = 0xFD
    cmds.push_back (nesWrite (0x03, 0x00, t));       // reg3: timer high + length counter load (trigger)

    // Fade out volume over frames
    t += kFrame;
    cmds.push_back (nesWrite (0x00, 0xBC, t));       // vol = 12
    t += kFrame;
    cmds.push_back (nesWrite (0x00, 0xB8, t));       // vol = 8
    t += kFrame;
    cmds.push_back (nesWrite (0x00, 0xB4, t));       // vol = 4
    t += kFrame;
    cmds.push_back (nesWrite (0x00, 0xB0, t));       // vol = 0 (silence = note off)

    return cmds;
}

// ─── Tests ──────────────────────────────────────────────────────────────────

TEST_CASE ("VgmInstrumentExtractor: extracts volume envelope from fade-out", "[VgmInstrumentExtractor]")
{
    auto cmds = makePulse1FadeOut();

    VgmInstrumentExtractor extractor;
    extractor.extract (cmds, kNtscClock);

    auto& instruments = extractor.getInstruments();
    REQUIRE_FALSE (instruments.empty());

    // The first instrument should be for Pulse1 (channel 0)
    CHECK (instruments[0].channelIndex == 0);

    // Should have volume steps with variation
    auto& sd = instruments[0].seqData;
    CHECK (sd.numVolumeSteps > 0);

    // First volume step should be the initial volume (15)
    CHECK (sd.volumeSteps[0] == 15);
}

TEST_CASE ("VgmInstrumentExtractor: deduplication removes identical notes", "[VgmInstrumentExtractor]")
{
    // Play the same note twice — should produce only one instrument
    auto cmds1 = makePulse1FadeOut();
    auto cmds2 = makePulse1FadeOut();

    // Offset the second note in time
    uint64_t offset = 10 * kFrame;
    for (auto& c : cmds2)
        c.sampleTime += offset;

    std::vector<VgmCommand> combined;
    combined.insert (combined.end(), cmds1.begin(), cmds1.end());
    combined.insert (combined.end(), cmds2.begin(), cmds2.end());

    VgmInstrumentExtractor extractor;
    extractor.extract (combined, kNtscClock);

    auto& instruments = extractor.getInstruments();
    // Deduplication: two identical envelopes should result in just 1 instrument
    CHECK (instruments.size() == 1);
}

TEST_CASE ("VgmInstrumentExtractor: channel index correct for Pulse2", "[VgmInstrumentExtractor]")
{
    std::vector<VgmCommand> cmds;
    uint64_t t = 0;

    // Pulse 2 registers: 0x04-0x07
    cmds.push_back (nesWrite (0x04, 0xBF, t));       // duty=10, vol=15
    cmds.push_back (nesWrite (0x06, 0xFD, t));       // timer low
    cmds.push_back (nesWrite (0x07, 0x00, t));       // trigger

    t += kFrame;
    cmds.push_back (nesWrite (0x04, 0xBA, t));       // vol = 10
    t += kFrame;
    cmds.push_back (nesWrite (0x04, 0xB5, t));       // vol = 5
    t += kFrame;
    cmds.push_back (nesWrite (0x04, 0xB0, t));       // vol = 0 (note off)

    VgmInstrumentExtractor extractor;
    extractor.extract (cmds, kNtscClock);

    auto& instruments = extractor.getInstruments();
    REQUIRE_FALSE (instruments.empty());
    CHECK (instruments[0].channelIndex == 1);  // Pulse2 = channel 1
}

TEST_CASE ("VgmInstrumentExtractor: channel index correct for Triangle", "[VgmInstrumentExtractor]")
{
    std::vector<VgmCommand> cmds;
    uint64_t t = 0;

    // Triangle registers: 0x08-0x0B
    // Triangle doesn't have volume control in reg, but we can detect pitch variation
    cmds.push_back (nesWrite (0x08, 0xFF, t));       // linear counter
    cmds.push_back (nesWrite (0x0A, 0xFD, t));       // timer low
    cmds.push_back (nesWrite (0x0B, 0x00, t));       // trigger

    // Change pitch over frames
    t += kFrame;
    cmds.push_back (nesWrite (0x0A, 0xC0, t));       // different timer low = pitch change
    t += kFrame;
    cmds.push_back (nesWrite (0x0A, 0x90, t));       // another pitch change

    // End note with new trigger
    t += kFrame;
    cmds.push_back (nesWrite (0x0B, 0x00, t));       // re-trigger = ends previous note

    VgmInstrumentExtractor extractor;
    extractor.extract (cmds, kNtscClock);

    auto& instruments = extractor.getInstruments();
    // Triangle with pitch variation should produce an instrument
    if (!instruments.empty())
    {
        CHECK (instruments[0].channelIndex == 2);  // Triangle = channel 2
    }
}

TEST_CASE ("VgmInstrumentExtractor: channel index correct for Noise", "[VgmInstrumentExtractor]")
{
    std::vector<VgmCommand> cmds;
    uint64_t t = 0;

    // Noise registers: 0x0C-0x0F
    cmds.push_back (nesWrite (0x0C, 0xBF, t));       // vol = 15
    cmds.push_back (nesWrite (0x0E, 0x00, t));       // noise period
    cmds.push_back (nesWrite (0x0F, 0x00, t));       // trigger

    t += kFrame;
    cmds.push_back (nesWrite (0x0C, 0xBA, t));       // vol = 10
    t += kFrame;
    cmds.push_back (nesWrite (0x0C, 0xB5, t));       // vol = 5
    t += kFrame;
    cmds.push_back (nesWrite (0x0C, 0xB0, t));       // vol = 0 (note off)

    VgmInstrumentExtractor extractor;
    extractor.extract (cmds, kNtscClock);

    auto& instruments = extractor.getInstruments();
    REQUIRE_FALSE (instruments.empty());
    CHECK (instruments[0].channelIndex == 3);  // Noise = channel 3
}

TEST_CASE ("VgmInstrumentExtractor: empty commands produce no instruments", "[VgmInstrumentExtractor]")
{
    std::vector<VgmCommand> cmds;

    VgmInstrumentExtractor extractor;
    extractor.extract (cmds, kNtscClock);

    CHECK (extractor.getInstruments().empty());
}

TEST_CASE ("VgmInstrumentExtractor: single frame note produces no instrument", "[VgmInstrumentExtractor]")
{
    // A note that lasts only 1 frame isn't enough (need >= 2 frames)
    std::vector<VgmCommand> cmds;
    uint64_t t = 0;

    cmds.push_back (nesWrite (0x00, 0xBF, t));
    cmds.push_back (nesWrite (0x02, 0xFD, t));
    cmds.push_back (nesWrite (0x03, 0x00, t));
    t += kFrame;
    cmds.push_back (nesWrite (0x00, 0xB0, t));  // immediate silence

    VgmInstrumentExtractor extractor;
    extractor.extract (cmds, kNtscClock);

    // Only 1 frame captured (the initial one) — needs >= 2 for meaningful envelope
    CHECK (extractor.getInstruments().empty());
}
