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

void ModernEngine::noteOn (int note, float velocity, float freqHz)
{
    // Apply pitch bend to frequency
    float finalFreq = freqHz * pitchBendMult;

    // 1. If note already active, retrigger same voice
    int idx = findVoiceForNote (note);
    if (idx >= 0)
    {
        voices[idx].noteOn (note, velocity, finalFreq);
        return;
    }

    // 2. Find a free voice
    idx = findFreeVoice();
    if (idx >= 0)
    {
        voices[idx].noteOn (note, velocity, finalFreq);
        return;
    }

    // 3. Steal oldest releasing voice
    idx = findOldestReleasingVoice();
    if (idx >= 0)
    {
        voices[idx].noteOn (note, velocity, finalFreq);
        return;
    }

    // 4. Steal oldest active voice
    idx = findOldestActiveVoice();
    if (idx >= 0)
    {
        voices[idx].noteOn (note, velocity, finalFreq);
    }
}

void ModernEngine::noteOff (int note)
{
    for (int i = 0; i < maxVoices; ++i)
    {
        if (voices[i].isActive() && voices[i].getNote() == note)
            voices[i].noteOff();
    }
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

} // namespace cart
