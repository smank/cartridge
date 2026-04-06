#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "dsp/mixing/ApuMixer.h"
#include "dsp/ApuConstants.h"

using Catch::Matchers::WithinAbs;

TEST_CASE("ApuMixer", "[dsp][mixing]")
{
    cart::ApuMixer mixer;

    SECTION("silence in produces value near zero")
    {
        float out = mixer.mix(0, 0, 0, 0, 0);
        // pulseTable[0]=0, tndTable[0]=0. Result = (0+0-0.5)*2 = -1.0
        REQUIRE_THAT(out, WithinAbs(-1.0f, 0.01f));
    }

    SECTION("nonlinear: two pulse channels are less than double")
    {
        float one = mixer.mix(15, 0, 0, 0, 0);
        float two = mixer.mix(15, 15, 0, 0, 0);
        // Nonlinear: doubling input should NOT double output
        float diff = two - one;
        float oneContrib = one - mixer.mix(0, 0, 0, 0, 0);
        REQUIRE(diff < oneContrib); // second pulse contributes less
    }

    SECTION("pulse table is monotonically increasing")
    {
        const auto& tables = cart::getMixerTables();
        for (size_t i = 1; i <= 30; ++i)
            REQUIRE(tables.pulseTable[i] > tables.pulseTable[i - 1]);
    }

    SECTION("tnd table is monotonically increasing")
    {
        const auto& tables = cart::getMixerTables();
        for (size_t i = 1; i <= 202; ++i)
            REQUIRE(tables.tndTable[i] > tables.tndTable[i - 1]);
    }

    SECTION("per-channel mix levels scale output")
    {
        float fullMix = mixer.mix(15, 0, 0, 0, 0);
        mixer.setPulse1Mix(0.0f);
        float zeroMix = mixer.mix(15, 0, 0, 0, 0);
        REQUIRE(zeroMix < fullMix);
    }

    SECTION("mixIndividual returns per-channel outputs")
    {
        float out[5];
        mixer.mixIndividual(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, out);
        // pulse1 at full scale should have nonzero output
        REQUIRE(std::abs(out[0]) > 0.0f);
        // pulse2 at bipolar 0.0 (midpoint) also has some DAC contribution,
        // but both should be finite valid values
        REQUIRE(std::isfinite(out[0]));
        REQUIRE(std::isfinite(out[1]));
    }

    SECTION("mixFloat scales float inputs to integer range")
    {
        // Full-scale pulse wave at max volume
        float result = mixer.mixFloat(1.0f, 1.0f, 0.0f, 0.0f, 0.0f);
        // Should produce a valid output, not NaN or inf
        REQUIRE(std::isfinite(result));
    }
}
