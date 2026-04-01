#include "ModernVoiceManager.h"
#include "NoteFrequencyTable.h"
#include <cmath>

namespace cart {

float ModernVoiceManager::getFrequencyForNote (int note) const
{
    float adjustedNote = static_cast<float> (note)
                       + pitchBendValue * static_cast<float> (pitchBendSemitones)
                       + (masterTuneCents / 100.0f);
    return midiNoteToFrequency (adjustedNote);
}

void ModernVoiceManager::processMidiMessage (const juce::MidiMessage& msg)
{
    if (enginePtr == nullptr)
        return;

    if (msg.isNoteOn())
    {
        handleNoteOn (msg.getNoteNumber(), msg.getFloatVelocity());
    }
    else if (msg.isNoteOff())
    {
        handleNoteOff (msg.getNoteNumber());
    }
    else if (msg.isPitchWheel())
    {
        handlePitchBend (msg.getPitchWheelValue());
    }
    else if (msg.isController())
    {
        if (onControlChange)
            onControlChange (msg.getControllerNumber(),
                             msg.getControllerValue() / 127.0f);
    }
    else if (msg.isAllNotesOff() || msg.isAllSoundOff())
    {
        handleAllNotesOff();
    }
}

void ModernVoiceManager::handleNoteOn (int note, float velocity)
{
    // Apply velocity sensitivity
    float vol = 1.0f - velocitySens + velocitySens * velocity;
    lastVelocity = velocity;

    float freq = getFrequencyForNote (note);
    enginePtr->noteOn (note, vol, freq);
}

void ModernVoiceManager::handleNoteOff (int note)
{
    enginePtr->noteOff (note);
}

void ModernVoiceManager::handlePitchBend (int bendValue)
{
    // MIDI pitch bend: 0-16383, center at 8192
    pitchBendValue = (static_cast<float> (bendValue) - 8192.0f) / 8192.0f;

    // Update pitch bend multiplier on the engine
    float bendSemitones = pitchBendValue * static_cast<float> (pitchBendSemitones);
    float mult = std::pow (2.0f, bendSemitones / 12.0f);
    enginePtr->setPitchBendMultiplier (mult);
}

void ModernVoiceManager::applyPitchMultiplier (float mult)
{
    if (enginePtr != nullptr)
        enginePtr->applyPitchMultiplier (mult);
}

void ModernVoiceManager::handleAllNotesOff()
{
    if (enginePtr != nullptr)
        enginePtr->allNotesOff();
}

} // namespace cart
