#include <catch2/catch_test_macros.hpp>
#include "dsp/components/LinearCounter.h"

TEST_CASE("LinearCounter", "[dsp][components]")
{
    cart::LinearCounter lc;

    SECTION("reset clears state")
    {
        lc.setReloadValue(100);
        lc.reload();
        lc.clock();
        lc.reset();
        REQUIRE(lc.value() == 0);
        REQUIRE_FALSE(lc.isActive());
    }

    SECTION("reload + clock loads counter")
    {
        lc.setReloadValue(50);
        lc.setControl(true);
        lc.reload();
        lc.clock(); // reload flag set -> counter = 50
        REQUIRE(lc.value() == 50);
        REQUIRE(lc.isActive());
    }

    SECTION("reload value masked to 7 bits")
    {
        lc.setReloadValue(0xFF); // Should mask to 0x7F = 127
        lc.setControl(true);
        lc.reload();
        lc.clock();
        REQUIRE(lc.value() == 127);
    }

    SECTION("counter decrements each clock")
    {
        lc.setReloadValue(3);
        lc.setControl(false); // control=false means reload flag is cleared after first clock
        lc.reload();
        lc.clock(); // counter = 3, clears reload flag (control=false)
        REQUIRE(lc.value() == 3);
        lc.clock(); // counter = 2
        REQUIRE(lc.value() == 2);
        lc.clock(); // counter = 1
        REQUIRE(lc.value() == 1);
        lc.clock(); // counter = 0
        REQUIRE(lc.value() == 0);
        REQUIRE_FALSE(lc.isActive());
    }

    SECTION("control flag keeps reload flag active")
    {
        lc.setReloadValue(5);
        lc.setControl(true); // reload flag stays set
        lc.reload();
        lc.clock(); // counter = 5
        lc.clock(); // reload flag still set -> counter = 5 again
        REQUIRE(lc.value() == 5);
    }

    SECTION("counter does not go below 0")
    {
        lc.setReloadValue(1);
        lc.setControl(false);
        lc.reload();
        lc.clock(); // counter = 1
        lc.clock(); // counter = 0
        lc.clock(); // stays at 0
        REQUIRE(lc.value() == 0);
    }

    SECTION("isActive returns false at 0")
    {
        lc.setReloadValue(0);
        lc.setControl(false);
        lc.reload();
        lc.clock(); // counter = 0
        REQUIRE_FALSE(lc.isActive());
    }
}
