#include "ModernEngine.h"
#include <cmath>
#include <algorithm>

namespace cart {

void ModernEngine::setSampleRate (double sr)
{
    sampleRate = sr;
    for (auto& v : voices)
        v.setSampleRate (sr);
}

void ModernEngine::reset()
{
    for (auto& v : voices)
        v.reset();
    for (auto& ch : voiceMidiChannel)
        ch = 0;
    pitchBendMult = 1.0f;
}

void ModernEngine::setMaxVoices (int n)
{
    maxVoices = std::clamp (n, 1, maxVoiceCount);
}

void ModernEngine::setWaveform (NesWaveform w)
{
    cachedWaveform = w;
    for (int i = 0; i < maxVoices; ++i)
        voices[i].setWaveform (w);
}

void ModernEngine::setWaveformB (NesWaveform w)
{
    cachedWaveformB = w;
    for (int i = 0; i < maxVoices; ++i)
        voices[i].setWaveformB (w);
}

void ModernEngine::setOscAEnabled (bool en)
{
    cachedOscAEn = en;
    for (int i = 0; i < maxVoices; ++i)
        voices[i].setOscAEnabled (en);
}

void ModernEngine::setOscBEnabled (bool en)
{
    cachedOscBEn = en;
    for (int i = 0; i < maxVoices; ++i)
        voices[i].setOscBEnabled (en);
}

void ModernEngine::setOscBLevel (float level)
{
    cachedOscBLevel = level;
    for (int i = 0; i < maxVoices; ++i)
        voices[i].setOscBLevel (level);
}

void ModernEngine::setOscBDetune (float semitones)
{
    cachedOscBDetune = semitones;
    for (int i = 0; i < maxVoices; ++i)
        voices[i].setOscBDetune (semitones);
}

void ModernEngine::setUnisonCount (int n)
{
    cachedUnison = std::clamp (n, 1, 7);
    for (int i = 0; i < maxVoices; ++i)
        voices[i].setUnisonCount (cachedUnison);
}

void ModernEngine::setUnisonDetune (float cents)
{
    cachedDetune = cents;
    for (int i = 0; i < maxVoices; ++i)
        voices[i].setUnisonDetune (cents);
}

void ModernEngine::setAdsr (float a, float d, float s, float r)
{
    cachedAttack = a;
    cachedDecay = d;
    cachedSustain = s;
    cachedRelease = r;
    for (int i = 0; i < maxVoices; ++i)
        voices[i].setAdsr (a, d, s, r);
}

void ModernEngine::setPortamento (bool en, float time)
{
    cachedPortaEn = en;
    cachedPortaTime = time;
    for (int i = 0; i < maxVoices; ++i)
        voices[i].setPortamento (en, time);
}

void ModernEngine::setMasterVolume (float vol)
{
    masterVolume = vol;
}

int ModernEngine::noteOn (int note, float velocity, float freqHz)
{
    // Apply pitch bend to frequency
    float finalFreq = freqHz * pitchBendMult;

    // 1. If note already active, retrigger same voice
    int idx = findVoiceForNote (note);
    if (idx >= 0)
    {
        voices[idx].noteOn (note, velocity, finalFreq);
        return idx;
    }

    // 2. Find a free voice
    idx = findFreeVoice();
    if (idx >= 0)
    {
        voices[idx].noteOn (note, velocity, finalFreq);
        return idx;
    }

    // 3. Steal oldest releasing voice
    idx = findOldestReleasingVoice();
    if (idx >= 0)
    {
        voices[idx].noteOn (note, velocity, finalFreq);
        return idx;
    }

    // 4. Steal oldest active voice
    idx = findOldestActiveVoice();
    if (idx >= 0)
    {
        voices[idx].noteOn (note, velocity, finalFreq);
    }
    return idx;
}

void ModernEngine::noteOff (int note)
{
    for (int i = 0; i < maxVoices; ++i)
    {
        if (voices[i].isActive() && voices[i].getNote() == note)
            voices[i].noteOff();
    }
}

void ModernEngine::noteOffByIndex (int voiceIndex)
{
    if (voiceIndex >= 0 && voiceIndex < maxVoiceCount && voices[voiceIndex].isActive())
        voices[voiceIndex].noteOff();
}

void ModernEngine::allNotesOff()
{
    for (auto& v : voices)
    {
        if (v.isActive())
            v.noteOff();
    }
}

float ModernEngine::process()
{
    float sum = 0.0f;
    int activeCount = 0;

    for (int i = 0; i < maxVoices; ++i)
    {
        if (voices[i].isActive())
        {
            sum += voices[i].process();
            ++activeCount;
        }
    }

    // Normalize by sqrt(maxVoices) for consistent output level
    if (activeCount > 0)
        sum /= std::sqrt (static_cast<float> (maxVoices));

    return sum * masterVolume;
}

void ModernEngine::setPitchBendMultiplier (float mult)
{
    pitchBendMult = mult;
}

void ModernEngine::applyPitchMultiplier (float mult)
{
    // Temporarily modify all active voice frequencies for LFO vibrato
    // This is called per-sample, so it's a lightweight modulation
    // The voices store their base frequency and this just shifts it
    // We don't need to store this — LFO calls it every sample
    (void) mult;
    // LFO pitch modulation is handled differently in Modern mode:
    // The ModernVoiceManager tracks the multiplier and reapplies frequencies
}

int ModernEngine::findVoiceForNote (int note) const
{
    for (int i = 0; i < maxVoices; ++i)
    {
        if (voices[i].isActive() && !voices[i].isReleasing() && voices[i].getNote() == note)
            return i;
    }
    return -1;
}

int ModernEngine::findFreeVoice() const
{
    for (int i = 0; i < maxVoices; ++i)
    {
        if (!voices[i].isActive())
            return i;
    }
    return -1;
}

int ModernEngine::findOldestReleasingVoice() const
{
    int oldest = -1;
    int maxAge = -1;

    for (int i = 0; i < maxVoices; ++i)
    {
        if (voices[i].isReleasing() && voices[i].getAge() > maxAge)
        {
            maxAge = voices[i].getAge();
            oldest = i;
        }
    }
    return oldest;
}

int ModernEngine::findOldestActiveVoice() const
{
    int oldest = -1;
    int maxAge = -1;

    for (int i = 0; i < maxVoices; ++i)
    {
        if (voices[i].isActive() && voices[i].getAge() > maxAge)
        {
            maxAge = voices[i].getAge();
            oldest = i;
        }
    }
    return oldest;
}

// --- MPE per-voice methods ---

void ModernEngine::setPerVoicePitchBend (int voiceIndex, float semitones)
{
    if (voiceIndex >= 0 && voiceIndex < maxVoiceCount)
    {
        float mult = std::pow (2.0f, semitones / 12.0f);
        voices[voiceIndex].setPitchBendMultiplier (mult);
    }
}

void ModernEngine::setPerVoiceSlide (int voiceIndex, float value01)
{
    if (voiceIndex >= 0 && voiceIndex < maxVoiceCount)
    {
        voices[voiceIndex].setMpeSlide (value01);
        // TODO: Route slide to a filter cutoff or timbre parameter when available
    }
}

void ModernEngine::setPerVoicePressure (int voiceIndex, float value01)
{
    if (voiceIndex >= 0 && voiceIndex < maxVoiceCount)
    {
        voices[voiceIndex].setMpePressure (value01);
        // TODO: Route pressure to volume/expression modulation when available
    }
}

void ModernEngine::setVoiceMidiChannel (int voiceIndex, int midiChannel)
{
    if (voiceIndex >= 0 && voiceIndex < maxVoiceCount)
        voiceMidiChannel[voiceIndex] = midiChannel;
}

int ModernEngine::getVoiceMidiChannel (int voiceIndex) const
{
    if (voiceIndex >= 0 && voiceIndex < maxVoiceCount)
        return voiceMidiChannel[voiceIndex];
    return 0;
}

int ModernEngine::findVoiceForChannel (int midiChannel) const
{
    for (int i = 0; i < maxVoices; ++i)
    {
        if (voices[i].isActive() && !voices[i].isReleasing()
            && voiceMidiChannel[i] == midiChannel)
            return i;
    }
    return -1;
}

int ModernEngine::findVoiceForNoteAndChannel (int note, int midiChannel) const
{
    for (int i = 0; i < maxVoices; ++i)
    {
        if (voices[i].isActive() && voices[i].getNote() == note
            && voiceMidiChannel[i] == midiChannel)
            return i;
    }
    return -1;
}

} // namespace cart
