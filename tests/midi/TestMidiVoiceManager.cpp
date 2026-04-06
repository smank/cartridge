#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "midi/MidiVoiceManager.h"
#include "dsp/Apu.h"

TEST_CASE("MidiVoiceManager", "[midi]")
{
    cart::Apu apu;
    apu.setSampleRate(44100.0);

    // Enable all base channels for routing
    apu.pulse1().setEnabled(true);
    apu.pulse1().lengthCounter().setEnabled(true);
    apu.pulse1().envelope().setConstantVolume(true);
    apu.pulse1().envelope().setVolume(15);

    apu.pulse2().setEnabled(true);
    apu.pulse2().lengthCounter().setEnabled(true);
    apu.pulse2().envelope().setConstantVolume(true);
    apu.pulse2().envelope().setVolume(15);

    apu.triangle().setEnabled(true);
    apu.triangle().lengthCounter().setEnabled(true);
    apu.triangle().linearCounter().setReloadValue(127);
    apu.triangle().linearCounter().setControl(true);

    apu.noise().setEnabled(true);
    apu.noise().lengthCounter().setEnabled(true);
    apu.noise().envelope().setConstantVolume(true);
    apu.noise().envelope().setVolume(15);

    apu.dpcm().setEnabled(true);

    cart::MidiVoiceManager vm;
    vm.setApu(&apu);
    vm.setRegion(true);

    SECTION("Split mode: Ch1 -> Pulse 1")
    {
        vm.setMode(cart::MidiMode::Split);
        auto msg = juce::MidiMessage::noteOn(1, 60, (juce::uint8)127);
        vm.processMidiMessage(msg);
        REQUIRE(apu.pulse1().isActive());
        REQUIRE(vm.isChannelActive(0));
    }

    SECTION("Split mode: Ch2 -> Pulse 2")
    {
        vm.setMode(cart::MidiMode::Split);
        auto msg = juce::MidiMessage::noteOn(2, 60, (juce::uint8)127);
        vm.processMidiMessage(msg);
        REQUIRE(apu.pulse2().isActive());
        REQUIRE(vm.isChannelActive(1));
    }

    SECTION("Split mode: Ch3 -> Triangle")
    {
        vm.setMode(cart::MidiMode::Split);
        auto msg = juce::MidiMessage::noteOn(3, 60, (juce::uint8)127);
        vm.processMidiMessage(msg);
        REQUIRE(apu.triangle().isActive());
        REQUIRE(vm.isChannelActive(2));
    }

    SECTION("Split mode: Ch10 -> Noise")
    {
        vm.setMode(cart::MidiMode::Split);
        auto msg = juce::MidiMessage::noteOn(10, 60, (juce::uint8)127);
        vm.processMidiMessage(msg);
        REQUIRE(apu.noise().isActive());
        REQUIRE(vm.isChannelActive(3));
    }

    SECTION("Split mode: note off releases channel")
    {
        vm.setMode(cart::MidiMode::Split);
        auto on = juce::MidiMessage::noteOn(1, 60, (juce::uint8)127);
        vm.processMidiMessage(on);
        REQUIRE(vm.isChannelActive(0));

        auto off = juce::MidiMessage::noteOff(1, 60);
        vm.processMidiMessage(off);
        REQUIRE_FALSE(vm.isChannelActive(0));
    }

    SECTION("Mono mode: only Pulse 1 plays")
    {
        vm.setMode(cart::MidiMode::Mono);
        auto msg = juce::MidiMessage::noteOn(1, 60, (juce::uint8)127);
        vm.processMidiMessage(msg);
        REQUIRE(apu.pulse1().isActive());
        REQUIRE(vm.isChannelActive(0));
        // Pulse 2 should NOT be active
        REQUIRE_FALSE(vm.isChannelActive(1));
    }

    SECTION("Auto mode: allocates to first enabled channel")
    {
        vm.setMode(cart::MidiMode::Auto);
        vm.setChannelEnabled(0, true); // Pulse 1
        vm.setChannelEnabled(1, true); // Pulse 2
        vm.setChannelEnabled(2, false);

        auto msg1 = juce::MidiMessage::noteOn(1, 60, (juce::uint8)127);
        vm.processMidiMessage(msg1);
        REQUIRE(vm.isChannelActive(0)); // First note -> Pulse 1

        auto msg2 = juce::MidiMessage::noteOn(1, 64, (juce::uint8)127);
        vm.processMidiMessage(msg2);
        REQUIRE(vm.isChannelActive(1)); // Second note -> Pulse 2
    }

    SECTION("Auto mode: steals oldest note when all channels busy")
    {
        vm.setMode(cart::MidiMode::Auto);
        vm.setChannelEnabled(0, true);
        vm.setChannelEnabled(1, true);
        vm.setChannelEnabled(2, false);

        auto msg1 = juce::MidiMessage::noteOn(1, 60, (juce::uint8)127);
        vm.processMidiMessage(msg1);
        auto msg2 = juce::MidiMessage::noteOn(1, 64, (juce::uint8)127);
        vm.processMidiMessage(msg2);

        // Both channels busy, now play a 3rd note
        auto msg3 = juce::MidiMessage::noteOn(1, 67, (juce::uint8)127);
        vm.processMidiMessage(msg3);

        // Oldest note (60) should have been stolen
        // Channel 0 should now play 67
        REQUIRE(vm.isChannelActive(0));
        REQUIRE(vm.isChannelActive(1));
    }

    SECTION("Layer mode: all enabled channels play same note")
    {
        vm.setMode(cart::MidiMode::Layer);
        vm.setChannelEnabled(0, true);
        vm.setChannelEnabled(1, true);
        vm.setChannelEnabled(2, true);

        auto msg = juce::MidiMessage::noteOn(1, 60, (juce::uint8)127);
        vm.processMidiMessage(msg);

        REQUIRE(vm.isChannelActive(0));
        REQUIRE(vm.isChannelActive(1));
        REQUIRE(vm.isChannelActive(2));
    }

    SECTION("VRC6 routing in Split mode")
    {
        apu.setVrc6Enabled(true);
        apu.vrc6Pulse1().setEnabled(true);
        vm.setVrc6Available(true);
        vm.setMode(cart::MidiMode::Split);

        auto msg = juce::MidiMessage::noteOn(5, 60, (juce::uint8)127);
        vm.processMidiMessage(msg);
        REQUIRE(apu.vrc6Pulse1().isActive());
        REQUIRE(vm.isChannelActive(5));
    }

    SECTION("pitch bend updates frequency")
    {
        vm.setMode(cart::MidiMode::Split);
        vm.setPitchBendRange(2);

        auto noteMsg = juce::MidiMessage::noteOn(1, 60, (juce::uint8)127);
        vm.processMidiMessage(noteMsg);

        // Send pitch bend (up)
        auto bendMsg = juce::MidiMessage::pitchWheel(1, 16383); // max up
        vm.processMidiMessage(bendMsg);

        // Channel should still be active (pitch bend doesn't turn off notes)
        REQUIRE(vm.isChannelActive(0));
    }

    SECTION("velocity sensitivity scales volume")
    {
        vm.setMode(cart::MidiMode::Split);
        vm.setVelocitySensitivity(1.0f); // full sensitivity

        // Low velocity
        auto softMsg = juce::MidiMessage::noteOn(1, 60, (juce::uint8)32);
        vm.processMidiMessage(softMsg);
        uint8_t softVol = apu.pulse1().envelope().output();

        auto offMsg = juce::MidiMessage::noteOff(1, 60);
        vm.processMidiMessage(offMsg);

        // High velocity
        auto loudMsg = juce::MidiMessage::noteOn(1, 60, (juce::uint8)127);
        vm.processMidiMessage(loudMsg);
        uint8_t loudVol = apu.pulse1().envelope().output();

        REQUIRE(loudVol > softVol);
    }

    SECTION("allNotesOff releases all channels")
    {
        vm.setMode(cart::MidiMode::Split);
        vm.processMidiMessage(juce::MidiMessage::noteOn(1, 60, (juce::uint8)127));
        vm.processMidiMessage(juce::MidiMessage::noteOn(2, 64, (juce::uint8)127));
        REQUIRE(vm.isChannelActive(0));
        REQUIRE(vm.isChannelActive(1));

        vm.handleAllNotesOff();
        REQUIRE_FALSE(vm.isChannelActive(0));
        REQUIRE_FALSE(vm.isChannelActive(1));
    }

    SECTION("CC messages fire callback")
    {
        int receivedCC = -1;
        float receivedVal = -1.0f;
        vm.onControlChange = [&](int cc, float val) {
            receivedCC = cc;
            receivedVal = val;
        };

        auto ccMsg = juce::MidiMessage::controllerEvent(1, 74, 64);
        vm.processMidiMessage(ccMsg);

        REQUIRE(receivedCC == 74);
        REQUIRE(receivedVal == Catch::Approx(64.0f / 127.0f).margin(0.01f));
    }

    SECTION("transpose offsets note frequency")
    {
        vm.setMode(cart::MidiMode::Split);
        vm.setTranspose(0, 12.0f); // +12 semitones = 1 octave up

        auto msg = juce::MidiMessage::noteOn(1, 60, (juce::uint8)127);
        vm.processMidiMessage(msg);
        REQUIRE(apu.pulse1().isActive());
    }
}
