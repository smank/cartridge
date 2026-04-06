#include <catch2/catch_test_macros.hpp>
#include "midi/Arpeggiator.h"

TEST_CASE("Arpeggiator", "[midi]")
{
    cart::Arpeggiator arp;
    arp.setSampleRate(44100.0);
    arp.setEnabled(true);
    arp.setRateHz(10.0f);
    arp.setOctaves(1);
    arp.setGateLength(0.8f);

    SECTION("no output without held notes")
    {
        int note = -1;
        for (int i = 0; i < 44100; ++i)
        {
            auto event = arp.process(note);
            REQUIRE(event == cart::ArpEvent::Nothing);
        }
    }

    SECTION("produces notes after noteOn")
    {
        arp.noteOn(60);
        int note = -1;
        bool gotNote = false;
        for (int i = 0; i < 44100; ++i) // 1 second at 10 Hz = ~10 notes
        {
            auto event = arp.process(note);
            if (event == cart::ArpEvent::NoteOn)
            {
                gotNote = true;
                REQUIRE(note == 60);
            }
        }
        REQUIRE(gotNote);
    }

    SECTION("Up pattern plays sorted ascending")
    {
        arp.setPattern(cart::ArpPattern::Up);
        arp.noteOn(64);
        arp.noteOn(60);
        arp.noteOn(67);

        std::vector<int> notes;
        int note = -1;
        for (int i = 0; i < 44100; ++i)
        {
            auto event = arp.process(note);
            if (event == cart::ArpEvent::NoteOn)
                notes.push_back(note);
        }

        // Should cycle through 60, 64, 67 in ascending order
        REQUIRE(notes.size() >= 3);
        // First 3 notes should be sorted ascending
        REQUIRE(notes[0] == 60);
        REQUIRE(notes[1] == 64);
        REQUIRE(notes[2] == 67);
    }

    SECTION("Down pattern plays sorted descending")
    {
        arp.setPattern(cart::ArpPattern::Down);
        arp.noteOn(60);
        arp.noteOn(64);
        arp.noteOn(67);

        std::vector<int> notes;
        int note = -1;
        for (int i = 0; i < 44100; ++i)
        {
            auto event = arp.process(note);
            if (event == cart::ArpEvent::NoteOn)
                notes.push_back(note);
        }

        REQUIRE(notes.size() >= 3);
        REQUIRE(notes[0] == 67);
        REQUIRE(notes[1] == 64);
        REQUIRE(notes[2] == 60);
    }

    SECTION("octaves expand sequence")
    {
        arp.setPattern(cart::ArpPattern::Up);
        arp.setOctaves(2);
        arp.noteOn(60);

        std::vector<int> notes;
        int note = -1;
        for (int i = 0; i < 44100; ++i)
        {
            auto event = arp.process(note);
            if (event == cart::ArpEvent::NoteOn)
                notes.push_back(note);
        }

        // Should have notes at 60 and 72 (one octave up)
        bool has60 = false, has72 = false;
        for (int n : notes) {
            if (n == 60) has60 = true;
            if (n == 72) has72 = true;
        }
        REQUIRE(has60);
        REQUIRE(has72);
    }

    SECTION("gate off event fires before next note")
    {
        arp.setGateLength(0.5f); // 50% gate
        arp.noteOn(60);

        bool gotGateOff = false;
        int note = -1;
        for (int i = 0; i < 44100; ++i)
        {
            auto event = arp.process(note);
            if (event == cart::ArpEvent::GateOff)
                gotGateOff = true;
        }
        REQUIRE(gotGateOff);
    }

    SECTION("noteOff removes note from sequence")
    {
        arp.noteOn(60);
        arp.noteOn(64);
        arp.noteOff(60);

        int note = -1;
        std::vector<int> notes;
        for (int i = 0; i < 44100; ++i)
        {
            auto event = arp.process(note);
            if (event == cart::ArpEvent::NoteOn)
                notes.push_back(note);
        }

        // Only 64 should remain
        for (int n : notes)
            REQUIRE(n == 64);
    }

    SECTION("all notes off clears sequence")
    {
        arp.noteOn(60);
        arp.noteOff(60);

        int note = -1;
        for (int i = 0; i < 4410; ++i)
        {
            auto event = arp.process(note);
            REQUIRE(event == cart::ArpEvent::Nothing);
        }
    }

    SECTION("reset clears state")
    {
        arp.noteOn(60);
        arp.reset();

        int note = -1;
        for (int i = 0; i < 4410; ++i)
        {
            auto event = arp.process(note);
            REQUIRE(event == cart::ArpEvent::Nothing);
        }
    }

    SECTION("disabled arpeggiator produces nothing")
    {
        arp.setEnabled(false);
        arp.noteOn(60);

        int note = -1;
        for (int i = 0; i < 4410; ++i)
        {
            auto event = arp.process(note);
            REQUIRE(event == cart::ArpEvent::Nothing);
        }
    }

    SECTION("random pattern produces notes from held set")
    {
        arp.setPattern(cart::ArpPattern::Random);
        arp.noteOn(60);
        arp.noteOn(64);
        arp.noteOn(67);

        std::set<int> receivedNotes;
        int note = -1;
        for (int i = 0; i < 44100 * 2; ++i) // 2 seconds
        {
            auto event = arp.process(note);
            if (event == cart::ArpEvent::NoteOn)
                receivedNotes.insert(note);
        }

        // Should have received all three notes
        REQUIRE(receivedNotes.count(60) > 0);
        REQUIRE(receivedNotes.count(64) > 0);
        REQUIRE(receivedNotes.count(67) > 0);
    }
}
