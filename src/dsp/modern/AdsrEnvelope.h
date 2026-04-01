#pragma once

#include <cmath>

namespace cart {

/// Full ADSR envelope for the Modern polyphonic engine.
/// Attack is linear (punchy chiptune feel), Decay/Release are exponential.
class AdsrEnvelope
{
public:
    enum class State { Idle, Attack, Decay, Sustain, Release };

    void setSampleRate (double sr)
    {
        sampleRate = sr;
        updateCoefficients();
    }

    void setAttack (float seconds)
    {
        attackTime = seconds;
        updateCoefficients();
    }

    void setDecay (float seconds)
    {
        decayTime = seconds;
        updateCoefficients();
    }

    void setSustain (float lvl) { sustainLevel = lvl; }

    void setRelease (float seconds)
    {
        releaseTime = seconds;
        updateCoefficients();
    }

    /// Trigger attack from current level (no click on retrigger)
    void noteOn()
    {
        state = State::Attack;
    }

    /// Trigger release from current level
    void noteOff()
    {
        if (state != State::Idle)
            state = State::Release;
    }

    float process()
    {
        switch (state)
        {
            case State::Idle:
                return 0.0f;

            case State::Attack:
            {
                // Linear ramp up
                level += attackRate;
                if (level >= 1.0f)
                {
                    level = 1.0f;
                    state = State::Decay;
                }
                return level;
            }

            case State::Decay:
            {
                // Exponential decay toward sustain level
                level += (sustainLevel - level) * decayCoeff;
                // Snap when close
                if (std::abs (level - sustainLevel) < 0.001f)
                {
                    level = sustainLevel;
                    state = State::Sustain;
                }
                return level;
            }

            case State::Sustain:
                return level;

            case State::Release:
            {
                // Exponential decay toward 0
                level += (0.0f - level) * releaseCoeff;
                // Snap when close
                if (level < 0.001f)
                {
                    level = 0.0f;
                    state = State::Idle;
                }
                return level;
            }
        }

        return 0.0f;
    }

    bool isFinished() const { return state == State::Idle; }
    bool isReleasing() const { return state == State::Release; }
    State getState() const { return state; }

    void reset()
    {
        state = State::Idle;
        level = 0.0f;
    }

private:
    void updateCoefficients()
    {
        if (sampleRate <= 0.0)
            return;

        // Attack: linear ramp (samples = attackTime * sampleRate)
        float attackSamples = static_cast<float> (sampleRate) * attackTime;
        attackRate = (attackSamples > 0.0f) ? (1.0f / attackSamples) : 1.0f;

        // Decay: exponential coefficient
        if (decayTime > 0.0f)
            decayCoeff = 1.0f - std::exp (-1.0f / (static_cast<float> (sampleRate) * decayTime));
        else
            decayCoeff = 1.0f;

        // Release: exponential coefficient
        if (releaseTime > 0.0f)
            releaseCoeff = 1.0f - std::exp (-1.0f / (static_cast<float> (sampleRate) * releaseTime));
        else
            releaseCoeff = 1.0f;
    }

    State   state        = State::Idle;
    float   level        = 0.0f;
    double  sampleRate   = 44100.0;

    float   attackTime   = 0.005f;
    float   decayTime    = 0.1f;
    float   sustainLevel = 1.0f;
    float   releaseTime  = 0.1f;

    float   attackRate   = 0.0f;
    float   decayCoeff   = 0.0f;
    float   releaseCoeff = 0.0f;
};

} // namespace cart
