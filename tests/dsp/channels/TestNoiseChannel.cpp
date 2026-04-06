#include <catch2/catch_test_macros.hpp>
#include "dsp/channels/NoiseChannel.h"
#include "dsp/ApuConstants.h"
#include "TestHelpers.h"

TEST_CASE("NoiseChannel", "[dsp][channels]")
{
    cart::NoiseChannel noise;
    noise.setSampleRate(44100.0);
    noise.setEnabled(true);

    SECTION("no output before noteOn")
    {
        noise.setPeriodIndex(0, true);
        auto samples = test::processN(noise, 100);
        REQUIRE_FALSE(test::hasEnergy(samples));
    }

    SECTION("produces output after noteOn")
    {
        noise.envelope().setConstantVolume(true);
        noise.envelope().setVolume(15);
        noise.lengthCounter().setEnabled(true);
        noise.setPeriodIndex(0, true); // fastest period
        noise.noteOn();
        auto samples = test::processN(noise, 1000);
        REQUIRE(test::hasEnergy(samples));
    }

    SECTION("noteOn resets LFSR to 1")
    {
        noise.envelope().setConstantVolume(true);
        noise.envelope().setVolume(15);
        noise.lengthCounter().setEnabled(true);
        noise.setPeriodIndex(0, true);
        noise.noteOn();
        // LFSR starts at 1. Bit 0 = 1, so output should be 0 (inverted logic)
        REQUIRE(noise.output() == 0);
    }

    SECTION("long mode and short mode produce different output")
    {
        noise.envelope().setConstantVolume(true);
        noise.envelope().setVolume(15);
        noise.lengthCounter().setEnabled(true);
        noise.setPeriodIndex(4, true); // moderate speed

        noise.setShortMode(false);
        noise.noteOn();
        auto longSamples = test::processN(noise, 2000);

        noise.reset();
        noise.setSampleRate(44100.0);
        noise.setEnabled(true);
        noise.envelope().setConstantVolume(true);
        noise.envelope().setVolume(15);
        noise.lengthCounter().setEnabled(true);
        noise.setPeriodIndex(4, true);
        noise.setShortMode(true);
        noise.noteOn();
        auto shortSamples = test::processN(noise, 2000);

        // Both modes should produce output (unipolar [0,1])
        REQUIRE(test::hasEnergy(longSamples));
        REQUIRE(test::hasEnergy(shortSamples));

        // Short mode (93-step LFSR) and long mode (32767-step LFSR) should
        // produce different sample patterns
        bool differ = false;
        for (size_t i = 0; i < longSamples.size() && i < shortSamples.size(); ++i)
        {
            if (std::abs(longSamples[i] - shortSamples[i]) > 1e-6f)
            {
                differ = true;
                break;
            }
        }
        REQUIRE(differ);
    }

    SECTION("PAL period table differs from NTSC")
    {
        // NTSC and PAL period tables have different values
        REQUIRE(cart::NOISE_PERIOD_NTSC[0] == 4);
        REQUIRE(cart::NOISE_PERIOD_PAL[0] == 4);
        // But they diverge for higher indices
        REQUIRE(cart::NOISE_PERIOD_NTSC[15] != cart::NOISE_PERIOD_PAL[15]);
    }

    SECTION("noteOff silences channel")
    {
        noise.envelope().setConstantVolume(true);
        noise.envelope().setVolume(15);
        noise.lengthCounter().setEnabled(true);
        noise.setPeriodIndex(0, true);
        noise.noteOn();
        test::processN(noise, 100);
        noise.noteOff();
        auto samples = test::processN(noise, 100);
        REQUIRE_FALSE(test::hasEnergy(samples));
    }

    SECTION("quarterFrame clocks envelope")
    {
        noise.envelope().setConstantVolume(false);
        noise.envelope().setVolume(0);
        noise.envelope().start();
        noise.clockQuarterFrame();
        REQUIRE(noise.envelope().output() == 15);
    }
}
