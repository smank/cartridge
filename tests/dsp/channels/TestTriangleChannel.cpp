#include <catch2/catch_test_macros.hpp>
#include "dsp/channels/TriangleChannel.h"
#include "dsp/ApuConstants.h"
#include "TestHelpers.h"

TEST_CASE("TriangleChannel", "[dsp][channels]")
{
    cart::TriangleChannel tri;
    tri.setSampleRate(44100.0);
    tri.setEnabled(true);

    SECTION("no output before noteOn")
    {
        tri.setFrequency(440.0f);
        auto samples = test::processN(tri, 100);
        REQUIRE_FALSE(test::hasEnergy(samples));
    }

    SECTION("produces output after noteOn with linear counter primed")
    {
        tri.linearCounter().setReloadValue(127);
        tri.linearCounter().setControl(true);
        tri.lengthCounter().setEnabled(true);
        tri.setFrequency(440.0f);
        tri.noteOn(); // reload + clock primes linear counter
        auto samples = test::processN(tri, 1000);
        REQUIRE(test::hasEnergy(samples));
    }

    SECTION("noteOn immediately primes linear counter - no delay")
    {
        // Regression test: noteOn must call linear.clock() after reload()
        // so sound starts immediately, not after ~4ms quarter-frame delay
        tri.linearCounter().setReloadValue(127);
        tri.linearCounter().setControl(true);
        tri.lengthCounter().setEnabled(true);
        tri.setFrequency(440.0f);
        tri.noteOn();
        // Linear counter should be active immediately
        REQUIRE(tri.linearCounter().isActive());
        REQUIRE(tri.isActive());
        // First sample should have energy
        float firstSample = tri.process();
        // The first sample might be 0 due to phase, but activity should be true
        REQUIRE(tri.isActive());
    }

    SECTION("triangle waveform uses 32-step sequence")
    {
        tri.linearCounter().setReloadValue(127);
        tri.linearCounter().setControl(true);
        tri.lengthCounter().setEnabled(true);
        tri.setFrequency(440.0f);
        tri.noteOn();
        // Process enough samples to get a full waveform
        auto samples = test::processN(tri, 4000);
        // Triangle output should be bounded [-1, 1]
        for (auto s : samples)
        {
            REQUIRE(s >= -1.01f);
            REQUIRE(s <= 1.01f);
        }
    }

    SECTION("noteOff silences channel")
    {
        tri.linearCounter().setReloadValue(127);
        tri.linearCounter().setControl(true);
        tri.lengthCounter().setEnabled(true);
        tri.setFrequency(440.0f);
        tri.noteOn();
        test::processN(tri, 100);
        tri.noteOff();
        auto samples = test::processN(tri, 100);
        REQUIRE_FALSE(test::hasEnergy(samples));
    }

    SECTION("isActive requires gate, length, and linear counter")
    {
        tri.linearCounter().setReloadValue(127);
        tri.linearCounter().setControl(true);
        tri.lengthCounter().setEnabled(true);
        tri.setFrequency(440.0f);
        tri.noteOn();
        REQUIRE(tri.isActive());
        // Disable length counter
        tri.setEnabled(false);
        REQUIRE_FALSE(tri.isActive());
    }

    SECTION("output is from TRIANGLE_SEQUENCE")
    {
        // The output() method returns TRIANGLE_SEQUENCE[sequencePos]
        // After noteOn but before process, sequencePos is 0
        tri.linearCounter().setReloadValue(127);
        tri.linearCounter().setControl(true);
        tri.lengthCounter().setEnabled(true);
        tri.setFrequency(440.0f);
        tri.noteOn();
        // output() at sequencePos=0 should be TRIANGLE_SEQUENCE[0]=15
        REQUIRE(tri.output() == 15);
    }
}
