#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "dsp/effects/BitCrush.h"

TEST_CASE("BitCrush", "[dsp][effects]")
{
    cart::BitCrush bc;
    bc.prepare(44100.0, 512);

    SECTION("disabled is passthrough")
    {
        bc.setEnabled(false);
        juce::AudioBuffer<float> buffer(1, 512);
        // Fill with a ramp
        for (int i = 0; i < 512; ++i)
            buffer.setSample(0, i, static_cast<float>(i) / 512.0f);

        juce::AudioBuffer<float> original(1, 512);
        original.makeCopyOf(buffer);

        bc.process(buffer);

        for (int i = 0; i < 512; ++i)
            REQUIRE(buffer.getSample(0, i) == original.getSample(0, i));
    }

    SECTION("low bit depth quantizes signal")
    {
        bc.setEnabled(true);
        bc.setBitDepth(1.0f); // 2 levels: very harsh quantization
        bc.setRateReduce(1.0f);
        bc.setMix(1.0f);

        juce::AudioBuffer<float> buffer(1, 512);
        for (int i = 0; i < 512; ++i)
            buffer.setSample(0, i, static_cast<float>(i) / 512.0f);

        bc.process(buffer);

        // With 1-bit depth (2 levels), output should be quantized
        // Count unique values
        std::set<float> uniqueVals;
        for (int i = 0; i < 512; ++i)
            uniqueVals.insert(buffer.getSample(0, i));

        // Should have very few unique values (2-3 quantization levels)
        REQUIRE(uniqueVals.size() <= 4);
    }

    SECTION("rate reduction holds samples")
    {
        bc.setEnabled(true);
        bc.setBitDepth(16.0f); // full depth, only rate reduce
        bc.setRateReduce(50.0f); // extreme rate reduction
        bc.setMix(1.0f);

        juce::AudioBuffer<float> buffer(1, 512);
        for (int i = 0; i < 512; ++i)
            buffer.setSample(0, i, std::sin(static_cast<float>(i) * 0.1f));

        bc.process(buffer);

        // With extreme rate reduction, many consecutive samples should be equal
        int sameCount = 0;
        for (int i = 1; i < 512; ++i)
            if (buffer.getSample(0, i) == buffer.getSample(0, i - 1))
                ++sameCount;

        REQUIRE(sameCount > 300); // Most samples held
    }

    SECTION("mix parameter blends dry and wet")
    {
        bc.setEnabled(true);
        bc.setBitDepth(1.0f);
        bc.setRateReduce(1.0f);
        bc.setMix(0.0f); // 0 mix = fully dry (no effect)

        juce::AudioBuffer<float> buffer(1, 512);
        for (int i = 0; i < 512; ++i)
            buffer.setSample(0, i, static_cast<float>(i) / 512.0f);

        juce::AudioBuffer<float> original(1, 512);
        original.makeCopyOf(buffer);

        bc.process(buffer);

        // With mix=0, output should equal input
        for (int i = 0; i < 512; ++i)
            REQUIRE(buffer.getSample(0, i) == Catch::Approx(original.getSample(0, i)).margin(1e-5f));
    }

    SECTION("16-bit depth at rate 1 is near passthrough")
    {
        bc.setEnabled(true);
        bc.setBitDepth(16.0f);
        bc.setRateReduce(1.0f);
        bc.setMix(1.0f);

        juce::AudioBuffer<float> buffer(1, 512);
        for (int i = 0; i < 512; ++i)
            buffer.setSample(0, i, std::sin(static_cast<float>(i) * 0.05f));

        juce::AudioBuffer<float> original(1, 512);
        original.makeCopyOf(buffer);

        bc.process(buffer);

        // 16-bit depth should barely change the signal
        float maxDiff = 0.0f;
        for (int i = 0; i < 512; ++i)
        {
            float diff = std::abs(buffer.getSample(0, i) - original.getSample(0, i));
            maxDiff = std::max(maxDiff, diff);
        }
        REQUIRE(maxDiff < 0.001f);
    }
}
