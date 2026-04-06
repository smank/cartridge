#include <catch2/catch_test_macros.hpp>
#include "dsp/channels/PulseChannel.h"
#include "dsp/ApuConstants.h"
#include "TestHelpers.h"

TEST_CASE("PulseChannel", "[dsp][channels]")
{
    cart::PulseChannel pulse(0);
    pulse.setSampleRate(44100.0);
    pulse.setEnabled(true);

    SECTION("no output before noteOn")
    {
        pulse.setFrequency(440.0f);
        auto samples = test::processN(pulse, 100);
        REQUIRE_FALSE(test::hasEnergy(samples));
    }

    SECTION("produces output after noteOn")
    {
        pulse.setFrequency(440.0f);
        pulse.envelope().setConstantVolume(true);
        pulse.envelope().setVolume(15);
        pulse.lengthCounter().setEnabled(true);
        pulse.noteOn();
        auto samples = test::processN(pulse, 1000);
        REQUIRE(test::hasEnergy(samples));
    }

    SECTION("noteOff silences channel")
    {
        pulse.setFrequency(440.0f);
        pulse.envelope().setConstantVolume(true);
        pulse.envelope().setVolume(15);
        pulse.lengthCounter().setEnabled(true);
        pulse.noteOn();
        test::processN(pulse, 100);
        pulse.noteOff();
        auto samples = test::processN(pulse, 100);
        REQUIRE_FALSE(test::hasEnergy(samples));
    }

    SECTION("different duty cycles both produce sound with different waveforms")
    {
        pulse.envelope().setConstantVolume(true);
        pulse.envelope().setVolume(15);
        pulse.lengthCounter().setEnabled(true);

        pulse.setDuty(0); // 12.5%
        pulse.setFrequency(440.0f);
        pulse.noteOn();
        auto samples125 = test::processN(pulse, 4000);
        float rms125 = test::rms(samples125);

        pulse.reset();
        pulse.setSampleRate(44100.0);
        pulse.setEnabled(true);
        pulse.envelope().setConstantVolume(true);
        pulse.envelope().setVolume(15);
        pulse.lengthCounter().setEnabled(true);
        pulse.setDuty(2); // 50%
        pulse.setFrequency(440.0f);
        pulse.noteOn();
        auto samples50 = test::processN(pulse, 4000);
        float rms50 = test::rms(samples50);

        // Both duty cycles should produce sound
        REQUIRE(rms125 > 0.0f);
        REQUIRE(rms50 > 0.0f);

        // Waveforms should differ (compare sample data)
        bool differ = false;
        for (size_t i = 0; i < samples125.size() && i < samples50.size(); ++i)
        {
            if (std::abs(samples125[i] - samples50[i]) > 1e-6f)
            {
                differ = true;
                break;
            }
        }
        REQUIRE(differ);
    }

    SECTION("setFrequency updates timerPeriod for sweep muting")
    {
        // Regression test: setFrequency must update timerPeriod
        // otherwise timerPeriod stays at 0 which is < 8 -> muted
        pulse.envelope().setConstantVolume(true);
        pulse.envelope().setVolume(15);
        pulse.lengthCounter().setEnabled(true);
        pulse.setFrequency(440.0f);
        pulse.noteOn();
        auto samples = test::processN(pulse, 1000);
        REQUIRE(test::hasEnergy(samples));
    }

    SECTION("sweep muting silences when period < 8")
    {
        // Very high frequency -> tiny timerPeriod < 8 -> muted
        pulse.envelope().setConstantVolume(true);
        pulse.envelope().setVolume(15);
        pulse.lengthCounter().setEnabled(true);
        pulse.setFrequency(20000.0f); // Very high freq
        pulse.noteOn();
        auto samples = test::processN(pulse, 100);
        // timerPeriod will be very small, should be muted
        float out = pulse.process();
        // The channel is muted when timerPeriod < 8
        // At 20kHz, period = 1789772.5/(16*20000)-1 ~ 4.59, which is < 8
        REQUIRE(out == 0.0f);
    }

    SECTION("quarterFrame clocks envelope")
    {
        pulse.envelope().setConstantVolume(false);
        pulse.envelope().setVolume(0); // fast decay
        pulse.envelope().start();
        pulse.clockQuarterFrame(); // process start flag
        REQUIRE(pulse.envelope().output() == 15);
    }

    SECTION("halfFrame clocks length counter and sweep")
    {
        pulse.lengthCounter().setEnabled(true);
        pulse.lengthCounter().load(0); // LENGTH_TABLE[0] = 10
        pulse.clockHalfFrame();
        REQUIRE(pulse.lengthCounter().value() == 9);
    }

    SECTION("channel index 0 vs 1 affects sweep negate")
    {
        cart::PulseChannel p0(0);
        cart::PulseChannel p1(1);
        p0.sweep().setNegate(true);
        p0.sweep().setShift(1);
        p1.sweep().setNegate(true);
        p1.sweep().setShift(1);
        // Channel 0: ones complement, channel 1: twos complement
        REQUIRE(p0.sweep().targetPeriod(400) == 199);
        REQUIRE(p1.sweep().targetPeriod(400) == 200);
    }

    SECTION("output returns 0 when inactive")
    {
        REQUIRE(pulse.output() == 0);
    }
}
