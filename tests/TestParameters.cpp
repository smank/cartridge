#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "TestHelpers.h"
#include "Parameters.h"
#include <set>
#include <string>

TEST_CASE("Parameters", "[params]")
{
    test::StubProcessor proc;
    auto apvts = test::makeTestApvts(proc);

    SECTION("all ParamIDs exist in layout")
    {
        // Check a selection of critical parameter IDs
        REQUIRE(apvts->getParameter(cart::ParamIDs::MasterVolume) != nullptr);
        REQUIRE(apvts->getParameter(cart::ParamIDs::MasterTune) != nullptr);
        REQUIRE(apvts->getParameter(cart::ParamIDs::Region) != nullptr);
        REQUIRE(apvts->getParameter(cart::ParamIDs::P1Enabled) != nullptr);
        REQUIRE(apvts->getParameter(cart::ParamIDs::P1Duty) != nullptr);
        REQUIRE(apvts->getParameter(cart::ParamIDs::P1Volume) != nullptr);
        REQUIRE(apvts->getParameter(cart::ParamIDs::P2Enabled) != nullptr);
        REQUIRE(apvts->getParameter(cart::ParamIDs::TriEnabled) != nullptr);
        REQUIRE(apvts->getParameter(cart::ParamIDs::NoiseEnabled) != nullptr);
        REQUIRE(apvts->getParameter(cart::ParamIDs::DpcmEnabled) != nullptr);
        REQUIRE(apvts->getParameter(cart::ParamIDs::Vrc6Enabled) != nullptr);
        REQUIRE(apvts->getParameter(cart::ParamIDs::EngineMode) != nullptr);
        REQUIRE(apvts->getParameter(cart::ParamIDs::ArpEnabled) != nullptr);
        REQUIRE(apvts->getParameter(cart::ParamIDs::BcEnabled) != nullptr);
        REQUIRE(apvts->getParameter(cart::ParamIDs::FltEnabled) != nullptr);
        REQUIRE(apvts->getParameter(cart::ParamIDs::ChEnabled) != nullptr);
        REQUIRE(apvts->getParameter(cart::ParamIDs::DlEnabled) != nullptr);
        REQUIRE(apvts->getParameter(cart::ParamIDs::RvEnabled) != nullptr);
        REQUIRE(apvts->getParameter(cart::ParamIDs::PortaEnabled) != nullptr);
        REQUIRE(apvts->getParameter(cart::ParamIDs::LfoEnabled) != nullptr);
        REQUIRE(apvts->getParameter(cart::ParamIDs::TuningSystem) != nullptr);
        REQUIRE(apvts->getParameter(cart::ParamIDs::P1Pan) != nullptr);
        REQUIRE(apvts->getParameter(cart::ParamIDs::Vrc6SawPan) != nullptr);
    }

    SECTION("no duplicate parameter IDs")
    {
        // Use the APVTS to iterate all parameters and check for duplicates
        auto& params = apvts->processor.getParameters();
        std::set<std::string> ids;
        int total = 0;
        for (auto* param : params)
        {
            if (auto* p = dynamic_cast<juce::RangedAudioParameter*>(param))
            {
                ids.insert(p->getParameterID().toStdString());
                ++total;
            }
        }
        REQUIRE(total > 0);
        REQUIRE(static_cast<int>(ids.size()) == total);
    }

    SECTION("master volume default is 0.8")
    {
        auto* param = apvts->getParameter(cart::ParamIDs::MasterVolume);
        float defaultVal = param->convertFrom0to1(param->getDefaultValue());
        REQUIRE(defaultVal == Catch::Approx(0.8f).margin(0.01f));
    }

    SECTION("pulse volume range is 0-15")
    {
        auto* param = apvts->getParameter(cart::ParamIDs::P1Volume);
        auto range = param->getNormalisableRange();
        REQUIRE(range.start == Catch::Approx(0.0f));
        REQUIRE(range.end == Catch::Approx(15.0f));
    }

    SECTION("pitch bend range default is 2")
    {
        auto* param = apvts->getParameter(cart::ParamIDs::PitchBendRange);
        float defaultVal = param->convertFrom0to1(param->getDefaultValue());
        REQUIRE(defaultVal == Catch::Approx(2.0f).margin(0.5f));
    }
}
