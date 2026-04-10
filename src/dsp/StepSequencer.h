#pragma once

#include <cmath>
#include <atomic>

namespace cart {

struct StepSequenceData
{
    static constexpr int kMaxSteps = 16;

    int  volumeSteps[kMaxSteps]  = { 15,15,15,15, 15,15,15,15, 15,15,15,15, 15,15,15,15 };
    int  pitchSteps[kMaxSteps]   = {};   // semitone offsets, -12 to +12
    int  dutySteps[kMaxSteps]    = {};   // 0-3 for NES pulse, 0-7 for VRC6 pulse

    int  numVolumeSteps = 0;   // 0 = lane disabled
    int  numPitchSteps  = 0;
    int  numDutySteps   = 0;

    bool volumeLoop = false;
    bool pitchLoop  = false;
    bool dutyLoop   = false;
};

class StepSequencer
{
public:
    void setSampleRate (double sr) { sampleRate = sr; }
    void setRateHz (float hz)      { rateHz = hz; }
    void setEnabled (bool en)      { enabled = en; }
    void setData (const StepSequenceData* d) { seqData = d; }

    void trigger()
    {
        phase = 0.0;
        volIndex = 0;  pitchIndex = 0;  dutyIndex = 0;
        volDone = false; pitchDone = false; dutyDone = false;
        gateActive = true;
        firstTick = true;
    }

    void release()
    {
        gateActive = false;
    }

    struct Modulation
    {
        int  volumeOverride = -1;  // -1 = no override
        int  pitchOffset    = 0;
        int  dutyOverride   = -1;  // -1 = no override
    };

    Modulation process()
    {
        Modulation mod;
        if (!enabled || seqData == nullptr || !gateActive)
            return mod;

        // On first tick after trigger, output step 0 immediately
        if (!firstTick)
        {
            phase += rateHz / sampleRate;
            if (phase >= 1.0)
            {
                phase -= 1.0;
                advanceStep (volIndex,   seqData->numVolumeSteps, seqData->volumeLoop, volDone);
                advanceStep (pitchIndex, seqData->numPitchSteps,  seqData->pitchLoop,  pitchDone);
                advanceStep (dutyIndex,  seqData->numDutySteps,   seqData->dutyLoop,   dutyDone);
            }
        }
        firstTick = false;

        // Read current step values
        if (seqData->numVolumeSteps > 0 && !volDone)
            mod.volumeOverride = seqData->volumeSteps[volIndex];

        if (seqData->numPitchSteps > 0 && !pitchDone)
            mod.pitchOffset = seqData->pitchSteps[pitchIndex];

        if (seqData->numDutySteps > 0 && !dutyDone)
            mod.dutyOverride = seqData->dutySteps[dutyIndex];

        return mod;
    }

    void reset()
    {
        phase = 0.0;
        volIndex = pitchIndex = dutyIndex = 0;
        volDone = pitchDone = dutyDone = false;
        gateActive = false;
        firstTick = true;
    }

    int getCurrentVolIndex() const  { return volIndex; }
    int getCurrentPitchIndex() const { return pitchIndex; }
    int getCurrentDutyIndex() const  { return dutyIndex; }
    bool isActive() const { return enabled && gateActive; }

    /// Returns the step index of the first active lane (for UI playback indicator)
    int getActiveStepIndex() const
    {
        if (seqData == nullptr) return 0;
        if (seqData->numVolumeSteps > 0) return volIndex;
        if (seqData->numPitchSteps > 0)  return pitchIndex;
        if (seqData->numDutySteps > 0)   return dutyIndex;
        return 0;
    }

private:
    void advanceStep (int& index, int numSteps, bool loop, bool& done)
    {
        if (numSteps == 0 || done) return;
        ++index;
        if (index >= numSteps)
        {
            if (loop)
                index = 0;
            else
            {
                index = numSteps - 1;
                done = true;
            }
        }
    }

    bool enabled = false;
    double sampleRate = 44100.0;
    float rateHz = 8.0f;
    double phase = 0.0;

    int volIndex = 0, pitchIndex = 0, dutyIndex = 0;
    bool volDone = false, pitchDone = false, dutyDone = false;
    bool gateActive = false;
    bool firstTick = true;

    const StepSequenceData* seqData = nullptr;
};

} // namespace cart
