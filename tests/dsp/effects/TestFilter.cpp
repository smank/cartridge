#include <catch2/catch_test_macros.hpp>
#include "dsp/effects/Filter.h"
#include <cmath>

TEST_CASE("Filter", "[dsp][effects]")
{
    cart::Filter filter;
    filter.prepare(44100.0, 512);

    SECTION("disabled is passthrough")
    {
        filter.setEnabled(false);
        juce::AudioBuffer<float> buffer(2, 512);
        for (int i = 0; i < 512; ++i)
        {
            buffer.setSample(0, i, std::sin(static_cast<float>(i) * 0.1f));
            buffer.setSample(1, i, std::sin(static_cast<float>(i) * 0.1f));
        }

        juce::AudioBuffer<float> original(2, 512);
        original.makeCopyOf(buffer);

        filter.process(buffer);

        for (int i = 0; i < 512; ++i)
            REQUIRE(buffer.getSample(0, i) == original.getSample(0, i));
    }

    SECTION("low-pass attenuates high frequencies")
    {
        filter.setEnabled(true);
        filter.setType(0); // LP
        filter.setCutoff(200.0f); // Very low cutoff
        filter.setResonance(0.707f);

        // Generate high-frequency signal (5000 Hz)
        juce::AudioBuffer<float> buffer(2, 2048);
        float freq = 5000.0f;
        for (int i = 0; i < 2048; ++i)
        {
            float val = std::sin(2.0f * 3.14159f * freq * static_cast<float>(i) / 44100.0f);
            buffer.setSample(0, i, val);
            buffer.setSample(1, i, val);
        }

        float inputRms = buffer.getRMSLevel(0, 0, 2048);
        filter.process(buffer);
        float outputRms = buffer.getRMSLevel(0, 0, 2048);

        // High freq should be heavily attenuated
        REQUIRE(outputRms < inputRms * 0.3f);
    }

    SECTION("high-pass attenuates low frequencies")
    {
        filter.setEnabled(true);
        filter.setType(2); // HP
        filter.setCutoff(5000.0f); // High cutoff
        filter.setResonance(0.707f);

        // Generate low-frequency signal (100 Hz)
        juce::AudioBuffer<float> buffer(2, 2048);
        float freq = 100.0f;
        for (int i = 0; i < 2048; ++i)
        {
            float val = std::sin(2.0f * 3.14159f * freq * static_cast<float>(i) / 44100.0f);
            buffer.setSample(0, i, val);
            buffer.setSample(1, i, val);
        }

        float inputRms = buffer.getRMSLevel(0, 0, 2048);
        filter.process(buffer);
        float outputRms = buffer.getRMSLevel(0, 0, 2048);

        REQUIRE(outputRms < inputRms * 0.3f);
    }

    SECTION("band-pass passes center frequency")
    {
        filter.setEnabled(true);
        filter.setType(1); // BP
        filter.setCutoff(1000.0f);
        filter.setResonance(2.0f);

        // Generate signal at cutoff frequency
        juce::AudioBuffer<float> buffer(2, 4096);
        float freq = 1000.0f;
        for (int i = 0; i < 4096; ++i)
        {
            float val = std::sin(2.0f * 3.14159f * freq * static_cast<float>(i) / 44100.0f);
            buffer.setSample(0, i, val);
            buffer.setSample(1, i, val);
        }

        filter.process(buffer);
        float outputRms = buffer.getRMSLevel(0, 0, 4096);

        // Signal at center freq should pass through with reasonable level
        REQUIRE(outputRms > 0.1f);
    }

    SECTION("reset does not crash")
    {
        filter.setEnabled(true);
        filter.setType(0);
        filter.setCutoff(1000.0f);
        filter.reset();
        // Just verifying no crash
        REQUIRE(true);
    }
}
