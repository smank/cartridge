#include <catch2/catch_test_macros.hpp>
#include "TestHelpers.h"
#include "PresetManager.h"

TEST_CASE("PresetManager", "[presets]")
{
    cart::PresetManager pm;

    SECTION("has factory presets")
    {
        REQUIRE(pm.getNumPresets() >= 20); // At least 20 factory presets
    }

    SECTION("factory preset count matches")
    {
        REQUIRE(pm.getFactoryPresetCount() > 0);
        REQUIRE(pm.getFactoryPresetCount() <= pm.getNumPresets());
    }

    SECTION("all factory presets have names")
    {
        for (int i = 0; i < pm.getFactoryPresetCount(); ++i)
        {
            auto name = pm.getPresetName(i);
            REQUIRE(name.isNotEmpty());
        }
    }

    SECTION("applyPreset does not crash")
    {
        test::StubProcessor proc;
        auto apvts = test::makeTestApvts(proc);

        for (int i = 0; i < pm.getFactoryPresetCount(); ++i)
        {
            REQUIRE_NOTHROW(pm.applyPreset(i, *apvts));
        }
    }

    SECTION("isFactoryPreset returns correct value")
    {
        REQUIRE(pm.isFactoryPreset(0));
        // Index beyond factory count should not be factory
        if (pm.getNumPresets() > pm.getFactoryPresetCount())
            REQUIRE_FALSE(pm.isFactoryPreset(pm.getNumPresets() - 1));
    }
}
