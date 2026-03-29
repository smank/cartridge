#include "Arpeggiator.h"
#include <random>

namespace cart {

void Arpeggiator::reset()
{
    heldNotes.clear();
    sequence.clear();
    sequenceIndex = 0;
    phase = 0.0;
    gatePhase = 0.0;
    gateOff = false;
    lastOutputNote = -1;
    goingUp = true;
}

void Arpeggiator::noteOn (int note, juce::uint8 vel)
{
    lastVelocity = vel;
    if (std::find (heldNotes.begin(), heldNotes.end(), note) == heldNotes.end())
    {
        heldNotes.push_back (note);
        buildSequence();
    }
}

void Arpeggiator::noteOff (int note)
{
    heldNotes.erase (std::remove (heldNotes.begin(), heldNotes.end(), note), heldNotes.end());
    if (heldNotes.empty())
    {
        sequence.clear();
        sequenceIndex = 0;
        lastOutputNote = -1;
        gateOff = false;
    }
    else
    {
        buildSequence();
    }
}

void Arpeggiator::buildSequence()
{
    sequence.clear();
    if (heldNotes.empty()) return;

    if (pattern == ArpPattern::AsPlayed)
    {
        // Use held order (not sorted) expanded with octaves
        for (int oct = 0; oct < octaves; ++oct)
            for (int n : heldNotes)
                sequence.push_back (n + oct * 12);
    }
    else
    {
        auto sorted = heldNotes;
        std::sort (sorted.begin(), sorted.end());

        for (int oct = 0; oct < octaves; ++oct)
            for (int n : sorted)
                sequence.push_back (n + oct * 12);
    }

    if (sequenceIndex >= static_cast<int> (sequence.size()))
        sequenceIndex = 0;
}

int Arpeggiator::getNextNote()
{
    if (sequence.empty()) return -1;

    int note = -1;

    switch (pattern)
    {
        case ArpPattern::Up:
        case ArpPattern::AsPlayed:
            note = sequence[static_cast<size_t> (sequenceIndex)];
            sequenceIndex = (sequenceIndex + 1) % static_cast<int> (sequence.size());
            break;

        case ArpPattern::Down:
            note = sequence[static_cast<size_t> (static_cast<int> (sequence.size()) - 1 - sequenceIndex)];
            sequenceIndex = (sequenceIndex + 1) % static_cast<int> (sequence.size());
            break;

        case ArpPattern::UpDown:
        {
            note = sequence[static_cast<size_t> (sequenceIndex)];
            if (sequence.size() <= 1) break;

            if (goingUp)
            {
                sequenceIndex++;
                if (sequenceIndex >= static_cast<int> (sequence.size()))
                {
                    sequenceIndex = static_cast<int> (sequence.size()) - 2;
                    goingUp = false;
                }
            }
            else
            {
                sequenceIndex--;
                if (sequenceIndex < 0)
                {
                    sequenceIndex = 1;
                    goingUp = true;
                }
            }
            break;
        }
        case ArpPattern::Random:
        {
            static std::mt19937 rng { std::random_device{}() };
            std::uniform_int_distribution<int> dist (0, static_cast<int> (sequence.size()) - 1);
            note = sequence[static_cast<size_t> (dist (rng))];
            break;
        }
    }

    return std::clamp (note, 0, 127);
}

ArpEvent Arpeggiator::process (int& outNote)
{
    if (!enabled || sequence.empty() || sampleRate <= 0.0)
        return ArpEvent::Nothing;

    double inc = rateHz / sampleRate;
    phase += inc;
    gatePhase += inc;

    if (phase >= 1.0)
    {
        phase -= 1.0;
        gatePhase = 0.0;
        gateOff = false;
        outNote = getNextNote();
        lastOutputNote = outNote;
        return outNote >= 0 ? ArpEvent::NoteOn : ArpEvent::Nothing;
    }

    // Gate off check: if we've passed the gate portion of this step
    if (!gateOff && gateLength < 1.0f && gatePhase >= static_cast<double> (gateLength))
    {
        gateOff = true;
        return ArpEvent::GateOff;
    }

    return ArpEvent::Nothing;
}

} // namespace cart
