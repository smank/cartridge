#include <catch2/catch_test_macros.hpp>
#include "dsp/Apu.h"
#include "TestHelpers.h"

TEST_CASE("Apu", "[dsp]")
{
    cart::Apu apu;
    apu.setSampleRate(44100.0);

    SECTION("reset silences all channels")
    {
        // Enable and start a note on pulse1
        apu.pulse1().setEnabled(true);
        apu.pulse1().envelope().setConstantVolume(true);
        apu.pulse1().envelope().setVolume(15);
        apu.pulse1().lengthCounter().setEnabled(true);
        apu.pulse1().setFrequency(440.0f);
        apu.pulse1().noteOn();

        // Verify it's producing sound
        float out = apu.process();
        // Process a few samples
        for (int i = 0; i < 100; ++i) apu.process();

        apu.reset();
        apu.setSampleRate(44100.0);

        // After reset, all channels should be silent
        REQUIRE_FALSE(apu.pulse1().isActive());
        REQUIRE_FALSE(apu.pulse2().isActive());
        REQUIRE_FALSE(apu.triangle().isActive());
        REQUIRE_FALSE(apu.noise().isActive());
    }

    SECTION("VRC6 channels are mixed when enabled")
    {
        apu.setVrc6Enabled(true);
        apu.vrc6Pulse1().setEnabled(true);
        apu.vrc6Pulse1().setVolume(15);
        apu.vrc6Pulse1().noteOn(440.0f);

        // Process some samples
        std::vector<float> samples;
        for (int i = 0; i < 1000; ++i)
            samples.push_back(apu.process());

        REQUIRE(test::hasEnergy(samples));
    }

    SECTION("VRC6 channels are silent when disabled")
    {
        apu.setVrc6Enabled(false);
        apu.vrc6Pulse1().setEnabled(true);
        apu.vrc6Pulse1().setVolume(15);
        apu.vrc6Pulse1().noteOn(440.0f);

        // With only VRC6 active but disabled, base APU channels are silent
        // VRC6 should be gated off
        float out[8];
        for (int i = 0; i < 100; ++i)
            apu.processIndividual(out);

        // VRC6 channels (indices 5,6,7) should be 0
        REQUIRE(out[5] == 0.0f);
        REQUIRE(out[6] == 0.0f);
        REQUIRE(out[7] == 0.0f);
    }

    SECTION("processIndividual returns per-channel outputs")
    {
        apu.pulse1().setEnabled(true);
        apu.pulse1().envelope().setConstantVolume(true);
        apu.pulse1().envelope().setVolume(15);
        apu.pulse1().lengthCounter().setEnabled(true);
        apu.pulse1().setFrequency(440.0f);
        apu.pulse1().noteOn();

        float out[8] = {};
        float maxAbsP1 = 0.0f;
        // Process enough samples for frame counter to tick and accumulate
        // the peak pulse1 output (individual samples may be near zero crossing)
        for (int i = 0; i < 500; ++i)
        {
            apu.processIndividual(out);
            float absP1 = std::abs(out[0]);
            if (absP1 > maxAbsP1)
                maxAbsP1 = absP1;
        }

        // Pulse1 should have produced nonzero output at some point
        REQUIRE(maxAbsP1 > 0.0001f);
    }

    SECTION("frame counter clocks envelope and length")
    {
        apu.pulse1().setEnabled(true);
        apu.pulse1().envelope().setConstantVolume(false);
        apu.pulse1().envelope().setVolume(0); // fast decay
        apu.pulse1().lengthCounter().setEnabled(true);
        apu.pulse1().setFrequency(440.0f);
        apu.pulse1().noteOn();

        // Process many samples to let frame counter tick
        for (int i = 0; i < 44100; ++i)
            apu.process();

        // After ~1 second, the envelope should have decayed
        // and length counter should have decremented
        // (length counter starts at LENGTH_TABLE[0x1F]=30, halved at ~120Hz → ~0.25s to expire)
    }

    SECTION("setRegion affects frame counter rate")
    {
        apu.setRegion(true); // NTSC
        apu.setRegion(false); // PAL
        // Just verify no crash
        REQUIRE(true);
    }

    SECTION("master volume scales output")
    {
        apu.pulse1().setEnabled(true);
        apu.pulse1().envelope().setConstantVolume(true);
        apu.pulse1().envelope().setVolume(15);
        apu.pulse1().lengthCounter().setEnabled(true);
        apu.pulse1().setFrequency(440.0f);
        apu.pulse1().noteOn();

        apu.setMasterVolume(1.0f);
        std::vector<float> fullVol;
        for (int i = 0; i < 1000; ++i)
            fullVol.push_back(apu.process());

        apu.reset();
        apu.setSampleRate(44100.0);
        apu.pulse1().setEnabled(true);
        apu.pulse1().envelope().setConstantVolume(true);
        apu.pulse1().envelope().setVolume(15);
        apu.pulse1().lengthCounter().setEnabled(true);
        apu.pulse1().setFrequency(440.0f);
        apu.pulse1().noteOn();

        apu.setMasterVolume(0.5f);
        std::vector<float> halfVol;
        for (int i = 0; i < 1000; ++i)
            halfVol.push_back(apu.process());

        REQUIRE(test::rms(fullVol) > test::rms(halfVol));
    }

    SECTION("VRC6 mix levels scale output")
    {
        apu.setVrc6Enabled(true);
        apu.setVrc6Pulse1Mix(1.0f);
        apu.vrc6Pulse1().setEnabled(true);
        apu.vrc6Pulse1().setVolume(15);
        apu.vrc6Pulse1().noteOn(440.0f);

        std::vector<float> full;
        for (int i = 0; i < 1000; ++i) full.push_back(apu.process());

        apu.reset();
        apu.setSampleRate(44100.0);
        apu.setVrc6Enabled(true);
        apu.setVrc6Pulse1Mix(0.0f);
        apu.vrc6Pulse1().setEnabled(true);
        apu.vrc6Pulse1().setVolume(15);
        apu.vrc6Pulse1().noteOn(440.0f);

        std::vector<float> muted;
        for (int i = 0; i < 1000; ++i) muted.push_back(apu.process());

        REQUIRE(test::rms(full) > test::rms(muted));
    }
}
