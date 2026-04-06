#include <catch2/catch_test_macros.hpp>
#include "dsp/components/EnvelopeGenerator.h"

TEST_CASE("EnvelopeGenerator", "[dsp][components]")
{
    cart::EnvelopeGenerator env;

    SECTION("reset clears state")
    {
        env.setConstantVolume(false);
        env.start();
        env.clock();
        env.reset();
        // After reset, decayLevel is 0, so decay mode outputs 0
        REQUIRE(env.output() == 0);
    }

    SECTION("constant volume mode outputs volume directly")
    {
        env.setConstantVolume(true);
        env.setVolume(10);
        REQUIRE(env.output() == 10);
    }

    SECTION("volume is masked to 4 bits")
    {
        env.setConstantVolume(true);
        env.setVolume(0x1F); // Should mask to 0x0F
        REQUIRE(env.output() == 0x0F);
    }

    SECTION("start + clock initializes decay to 15")
    {
        env.setConstantVolume(false);
        env.setVolume(5); // period = 5
        env.start();
        env.clock(); // processes start flag: decayLevel=15, dividerCount=5
        REQUIRE(env.output() == 15);
    }

    SECTION("decay counts down after divider expires")
    {
        env.setConstantVolume(false);
        env.setVolume(2); // period=2
        env.start();
        env.clock(); // start: decayLevel=15, divider=2
        // Need 3 more clocks for divider to reach 0 and decrement decayLevel
        env.clock(); // divider 2->1
        env.clock(); // divider 1->0
        env.clock(); // divider=0, reload to 2, decayLevel 15->14
        REQUIRE(env.output() == 14);
    }

    SECTION("decay reaches zero without loop")
    {
        env.setConstantVolume(false);
        env.setVolume(0); // period=0 -> divider reloads to 0 each time, decays fast
        env.setLoop(false);
        env.start();
        env.clock(); // start: decayLevel=15, divider=0
        // Each subsequent clock: divider is 0, reloads to 0, decrements decayLevel
        for (int i = 0; i < 16; ++i)
            env.clock();
        REQUIRE(env.output() == 0);
        env.clock(); // Should stay at 0
        REQUIRE(env.output() == 0);
    }

    SECTION("decay loops when loop flag is set")
    {
        env.setConstantVolume(false);
        env.setVolume(0); // fast decay
        env.setLoop(true);
        env.start();
        env.clock(); // start: decayLevel=15
        for (int i = 0; i < 15; ++i)
            env.clock(); // decayLevel goes 15->0
        REQUIRE(env.output() == 0);
        env.clock(); // loop: decayLevel wraps to 15
        REQUIRE(env.output() == 15);
    }

    SECTION("start flag is consumed by one clock")
    {
        env.setConstantVolume(false);
        env.setVolume(0);
        env.start();
        env.clock(); // consumes start, decayLevel=15
        REQUIRE(env.output() == 15);
        // Second clock should not re-trigger start
        env.clock(); // normal decay: decayLevel=14
        REQUIRE(env.output() == 14);
    }
}
