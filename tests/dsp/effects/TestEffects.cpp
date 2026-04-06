#include <catch2/catch_test_macros.hpp>
#include "dsp/effects/Chorus.h"
#include "dsp/effects/Delay.h"
#include "dsp/effects/Reverb.h"
#include <cmath>

TEST_CASE("Chorus", "[dsp][effects]")
{
    cart::Chorus chorus;
    chorus.prepare(44100.0, 512);

    SECTION("disabled is passthrough")
    {
        chorus.setEnabled(false);
        juce::AudioBuffer<float> buffer(2, 512);
        for (int i = 0; i < 512; ++i)
        {
            buffer.setSample(0, i, std::sin(static_cast<float>(i) * 0.1f));
            buffer.setSample(1, i, std::sin(static_cast<float>(i) * 0.1f));
        }
        juce::AudioBuffer<float> original(2, 512);
        original.makeCopyOf(buffer);

        chorus.process(buffer);

        for (int i = 0; i < 512; ++i)
            REQUIRE(buffer.getSample(0, i) == original.getSample(0, i));
    }

    SECTION("enabled modifies signal")
    {
        chorus.setEnabled(true);
        chorus.setRate(1.0f);
        chorus.setDepth(0.5f);
        chorus.setMix(0.5f);

        juce::AudioBuffer<float> buffer(2, 2048);
        for (int i = 0; i < 2048; ++i)
        {
            float val = std::sin(static_cast<float>(i) * 0.1f);
            buffer.setSample(0, i, val);
            buffer.setSample(1, i, val);
        }
        juce::AudioBuffer<float> original(2, 2048);
        original.makeCopyOf(buffer);

        chorus.process(buffer);

        // Some samples should be different after chorus
        int diffCount = 0;
        for (int i = 0; i < 2048; ++i)
            if (buffer.getSample(0, i) != original.getSample(0, i))
                ++diffCount;
        REQUIRE(diffCount > 0);
    }

    SECTION("reset does not crash")
    {
        chorus.setEnabled(true);
        chorus.reset();
        REQUIRE(true);
    }
}

TEST_CASE("Delay", "[dsp][effects]")
{
    cart::Delay delay;
    delay.prepare(44100.0, 512);

    SECTION("disabled is passthrough")
    {
        delay.setEnabled(false);
        juce::AudioBuffer<float> buffer(2, 512);
        for (int i = 0; i < 512; ++i)
        {
            buffer.setSample(0, i, (i == 0) ? 1.0f : 0.0f);
            buffer.setSample(1, i, (i == 0) ? 1.0f : 0.0f);
        }
        juce::AudioBuffer<float> original(2, 512);
        original.makeCopyOf(buffer);

        delay.process(buffer);

        for (int i = 0; i < 512; ++i)
            REQUIRE(buffer.getSample(0, i) == original.getSample(0, i));
    }

    SECTION("enabled produces delayed signal")
    {
        delay.setEnabled(true);
        delay.setTime(100.0f); // 100ms
        delay.setFeedback(0.0f);
        delay.setMix(1.0f);

        // Process an impulse followed by silence
        juce::AudioBuffer<float> impulse(2, 512);
        impulse.clear();
        impulse.setSample(0, 0, 1.0f);
        impulse.setSample(1, 0, 1.0f);
        delay.process(impulse);

        // Process more silence to let delay appear
        juce::AudioBuffer<float> silence(2, 8192);
        silence.clear();
        delay.process(silence);

        // There should be some energy in the silence buffer from the delayed impulse
        bool hasDelayedOutput = false;
        for (int i = 0; i < 8192; ++i)
            if (std::abs(silence.getSample(0, i)) > 0.01f)
                hasDelayedOutput = true;
        REQUIRE(hasDelayedOutput);
    }

    SECTION("reset clears delay lines")
    {
        delay.setEnabled(true);
        delay.setTime(100.0f);
        delay.setMix(1.0f);

        juce::AudioBuffer<float> impulse(2, 512);
        impulse.clear();
        impulse.setSample(0, 0, 1.0f);
        impulse.setSample(1, 0, 1.0f);
        delay.process(impulse);

        delay.reset();

        juce::AudioBuffer<float> silence(2, 8192);
        silence.clear();
        delay.process(silence);

        float maxLevel = 0.0f;
        for (int i = 0; i < 8192; ++i)
            maxLevel = std::max(maxLevel, std::abs(silence.getSample(0, i)));
        REQUIRE(maxLevel < 0.01f);
    }
}

TEST_CASE("Reverb", "[dsp][effects]")
{
    cart::Reverb reverb;
    reverb.prepare(44100.0, 512);

    SECTION("disabled is passthrough")
    {
        reverb.setEnabled(false);
        juce::AudioBuffer<float> buffer(2, 512);
        for (int i = 0; i < 512; ++i)
        {
            buffer.setSample(0, i, std::sin(static_cast<float>(i) * 0.1f));
            buffer.setSample(1, i, std::sin(static_cast<float>(i) * 0.1f));
        }
        juce::AudioBuffer<float> original(2, 512);
        original.makeCopyOf(buffer);

        reverb.process(buffer);

        for (int i = 0; i < 512; ++i)
            REQUIRE(buffer.getSample(0, i) == original.getSample(0, i));
    }

    SECTION("enabled adds tail to impulse")
    {
        reverb.setEnabled(true);
        reverb.setSize(0.8f);
        reverb.setMix(0.5f);
        reverb.setDamping(0.5f);
        reverb.setWidth(1.0f);

        // Process an impulse
        juce::AudioBuffer<float> impulse(2, 512);
        impulse.clear();
        impulse.setSample(0, 0, 1.0f);
        impulse.setSample(1, 0, 1.0f);
        reverb.process(impulse);

        // Process silence — reverb tail should still be present
        juce::AudioBuffer<float> tail(2, 4096);
        tail.clear();
        reverb.process(tail);

        bool hasTail = false;
        for (int i = 0; i < 4096; ++i)
            if (std::abs(tail.getSample(0, i)) > 0.001f)
                hasTail = true;
        REQUIRE(hasTail);
    }

    SECTION("reset does not crash")
    {
        reverb.setEnabled(true);
        reverb.setSize(0.5f);
        reverb.reset();
        REQUIRE(true);
    }
}
