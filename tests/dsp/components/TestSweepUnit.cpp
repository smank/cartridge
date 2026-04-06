#include <catch2/catch_test_macros.hpp>
#include "dsp/components/SweepUnit.h"

TEST_CASE("SweepUnit", "[dsp][components]")
{
    cart::SweepUnit sweep;

    SECTION("reset clears reload flag")
    {
        sweep.reload();
        sweep.reset();
        // After reset, clock should not modify period on first call
        uint16_t period = 500;
        sweep.setEnabled(true);
        sweep.setShift(1);
        sweep.setPeriod(0);
        sweep.clock(period);
        // Without reload flag set, divider starts at 0, should adjust
        // But after reset, reloadFlag is false and divider is 0
    }

    SECTION("targetPeriod positive shift adds")
    {
        sweep.setChannel(0);
        sweep.setNegate(false);
        sweep.setShift(2);
        // target = current + (current >> shift)
        REQUIRE(sweep.targetPeriod(400) == 400 + 100); // 400 + 400>>2
    }

    SECTION("targetPeriod channel 0 negate uses ones complement")
    {
        sweep.setChannel(0);
        sweep.setNegate(true);
        sweep.setShift(1);
        // Channel 0: target = current - (current >> shift) - 1
        REQUIRE(sweep.targetPeriod(400) == 400 - 200 - 1); // 199
    }

    SECTION("targetPeriod channel 1 negate uses twos complement")
    {
        sweep.setChannel(1);
        sweep.setNegate(true);
        sweep.setShift(1);
        // Channel 1: target = current - (current >> shift)
        REQUIRE(sweep.targetPeriod(400) == 400 - 200); // 200
    }

    SECTION("muting when period < 8")
    {
        sweep.setShift(0);
        REQUIRE(sweep.isMuting(7));
        REQUIRE_FALSE(sweep.isMuting(8));
    }

    SECTION("muting when target > 0x7FF")
    {
        sweep.setChannel(0);
        sweep.setNegate(false);
        sweep.setShift(1);
        // target = 1500 + 750 = 2250 > 2047
        REQUIRE(sweep.isMuting(1500));
    }

    SECTION("clock modifies period when enabled with shift > 0")
    {
        sweep.setChannel(0);
        sweep.setEnabled(true);
        sweep.setShift(1);
        sweep.setPeriod(0); // divider period = 0
        sweep.setNegate(false);

        uint16_t period = 400;
        sweep.reload();
        // First clock with reload flag: divider is 0, enabled, shift > 0, not muting
        // Should update period
        bool changed = sweep.clock(period);
        REQUIRE(changed);
        REQUIRE(period == 400 + 200); // 600
    }

    SECTION("clock does not modify when disabled")
    {
        sweep.setChannel(0);
        sweep.setEnabled(false);
        sweep.setShift(1);
        sweep.setPeriod(0);
        sweep.setNegate(false);

        uint16_t period = 400;
        sweep.reload();
        sweep.clock(period);
        REQUIRE(period == 400); // unchanged
    }

    SECTION("clock does not modify when shift is 0")
    {
        sweep.setChannel(0);
        sweep.setEnabled(true);
        sweep.setShift(0);
        sweep.setPeriod(0);

        uint16_t period = 400;
        sweep.reload();
        sweep.clock(period);
        REQUIRE(period == 400);
    }
}
