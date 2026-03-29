#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>
#include <algorithm>

namespace cart {

enum class ArpPattern { Up, Down, UpDown, Random, AsPlayed };
enum class ArpEvent  { Nothing, NoteOn, GateOff };

class Arpeggiator
{
public:
    void setEnabled (bool en) { enabled = en; }
    bool isEnabled() const { return enabled; }

    void setPattern (ArpPattern p) { pattern = p; }
    void setRateHz (float hz) { rateHz = hz; }
    void setOctaves (int oct) { octaves = std::clamp (oct, 1, 4); }
    void setSampleRate (double sr) { sampleRate = sr; }
    void setGateLength (float g) { gateLength = std::clamp (g, 0.1f, 1.0f); }

    /// Call per-sample. Returns ArpEvent::NoteOn when a new note triggers,
    /// ArpEvent::GateOff when the current note should be cut.
    ArpEvent process (int& outNote);

    void noteOn (int note, juce::uint8 vel = 127);
    void noteOff (int note);
    void reset();

    juce::uint8 getLastVelocity() const { return lastVelocity; }

private:
    void buildSequence();
    int getNextNote();

    bool enabled = false;
    ArpPattern pattern = ArpPattern::Up;
    float rateHz = 8.0f;
    int octaves = 1;
    double sampleRate = 44100.0;
    float gateLength = 0.8f;

    std::vector<int> heldNotes;
    std::vector<int> sequence;
    int sequenceIndex = 0;
    bool goingUp = true;

    double phase = 0.0;
    double gatePhase = 0.0;
    bool gateOff = false;
    int lastOutputNote = -1;
    juce::uint8 lastVelocity = 127;
};

} // namespace cart
