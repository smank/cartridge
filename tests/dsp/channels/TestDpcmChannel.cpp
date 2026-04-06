#include <catch2/catch_test_macros.hpp>
#include "dsp/channels/DpcmChannel.h"
#include "dsp/ApuConstants.h"
#include "TestHelpers.h"
#include <vector>

TEST_CASE("DpcmChannel", "[dsp][channels]")
{
    cart::DpcmChannel dpcm;
    dpcm.setSampleRate(44100.0);
    dpcm.setEnabled(true);

    SECTION("no playback without sample loaded")
    {
        dpcm.setRateIndex(0, true);
        dpcm.noteOn(); // Should not start playing if no sample
        REQUIRE_FALSE(dpcm.isActive());
    }

    SECTION("noteOn starts playback with loaded sample")
    {
        std::vector<uint8_t> sample = { 0xFF, 0xFF, 0xFF, 0xFF };
        dpcm.loadSample(sample);
        dpcm.setRateIndex(15, true); // fastest rate
        dpcm.noteOn();
        REQUIRE(dpcm.isActive());
    }

    SECTION("output starts at midpoint 64")
    {
        dpcm.noteOn(); // no sample, won't play
        REQUIRE(dpcm.output() == 64);
    }

    SECTION("delta modulation: all-1 bits increase output")
    {
        std::vector<uint8_t> sample = { 0xFF, 0xFF, 0xFF, 0xFF }; // all 1s -> +2 per bit
        dpcm.loadSample(sample);
        dpcm.setRateIndex(15, true); // fastest rate
        dpcm.noteOn();
        // Process enough samples to consume some bits
        test::processN(dpcm, 5000);
        // Output should have increased from 64 toward 126
        REQUIRE(dpcm.output() > 64);
    }

    SECTION("delta modulation: all-0 bits decrease output")
    {
        std::vector<uint8_t> sample = { 0x00, 0x00, 0x00, 0x00 }; // all 0s -> -2 per bit
        dpcm.loadSample(sample);
        dpcm.setRateIndex(15, true);
        dpcm.noteOn();
        test::processN(dpcm, 5000);
        REQUIRE(dpcm.output() < 64);
    }

    SECTION("playback stops at end of sample without loop")
    {
        std::vector<uint8_t> sample = { 0xFF };
        dpcm.loadSample(sample);
        dpcm.setRateIndex(15, true);
        dpcm.setLoop(false);
        dpcm.noteOn();
        // Process enough to exhaust the 1-byte sample
        test::processN(dpcm, 10000);
        REQUIRE_FALSE(dpcm.isActive());
    }

    SECTION("loop mode restarts sample")
    {
        std::vector<uint8_t> sample = { 0xFF };
        dpcm.loadSample(sample);
        dpcm.setRateIndex(15, true);
        dpcm.setLoop(true);
        dpcm.noteOn();
        // Process lots of samples -- should keep playing
        test::processN(dpcm, 50000);
        REQUIRE(dpcm.isActive());
    }

    SECTION("noteOff stops playback")
    {
        std::vector<uint8_t> sample = { 0xFF, 0xFF };
        dpcm.loadSample(sample);
        dpcm.setRateIndex(0, true);
        dpcm.noteOn();
        dpcm.noteOff();
        REQUIRE_FALSE(dpcm.isActive());
    }

    SECTION("setEnabled false stops playback")
    {
        std::vector<uint8_t> sample = { 0xFF };
        dpcm.loadSample(sample);
        dpcm.noteOn();
        REQUIRE(dpcm.isActive());
        dpcm.setEnabled(false);
        REQUIRE_FALSE(dpcm.isActive());
    }

    SECTION("reset restores output to 64")
    {
        std::vector<uint8_t> sample = { 0xFF, 0xFF };
        dpcm.loadSample(sample);
        dpcm.setRateIndex(15, true);
        dpcm.noteOn();
        test::processN(dpcm, 5000);
        dpcm.reset();
        REQUIRE(dpcm.output() == 64);
    }
}
