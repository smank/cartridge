#pragma once
#include <cmath>
#include <cstdint>

namespace cart {

class TuningTable
{
public:
    TuningTable() { makeEqual(); }

    // Factory methods — build frequency table for MIDI notes 0-127
    void makeEqual()
    {
        for (int i = 0; i < 128; ++i)
            frequencies[i] = 440.0f * std::pow(2.0f, (static_cast<float>(i) - 69.0f) / 12.0f);
    }

    void makeJust(int rootNote = 60)
    {
        // Just intonation ratios for 12 notes relative to root
        static constexpr float ratios[12] = {
            1.0f,          // Unison
            16.0f/15.0f,   // Minor 2nd
            9.0f/8.0f,     // Major 2nd
            6.0f/5.0f,     // Minor 3rd
            5.0f/4.0f,     // Major 3rd
            4.0f/3.0f,     // Perfect 4th
            45.0f/32.0f,   // Tritone
            3.0f/2.0f,     // Perfect 5th
            8.0f/5.0f,     // Minor 6th
            5.0f/3.0f,     // Major 6th
            9.0f/5.0f,     // Minor 7th (just)
            15.0f/8.0f     // Major 7th
        };

        float rootFreq = 440.0f * std::pow(2.0f, (static_cast<float>(rootNote) - 69.0f) / 12.0f);

        for (int i = 0; i < 128; ++i)
        {
            int octaveDiff = (i - rootNote) / 12;
            int noteInScale = ((i - rootNote) % 12 + 12) % 12;

            // Handle negative octave differences correctly
            if (i < rootNote && (i - rootNote) % 12 != 0)
                octaveDiff -= 1;

            frequencies[i] = rootFreq * ratios[noteInScale] * std::pow(2.0f, static_cast<float>(octaveDiff));
        }
    }

    void makePythagorean(int rootNote = 60)
    {
        // Pythagorean ratios: built from perfect fifths (3:2)
        static constexpr float ratios[12] = {
            1.0f,            // Unison
            256.0f/243.0f,   // Minor 2nd
            9.0f/8.0f,       // Major 2nd
            32.0f/27.0f,     // Minor 3rd
            81.0f/64.0f,     // Major 3rd (ditone)
            4.0f/3.0f,       // Perfect 4th
            729.0f/512.0f,   // Tritone
            3.0f/2.0f,       // Perfect 5th
            128.0f/81.0f,    // Minor 6th
            27.0f/16.0f,     // Major 6th
            16.0f/9.0f,      // Minor 7th
            243.0f/128.0f    // Major 7th
        };

        float rootFreq = 440.0f * std::pow(2.0f, (static_cast<float>(rootNote) - 69.0f) / 12.0f);

        for (int i = 0; i < 128; ++i)
        {
            int octaveDiff = (i - rootNote) / 12;
            int noteInScale = ((i - rootNote) % 12 + 12) % 12;
            if (i < rootNote && (i - rootNote) % 12 != 0)
                octaveDiff -= 1;
            frequencies[i] = rootFreq * ratios[noteInScale] * std::pow(2.0f, static_cast<float>(octaveDiff));
        }
    }

    void makeMeantone(int rootNote = 60)
    {
        // Quarter-comma meantone temperament
        // Fifth = 5^(1/4) approximately 1.49535
        float fifth = std::pow(5.0f, 0.25f);
        float rootFreq = 440.0f * std::pow(2.0f, (static_cast<float>(rootNote) - 69.0f) / 12.0f);

        // Generate 12 notes by stacking meantone fifths from F to B
        // Circle of fifths order: F C G D A E B F# C# G# D# A#
        // Map to chromatic: C=0, C#=1, D=2, etc.
        float notes[12];
        notes[0] = 1.0f; // C (root of generation)

        // Going up by fifths: C -> G -> D -> A -> E -> B -> F#
        float current = 1.0f;
        int fifthOrder[] = { 7, 2, 9, 4, 11, 6 }; // G, D, A, E, B, F#
        for (int idx : fifthOrder)
        {
            current *= fifth;
            while (current >= 2.0f) current /= 2.0f;
            notes[idx] = current;
        }

        // Going down by fifths: C -> F -> Bb -> Eb -> Ab -> Db
        current = 1.0f;
        int fourthOrder[] = { 5, 10, 3, 8, 1 }; // F, Bb, Eb, Ab, Db
        for (int idx : fourthOrder)
        {
            current /= fifth;
            while (current < 1.0f) current *= 2.0f;
            notes[idx] = current;
        }

        for (int i = 0; i < 128; ++i)
        {
            int octaveDiff = (i - rootNote) / 12;
            int noteInScale = ((i - rootNote) % 12 + 12) % 12;
            if (i < rootNote && (i - rootNote) % 12 != 0)
                octaveDiff -= 1;
            frequencies[i] = rootFreq * notes[noteInScale] * std::pow(2.0f, static_cast<float>(octaveDiff));
        }
    }

    /// Get frequency for a potentially fractional MIDI note (for pitch bend interpolation)
    float getFrequency(float midiNote) const
    {
        if (midiNote <= 0.0f) return frequencies[0];
        if (midiNote >= 127.0f) return frequencies[127];

        int lower = static_cast<int>(midiNote);
        int upper = lower + 1;
        if (upper > 127) upper = 127;

        float frac = midiNote - static_cast<float>(lower);
        // Linear interpolation in frequency domain
        return frequencies[lower] + frac * (frequencies[upper] - frequencies[lower]);
    }

    float frequencies[128] = {};
};

} // namespace cart
