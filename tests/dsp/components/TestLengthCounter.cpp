#include <catch2/catch_test_macros.hpp>
#include "dsp/components/LengthCounter.h"
#include "dsp/ApuConstants.h"

TEST_CASE("LengthCounter", "[dsp][components]")
{
    cart::LengthCounter lc;

    SECTION("reset clears state")
    {
        lc.setEnabled(true);
        lc.load(0x1F);
        lc.reset();
        REQUIRE_FALSE(lc.isActive());
        REQUIRE(lc.value() == 0);
    }

    SECTION("load uses LENGTH_TABLE lookup")
    {
        lc.setEnabled(true);
        lc.load(0); // LENGTH_TABLE[0] = 10
        REQUIRE(lc.value() == 10);
        REQUIRE(lc.isActive());
    }

    SECTION("load with index 0x1F gives max length 30")
    {
        lc.setEnabled(true);
        lc.load(0x1F); // LENGTH_TABLE[31] = 30
        REQUIRE(lc.value() == 30);
    }

    SECTION("load ignored when disabled")
    {
        lc.setEnabled(false);
        lc.load(0x1F);
        REQUIRE(lc.value() == 0);
        REQUIRE_FALSE(lc.isActive());
    }

    SECTION("clock decrements when not halted")
    {
        lc.setEnabled(true);
        lc.load(0); // value = 10
        lc.clock();
        REQUIRE(lc.value() == 9);
    }

    SECTION("clock does not decrement when halted")
    {
        lc.setEnabled(true);
        lc.load(0); // value = 10
        lc.setHalt(true);
        lc.clock();
        REQUIRE(lc.value() == 10);
    }

    SECTION("setEnabled false zeroes counter")
    {
        lc.setEnabled(true);
        lc.load(0x1F);
        REQUIRE(lc.isActive());
        lc.setEnabled(false);
        REQUIRE(lc.value() == 0);
        REQUIRE_FALSE(lc.isActive());
    }

    SECTION("all 32 table indices are valid")
    {
        for (uint8_t i = 0; i < 32; ++i)
        {
            lc.reset();
            lc.setEnabled(true);
            lc.load(i);
            REQUIRE(lc.value() == cart::LENGTH_TABLE[i]);
        }
    }
}
