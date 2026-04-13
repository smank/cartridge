#include "ModernVoiceManager.h"
#include "NoteFrequencyTable.h"
#include <cmath>

namespace cart {

float ModernVoiceManager::getFrequencyForNote (int note) const
{
    float adjustedNote = static_cast<float> (note)
                       + pitchBendValue * static_cast<float> (pitchBendSemitones)
                       + (masterTuneCents / 100.0f);
    if (tuningTablePtr != nullptr)
        return tuningTablePtr->getFrequency (adjustedNote);
    return midiNoteToFrequency (adjustedNote);
}

float ModernVoiceManager::getFrequencyForNoteWithBend (int note, float bendSemitones) const
{
    float adjustedNote = static_cast<float> (note)
                       + bendSemitones
                       + (masterTuneCents / 100.0f);
    if (tuningTablePtr != nullptr)
        return tuningTablePtr->getFrequency (adjustedNote);
    return midiNoteToFrequency (adjustedNote);
}

void ModernVoiceManager::processMidiMessage (const juce::MidiMessage& msg)
{
    if (enginePtr == nullptr)
        return;

    int channel = msg.getChannel(); // 1-based

    if (msg.isNoteOn())
    {
        handleNoteOn (msg.getNoteNumber(), msg.getFloatVelocity(), channel);
    }
    else if (msg.isNoteOff())
    {
        handleNoteOff (msg.getNoteNumber(), channel);
    }
    else if (msg.isPitchWheel())
    {
        handlePitchBend (msg.getPitchWheelValue(), channel);
    }
    else if (msg.isController())
    {
        int cc = msg.getControllerNumber();
        float value01 = msg.getControllerValue() / 127.0f;

        // RPN tracking for MPE Configuration Message detection
        if (cc == 101) // RPN MSB
        {
            rpnMsb = msg.getControllerValue();
        }
        else if (cc == 100) // RPN LSB
        {
            rpnLsb = msg.getControllerValue();
        }
        else if (cc == 6 && rpnMsb == 0 && rpnLsb == 6 && channel == 1)
        {
            // MPE Configuration Message (MCM): RPN 0x0006 on channel 1
            int zoneSize = msg.getControllerValue();
            mpeEnabled = (zoneSize > 0);
            mpeMemberChannels = zoneSize;
            rpnMsb = rpnLsb = -1; // Reset RPN state after MCM

            // Reset per-voice bend state when MPE config changes
            for (auto& b : perVoiceBendSemitones)
                b = 0.0f;
        }
        else if (cc == 74 && mpeEnabled && channel > 1)
        {
            // MPE Slide (CC74) -- route to voice on this member channel
            handleMpeSlide (channel, value01);
        }
        else
        {
            if (onControlChange)
                onControlChange (ccContext, cc, value01);
        }
    }
    else if (msg.isChannelPressure())
    {
        if (mpeEnabled && channel > 1)
        {
            // MPE per-note pressure -- route to voice on this member channel
            handleMpePressure (channel, msg.getChannelPressureValue() / 127.0f);
        }
        // Non-MPE channel pressure is not currently routed
    }
    else if (msg.isAllNotesOff() || msg.isAllSoundOff())
    {
        handleAllNotesOff();
    }
}

void ModernVoiceManager::handleNoteOn (int note, float velocity, int midiChannel)
{
    // Apply velocity sensitivity
    float vol = 1.0f - velocitySens + velocitySens * velocity;
    lastVelocity = velocity;

    float freq = getFrequencyForNote (note);
    int voiceIdx = enginePtr->noteOn (note, vol, freq);

    // In MPE mode, tag the voice with its MIDI channel for per-note routing
    if (mpeEnabled && midiChannel > 1 && voiceIdx >= 0)
    {
        enginePtr->setVoiceMidiChannel (voiceIdx, midiChannel);
    }

    if (onNoteGate) onNoteGate (gateContext,true);
}

void ModernVoiceManager::handleNoteOff (int note, int midiChannel)
{
    if (onNoteGate) onNoteGate (gateContext,false);

    if (mpeEnabled && midiChannel > 1)
    {
        // In MPE mode, release only the specific voice on this channel+note
        int idx = enginePtr->findVoiceForNoteAndChannel (note, midiChannel);
        if (idx >= 0)
        {
            enginePtr->noteOffByIndex (idx);
            enginePtr->setVoiceMidiChannel (idx, 0); // clear channel assignment
            return;
        }
    }

    // Non-MPE or global channel: release by note (existing behavior)
    enginePtr->noteOff (note);
}

void ModernVoiceManager::handlePitchBend (int bendValue, int midiChannel)
{
    // MIDI pitch bend: 0-16383, center at 8192
    float normalizedBend = (static_cast<float> (bendValue) - 8192.0f) / 8192.0f;
    float bendSemitones = normalizedBend * static_cast<float> (pitchBendSemitones);

    if (mpeEnabled && midiChannel > 1)
    {
        // MPE per-note pitch bend: apply only to the voice on this member channel
        int idx = enginePtr->findVoiceForChannel (midiChannel);
        if (idx >= 0)
        {
            enginePtr->setPerVoicePitchBend (idx, bendSemitones);
        }
        return;
    }

    // Non-MPE or global channel (1): apply to all voices (existing behavior)
    pitchBendValue = normalizedBend;
    float mult = std::pow (2.0f, bendSemitones / 12.0f);
    enginePtr->setPitchBendMultiplier (mult);
}

void ModernVoiceManager::handleMpeSlide (int midiChannel, float value01)
{
    int idx = enginePtr->findVoiceForChannel (midiChannel);
    if (idx >= 0)
    {
        enginePtr->setPerVoiceSlide (idx, value01);
    }
}

void ModernVoiceManager::handleMpePressure (int midiChannel, float value01)
{
    int idx = enginePtr->findVoiceForChannel (midiChannel);
    if (idx >= 0)
    {
        enginePtr->setPerVoicePressure (idx, value01);
    }
}

void ModernVoiceManager::applyPitchMultiplier (float mult)
{
    if (enginePtr != nullptr)
        enginePtr->applyPitchMultiplier (mult);
}

void ModernVoiceManager::handleAllNotesOff()
{
    if (onNoteGate) onNoteGate (gateContext,false);
    if (enginePtr != nullptr)
        enginePtr->allNotesOff();
}

} // namespace cart
