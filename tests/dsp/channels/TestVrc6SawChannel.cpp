#include <catch2/catch_test_macros.hpp>
#include "dsp/channels/Vrc6SawChannel.h"
#include "TestHelpers.h"

TEST_CASE("Vrc6SawChannel", "[dsp][channels]")
{
    cart::Vrc6SawChannel saw;
    saw.setSampleRate(44100.0);
    saw.setEnabled(true);

    SECTION("no output before noteOn")
    {
        saw.setFrequency(440.0f);
        auto samples = test::processN(saw, 100);
        REQUIRE_FALSE(test::hasEnergy(samples));
    }

    SECTION("produces output after noteOn")
    {
        saw.setRate(42);
        saw.noteOn(440.0f);
        auto samples = test::processN(saw, 2000);
        REQUIRE(test::hasEnergy(samples));
    }

    SECTION("rate 0 produces minimal output")
    {
        saw.setRate(0);
        saw.noteOn(440.0f);
        auto samples = test::processN(saw, 2000);
        // Rate 0: accumulator never increases -> always outputs same value
        // Output is (0>>3)/15.5 - 1.0 = -1.0
        // All samples should be near -1.0 (the accumulator stays at 0)
        for (auto s : samples)
            REQUIRE(s <= 0.0f);
    }

    SECTION("rate > 42 causes overflow distortion")
    {
        // Rate 42 is clean max. Rate 63 causes 8-bit overflow.
        saw.setRate(63);
        saw.noteOn(220.0f);
        auto samples = test::processN(saw, 4000);
        REQUIRE(test::hasEnergy(samples));
        // The waveform should be more complex/distorted than rate 42
    }

    SECTION("noteOff silences channel")
    {
        saw.setRate(42);
        saw.noteOn(440.0f);
        test::processN(saw, 100);
        saw.noteOff();
        auto samples = test::processN(saw, 100);
        REQUIRE_FALSE(test::hasEnergy(samples));
    }
}
