#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "TestHelpers.h"
#include "ABCompare.h"

TEST_CASE("ABCompare", "[ab]")
{
    test::StubProcessor proc;
    auto apvts = test::makeTestApvts(proc);

    cart::ABCompare ab;
    ab.initialize(*apvts);

    SECTION("starts showing A")
    {
        REQUIRE(ab.isShowingA());
    }

    SECTION("toggle switches to B and back")
    {
        ab.toggle(*apvts);
        REQUIRE_FALSE(ab.isShowingA());
        ab.toggle(*apvts);
        REQUIRE(ab.isShowingA());
    }

    SECTION("toggle preserves state changes")
    {
        // Change a parameter while showing A
        if (auto* param = apvts->getParameter(cart::ParamIDs::MasterVolume))
            param->setValueNotifyingHost(0.5f);

        ab.toggle(*apvts); // Save A, load B (which was initialized with original values)

        // Now change something in B
        if (auto* param = apvts->getParameter(cart::ParamIDs::MasterVolume))
            param->setValueNotifyingHost(0.2f);

        ab.toggle(*apvts); // Save B, load A

        // A should have the value we set (0.5)
        if (auto* param = apvts->getParameter(cart::ParamIDs::MasterVolume))
        {
            float val = param->getValue();
            REQUIRE(val == Catch::Approx(0.5f).margin(0.05f));
        }
    }

    SECTION("re-initialize after state restore")
    {
        ab.toggle(*apvts);
        ab.toggle(*apvts);
        // Re-initialize (simulates setStateInformation)
        ab.initialize(*apvts);
        REQUIRE(ab.isShowingA());
    }
}
