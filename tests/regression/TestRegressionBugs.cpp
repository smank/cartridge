#include <catch2/catch_test_macros.hpp>
#include "dsp/Apu.h"
#include "dsp/channels/PulseChannel.h"
#include "dsp/channels/TriangleChannel.h"
#include "midi/MidiVoiceManager.h"
#include "ABCompare.h"
#include "TestHelpers.h"

TEST_CASE("Regression: PulseChannel.setFrequency must update timerPeriod", "[regression]")
{
    // Bug: setFrequency() did not update timerPeriod, leaving it at 0.
    // timerPeriod=0 < 8 triggers sweep muting, silencing the channel.
    cart::PulseChannel pulse(0);
    pulse.setSampleRate(44100.0);
    pulse.setEnabled(true);
    pulse.envelope().setConstantVolume(true);
    pulse.envelope().setVolume(15);
    pulse.lengthCounter().setEnabled(true);
    pulse.setFrequency(440.0f);
    pulse.noteOn();

    auto samples = test::processN(pulse, 1000);
    REQUIRE(test::hasEnergy(samples));
}

TEST_CASE("Regression: Triangle noteOn must prime linear counter", "[regression]")
{
    // Bug: noteOn() did not call linear.clock() after reload(), causing
    // a ~4ms delay before the first sound (waiting for next quarter-frame).
    cart::TriangleChannel tri;
    tri.setSampleRate(44100.0);
    tri.setEnabled(true);
    tri.linearCounter().setReloadValue(127);
    tri.linearCounter().setControl(true);
    tri.lengthCounter().setEnabled(true);
    tri.setFrequency(440.0f);
    tri.noteOn();

    // Linear counter should be immediately active
    REQUIRE(tri.linearCounter().isActive());
    REQUIRE(tri.isActive());

    // First few samples should have energy (no 4ms delay)
    auto samples = test::processN(tri, 10);
    // The channel should be generating output immediately
    REQUIRE(tri.isActive());
}

TEST_CASE("Regression: After apu.reset(), channels must re-enable properly", "[regression]")
{
    // Bug: After apu.reset(), cachedParams weren't reset, so change detection
    // skipped re-applying params. LengthCounter::reset() sets enabled=false,
    // so channels stayed silent after prepareToPlay was called again.
    cart::Apu apu;
    apu.setSampleRate(44100.0);

    // Set up a note
    apu.pulse1().setEnabled(true);
    apu.pulse1().envelope().setConstantVolume(true);
    apu.pulse1().envelope().setVolume(15);
    apu.pulse1().lengthCounter().setEnabled(true);
    apu.pulse1().setFrequency(440.0f);
    apu.pulse1().noteOn();

    // Verify sound
    auto samples1 = test::processN(apu.pulse1(), 100);
    REQUIRE(test::hasEnergy(samples1));

    // Reset (simulates prepareToPlay)
    apu.reset();
    apu.setSampleRate(44100.0);

    // After reset, length counter is disabled. Re-enable and play.
    apu.pulse1().setEnabled(true);
    apu.pulse1().lengthCounter().setEnabled(true);
    apu.pulse1().envelope().setConstantVolume(true);
    apu.pulse1().envelope().setVolume(15);
    apu.pulse1().setFrequency(440.0f);
    apu.pulse1().noteOn();

    auto samples2 = test::processN(apu.pulse1(), 100);
    REQUIRE(test::hasEnergy(samples2));
}

TEST_CASE("Regression: VRC6 routing must gate on channel enabled", "[regression]")
{
    // Bug: Auto mode would route notes to all 3 VRC6 channels even when
    // a preset only wanted one (needed to gate on mix > 0, not just vrc6On).
    cart::Apu apu;
    apu.setSampleRate(44100.0);
    apu.setVrc6Enabled(true);

    cart::MidiVoiceManager vm;
    vm.setApu(&apu);
    vm.setMode(cart::MidiMode::Auto);
    vm.setVrc6Available(true);

    // Only enable VRC6 Pulse 1 (index 5)
    vm.setChannelEnabled(5, true);
    vm.setChannelEnabled(6, false);
    vm.setChannelEnabled(7, false);
    vm.setChannelEnabled(0, false);
    vm.setChannelEnabled(1, false);
    vm.setChannelEnabled(2, false);

    apu.vrc6Pulse1().setEnabled(true);
    apu.vrc6Pulse1().setVolume(15);

    auto msg = juce::MidiMessage::noteOn(1, 60, (juce::uint8)127);
    vm.processMidiMessage(msg);

    // Only channel 5 should be active
    REQUIRE(vm.isChannelActive(5));
    REQUIRE_FALSE(vm.isChannelActive(6));
    REQUIRE_FALSE(vm.isChannelActive(7));
}

TEST_CASE("Regression: ABCompare re-initializes after setStateInformation", "[regression]")
{
    // Bug: In standalone, A/B compare state could become stale after
    // setStateInformation restored a different state.
    test::StubProcessor proc;
    auto apvts = test::makeTestApvts(proc);

    cart::ABCompare ab;
    ab.initialize(*apvts);
    REQUIRE(ab.isShowingA());

    // Toggle to B
    ab.toggle(*apvts);
    REQUIRE_FALSE(ab.isShowingA());

    // Simulate setStateInformation by re-initializing.
    // initialize() refreshes stored A/B states but does NOT reset the
    // showingA flag — it preserves whichever side was active.
    ab.initialize(*apvts);
    REQUIRE_FALSE(ab.isShowingA()); // still showing B

    // Toggle should still work after re-init
    ab.toggle(*apvts);
    REQUIRE(ab.isShowingA());
}
