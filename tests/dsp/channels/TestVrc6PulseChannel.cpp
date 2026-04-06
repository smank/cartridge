#include <catch2/catch_test_macros.hpp>
#include "dsp/channels/Vrc6PulseChannel.h"
#include "TestHelpers.h"

TEST_CASE("Vrc6PulseChannel", "[dsp][channels]")
{
    cart::Vrc6PulseChannel vp;
    vp.setSampleRate(44100.0);
    vp.setEnabled(true);

    SECTION("no output before noteOn")
    {
        vp.setFrequency(440.0f);
        auto samples = test::processN(vp, 100);
        REQUIRE_FALSE(test::hasEnergy(samples));
    }

    SECTION("produces output after noteOn")
    {
        vp.setVolume(15);
        vp.noteOn(440.0f);
        auto samples = test::processN(vp, 1000);
        REQUIRE(test::hasEnergy(samples));
    }

    SECTION("different duty cycles both produce sound with different waveforms")
    {
        vp.setVolume(15);
        vp.setDuty(0); // 1/16 = 6.25%
        vp.noteOn(440.0f);
        auto samplesLow = test::processN(vp, 4000);
        float rmsLow = test::rms(samplesLow);

        vp.noteOff();
        vp.reset();
        vp.setSampleRate(44100.0);
        vp.setEnabled(true);
        vp.setVolume(15);
        vp.setDuty(7); // 8/16 = 50%
        vp.noteOn(440.0f);
        auto samplesHigh = test::processN(vp, 4000);
        float rmsHigh = test::rms(samplesHigh);

        // Both duty cycles should produce sound
        REQUIRE(rmsLow > 0.0f);
        REQUIRE(rmsHigh > 0.0f);

        // Waveforms should differ (different duty cycles produce different patterns)
        bool differ = false;
        for (size_t i = 0; i < samplesLow.size() && i < samplesHigh.size(); ++i)
        {
            if (std::abs(samplesLow[i] - samplesHigh[i]) > 1e-6f)
            {
                differ = true;
                break;
            }
        }
        REQUIRE(differ);
    }

    SECTION("volume 0 produces no output")
    {
        vp.setVolume(0);
        vp.noteOn(440.0f);
        auto samples = test::processN(vp, 1000);
        REQUIRE_FALSE(test::hasEnergy(samples));
    }

    SECTION("noteOff silences channel")
    {
        vp.setVolume(15);
        vp.noteOn(440.0f);
        test::processN(vp, 100);
        vp.noteOff();
        auto samples = test::processN(vp, 100);
        REQUIRE_FALSE(test::hasEnergy(samples));
    }

    SECTION("disabled channel produces no output")
    {
        vp.setEnabled(false);
        vp.setVolume(15);
        vp.noteOn(440.0f);
        auto samples = test::processN(vp, 100);
        REQUIRE_FALSE(test::hasEnergy(samples));
    }
}
