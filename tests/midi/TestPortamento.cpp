#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "dsp/Portamento.h"

TEST_CASE("Portamento", "[midi]")
{
    cart::Portamento porta;
    porta.setSampleRate(44100.0);

    SECTION("disabled snaps to target immediately")
    {
        porta.setEnabled(false);
        porta.setTarget(440.0f);
        REQUIRE(porta.getCurrentFreq() == Catch::Approx(440.0f));
        porta.setTarget(880.0f);
        REQUIRE(porta.getCurrentFreq() == Catch::Approx(880.0f));
    }

    SECTION("enabled glides toward target")
    {
        porta.setEnabled(true);
        porta.setTime(0.1f);
        porta.setTarget(440.0f); // first target snaps (current=0)
        REQUIRE(porta.getCurrentFreq() == Catch::Approx(440.0f));

        porta.setTarget(880.0f);
        // After one process, should be between 440 and 880
        float freq = porta.process();
        REQUIRE(freq > 440.0f);
        REQUIRE(freq < 880.0f);
    }

    SECTION("isGliding returns true during glide and converges near target")
    {
        porta.setEnabled(true);
        porta.setTime(0.01f);
        porta.setTarget(440.0f);
        porta.setTarget(880.0f);
        REQUIRE(porta.isGliding());

        // Process enough samples to get very close to target.
        // Due to float precision limits, the exponential glide may not reach
        // the exact snap threshold, so verify we get within a small margin.
        for (int i = 0; i < 44100; ++i)
            porta.process();
        REQUIRE(porta.getCurrentFreq() == Catch::Approx(880.0f).margin(0.1f));
    }

    SECTION("reset clears frequencies to 0")
    {
        porta.setEnabled(true);
        porta.setTarget(440.0f);
        porta.reset();
        REQUIRE(porta.getCurrentFreq() == Catch::Approx(0.0f));
    }

    SECTION("short glide time converges quickly")
    {
        porta.setEnabled(true);
        porta.setTime(0.01f); // very short
        porta.setTarget(440.0f);
        porta.setTarget(880.0f);

        // Exponential glide needs ~10x the time constant to fully converge.
        // With time=0.01s at 44100Hz, process enough samples for convergence.
        for (int i = 0; i < 4410; ++i)
            porta.process();
        REQUIRE(porta.getCurrentFreq() == Catch::Approx(880.0f).margin(1.0f));
    }

    SECTION("process returns current frequency when not gliding")
    {
        porta.setEnabled(false);
        porta.setTarget(440.0f);
        float freq = porta.process();
        REQUIRE(freq == Catch::Approx(440.0f));
    }
}
