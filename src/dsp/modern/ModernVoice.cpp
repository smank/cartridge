#include "ModernVoice.h"
#include <cmath>
#include <algorithm>

namespace cart {

void ModernVoice::setSampleRate (double sr)
{
    sampleRate = sr;
    envelope.setSampleRate (sr);
    porta.setSampleRate (sr);

    for (auto& osc : oscillators)
        osc.setSampleRate (sr);
    for (auto& osc : oscillatorsB)
        osc.setSampleRate (sr);
}

void ModernVoice::setWaveform (NesWaveform w)
{
    waveform = w;
    for (auto& osc : oscillators)
        osc.setWaveform (w);
}

void ModernVoice::setWaveformB (NesWaveform w)
{
    waveformB = w;
    for (auto& osc : oscillatorsB)
        osc.setWaveform (w);
}

void ModernVoice::setOscAEnabled (bool en)  { oscAEnabled = en; }
void ModernVoice::setOscBEnabled (bool en)  { oscBEnabled = en; }
void ModernVoice::setOscBLevel (float level) { oscBLevel = level; }
void ModernVoice::setOscBDetune (float semitones) { oscBDetuneSemi = semitones; }

void ModernVoice::setUnisonCount (int count)
{
    unisonCount = std::clamp (count, 1, maxUnison);
}

void ModernVoice::setUnisonDetune (float cents)
{
    unisonDetune = cents;
}

void ModernVoice::setAdsr (float a, float d, float s, float r)
{
    envelope.setAttack (a);
    envelope.setDecay (d);
    envelope.setSustain (s);
    envelope.setRelease (r);
}

void ModernVoice::setPortamento (bool en, float time)
{
    porta.setEnabled (en);
    porta.setTime (time);
}

void ModernVoice::noteOn (int note, float velocity, float freqHz)
{
    currentNote = note;
    velocityGain = velocity;
    baseFreq = freqHz;
    age = 0;

    porta.setTarget (freqHz);

    // If portamento is enabled and voice was already active, glide to new freq
    // Otherwise set frequency immediately
    if (!porta.isGliding() || !isActive())
    {
        // First note or portamento disabled: set freq directly
        for (int i = 0; i < unisonCount; ++i)
        {
            oscillators[i].setWaveform (waveform);
            oscillators[i].noteOn();
            oscillatorsB[i].setWaveform (waveformB);
            oscillatorsB[i].noteOn();
        }
    }
    else
    {
        // Portamento glide: don't reset oscillator phase
        for (int i = 0; i < unisonCount; ++i)
        {
            oscillators[i].setWaveform (waveform);
            oscillatorsB[i].setWaveform (waveformB);
        }
    }

    updateOscFrequencies();
    envelope.noteOn();
}

void ModernVoice::noteOff()
{
    envelope.noteOff();
}

float ModernVoice::process()
{
    if (envelope.isFinished())
        return 0.0f;

    ++age;

    // Update frequency if portamento is gliding
    if (porta.isGliding())
    {
        baseFreq = porta.process();
        updateOscFrequencies();
    }

    float unisonNorm = (unisonCount > 1)
        ? 1.0f / std::sqrt (static_cast<float> (unisonCount))
        : 1.0f;

    float mixed = 0.0f;

    // Osc A
    if (oscAEnabled)
    {
        float sumA = 0.0f;
        for (int i = 0; i < unisonCount; ++i)
            sumA += oscillators[i].process();
        mixed += sumA * unisonNorm;
    }

    // Osc B
    if (oscBEnabled)
    {
        float sumB = 0.0f;
        for (int i = 0; i < unisonCount; ++i)
            sumB += oscillatorsB[i].process();
        mixed += sumB * unisonNorm * oscBLevel;
    }

    // Apply envelope and velocity
    float env = envelope.process();
    return mixed * env * velocityGain;
}

void ModernVoice::reset()
{
    envelope.reset();
    porta.reset();
    currentNote = -1;
    age = 0;
    velocityGain = 1.0f;
    perVoicePitchBend = 1.0f;
    mpeSlide = 0.5f;
    mpePressure = 0.0f;

    for (auto& osc : oscillators)
        osc.reset();
    for (auto& osc : oscillatorsB)
        osc.reset();
}

void ModernVoice::updateOscFrequencies()
{
    // Apply per-voice MPE pitch bend to base frequency
    float bentFreq = baseFreq * perVoicePitchBend;

    // Osc B base frequency with semitone offset
    float freqB = bentFreq * std::pow (2.0f, oscBDetuneSemi / 12.0f);

    if (unisonCount == 1)
    {
        oscillators[0].setFrequency (bentFreq);
        oscillatorsB[0].setFrequency (freqB);
        return;
    }

    // Spread N oscillators symmetrically from -S/2 to +S/2 cents
    for (int i = 0; i < unisonCount; ++i)
    {
        float offset = (static_cast<float> (i) - static_cast<float> (unisonCount - 1) / 2.0f)
                      * (unisonDetune / std::max (static_cast<float> (unisonCount - 1), 1.0f));
        float freqA = bentFreq * std::pow (2.0f, offset / 1200.0f);
        oscillators[i].setFrequency (freqA);
        oscillatorsB[i].setFrequency (freqB * std::pow (2.0f, offset / 1200.0f));
    }
}

} // namespace cart
