#include <catch2/catch_test_macros.hpp>
#include "dsp/StepSequencer.h"

using namespace cart;

// ─── Helpers ────────────────────────────────────────────────────────────────

static StepSequenceData makeVolumeData (std::initializer_list<int> vols, bool loop = false)
{
    StepSequenceData d;
    int i = 0;
    for (auto v : vols)
        d.volumeSteps[i++] = v;
    d.numVolumeSteps = static_cast<int> (vols.size());
    d.volumeLoop = loop;
    return d;
}

static StepSequenceData makePitchData (std::initializer_list<int> pitches, bool loop = false)
{
    StepSequenceData d;
    int i = 0;
    for (auto p : pitches)
        d.pitchSteps[i++] = p;
    d.numPitchSteps = static_cast<int> (pitches.size());
    d.pitchLoop = loop;
    return d;
}

static StepSequenceData makeDutyData (std::initializer_list<int> duties, bool loop = false)
{
    StepSequenceData d;
    int i = 0;
    for (auto v : duties)
        d.dutySteps[i++] = v;
    d.numDutySteps = static_cast<int> (duties.size());
    d.dutyLoop = loop;
    return d;
}

// ─── Tests ──────────────────────────────────────────────────────────────────

TEST_CASE ("StepSequencer: trigger resets indices to 0", "[StepSequencer]")
{
    StepSequencer seq;
    auto data = makeVolumeData ({ 15, 10, 5, 0 }, true);

    seq.setSampleRate (44100.0);
    seq.setRateHz (44100.0f);   // 1 step per sample
    seq.setEnabled (true);
    seq.setData (&data);

    // Advance a few steps
    seq.trigger();
    seq.process();  // step 0
    seq.process();  // step 1
    seq.process();  // step 2

    CHECK (seq.getCurrentVolIndex() == 2);

    // Re-trigger should reset to 0
    seq.trigger();
    auto mod = seq.process();  // should be step 0 again

    CHECK (seq.getCurrentVolIndex() == 0);
    CHECK (mod.volumeOverride == 15);
}

TEST_CASE ("StepSequencer: first tick outputs step 0 immediately", "[StepSequencer]")
{
    StepSequencer seq;
    auto data = makeVolumeData ({ 12, 8, 4 });

    seq.setSampleRate (44100.0);
    seq.setRateHz (44100.0f);
    seq.setEnabled (true);
    seq.setData (&data);
    seq.trigger();

    auto mod = seq.process();
    CHECK (mod.volumeOverride == 12);
    CHECK (seq.getCurrentVolIndex() == 0);
}

TEST_CASE ("StepSequencer: phase accumulator advances at correct rate", "[StepSequencer]")
{
    StepSequencer seq;
    auto data = makeVolumeData ({ 15, 10, 5, 0 }, true);

    seq.setSampleRate (44100.0);
    seq.setRateHz (44100.0f);   // 1 step per sample
    seq.setEnabled (true);
    seq.setData (&data);
    seq.trigger();

    // Step 0 on first tick
    auto m0 = seq.process();
    CHECK (m0.volumeOverride == 15);

    // Step 1 on second tick (phase wraps after 1 sample)
    auto m1 = seq.process();
    CHECK (m1.volumeOverride == 10);

    // Step 2
    auto m2 = seq.process();
    CHECK (m2.volumeOverride == 5);

    // Step 3
    auto m3 = seq.process();
    CHECK (m3.volumeOverride == 0);
}

TEST_CASE ("StepSequencer: volume override returns correct values", "[StepSequencer]")
{
    StepSequencer seq;
    auto data = makeVolumeData ({ 15, 12, 8, 3 }, true);

    seq.setSampleRate (44100.0);
    seq.setRateHz (44100.0f);
    seq.setEnabled (true);
    seq.setData (&data);
    seq.trigger();

    CHECK (seq.process().volumeOverride == 15);
    CHECK (seq.process().volumeOverride == 12);
    CHECK (seq.process().volumeOverride == 8);
    CHECK (seq.process().volumeOverride == 3);
}

TEST_CASE ("StepSequencer: pitch offset returns correct values", "[StepSequencer]")
{
    StepSequencer seq;
    auto data = makePitchData ({ 0, 3, 7, -5 }, true);

    seq.setSampleRate (44100.0);
    seq.setRateHz (44100.0f);
    seq.setEnabled (true);
    seq.setData (&data);
    seq.trigger();

    CHECK (seq.process().pitchOffset == 0);
    CHECK (seq.process().pitchOffset == 3);
    CHECK (seq.process().pitchOffset == 7);
    CHECK (seq.process().pitchOffset == -5);
}

TEST_CASE ("StepSequencer: duty override returns correct values", "[StepSequencer]")
{
    StepSequencer seq;
    auto data = makeDutyData ({ 0, 1, 2, 3 }, true);

    seq.setSampleRate (44100.0);
    seq.setRateHz (44100.0f);
    seq.setEnabled (true);
    seq.setData (&data);
    seq.trigger();

    CHECK (seq.process().dutyOverride == 0);
    CHECK (seq.process().dutyOverride == 1);
    CHECK (seq.process().dutyOverride == 2);
    CHECK (seq.process().dutyOverride == 3);
}

TEST_CASE ("StepSequencer: loop wraps around to step 0", "[StepSequencer]")
{
    StepSequencer seq;
    auto data = makeVolumeData ({ 15, 10, 5 }, true);  // 3 steps, loop on

    seq.setSampleRate (44100.0);
    seq.setRateHz (44100.0f);
    seq.setEnabled (true);
    seq.setData (&data);
    seq.trigger();

    CHECK (seq.process().volumeOverride == 15);  // step 0
    CHECK (seq.process().volumeOverride == 10);  // step 1
    CHECK (seq.process().volumeOverride == 5);   // step 2
    CHECK (seq.process().volumeOverride == 15);  // wraps to step 0
    CHECK (seq.process().volumeOverride == 10);  // step 1 again
}

TEST_CASE ("StepSequencer: non-loop stops after last step (done)", "[StepSequencer]")
{
    StepSequencer seq;
    auto data = makeVolumeData ({ 15, 10, 5 }, false);  // 3 steps, no loop

    // Use rate = sampleRate/2 so we need 2 process() calls per step advance
    seq.setSampleRate (100.0);
    seq.setRateHz (100.0f);
    seq.setEnabled (true);
    seq.setData (&data);
    seq.trigger();

    CHECK (seq.process().volumeOverride == 15);  // step 0 (first tick)
    // Advance to step 1
    CHECK (seq.process().volumeOverride == 10);  // step 1
    // Advance to step 2
    CHECK (seq.process().volumeOverride == 5);   // step 2 (last)
    // Advance past end — done flag set
    CHECK (seq.process().volumeOverride == -1);  // done = no override
    CHECK (seq.process().volumeOverride == -1);  // still done
}

TEST_CASE ("StepSequencer: release stops producing overrides", "[StepSequencer]")
{
    StepSequencer seq;
    auto data = makeVolumeData ({ 15, 10, 5 }, true);

    seq.setSampleRate (44100.0);
    seq.setRateHz (44100.0f);
    seq.setEnabled (true);
    seq.setData (&data);
    seq.trigger();

    CHECK (seq.process().volumeOverride == 15);

    seq.release();

    auto mod = seq.process();
    CHECK (mod.volumeOverride == -1);
    CHECK (mod.pitchOffset == 0);
    CHECK (mod.dutyOverride == -1);
}

TEST_CASE ("StepSequencer: reset clears all state", "[StepSequencer]")
{
    StepSequencer seq;
    auto data = makeVolumeData ({ 15, 10, 5 }, true);

    seq.setSampleRate (44100.0);
    seq.setRateHz (44100.0f);
    seq.setEnabled (true);
    seq.setData (&data);
    seq.trigger();

    seq.process();
    seq.process();

    seq.reset();

    CHECK (seq.getCurrentVolIndex() == 0);
    CHECK (seq.getCurrentPitchIndex() == 0);
    CHECK (seq.getCurrentDutyIndex() == 0);
    CHECK_FALSE (seq.isActive());

    // After reset, process should return no overrides (gateActive is false)
    auto mod = seq.process();
    CHECK (mod.volumeOverride == -1);
}

TEST_CASE ("StepSequencer: lanes with numSteps=0 return no override", "[StepSequencer]")
{
    StepSequencer seq;
    StepSequenceData data;  // all numSteps = 0

    seq.setSampleRate (44100.0);
    seq.setRateHz (44100.0f);
    seq.setEnabled (true);
    seq.setData (&data);
    seq.trigger();

    auto mod = seq.process();
    CHECK (mod.volumeOverride == -1);
    CHECK (mod.pitchOffset == 0);
    CHECK (mod.dutyOverride == -1);
}

TEST_CASE ("StepSequencer: independent lane step counts", "[StepSequencer]")
{
    StepSequencer seq;
    StepSequenceData data;

    // 4 volume steps (loop)
    data.volumeSteps[0] = 15; data.volumeSteps[1] = 10;
    data.volumeSteps[2] = 5;  data.volumeSteps[3] = 0;
    data.numVolumeSteps = 4;
    data.volumeLoop = true;

    // 2 pitch steps (loop)
    data.pitchSteps[0] = 0;   data.pitchSteps[1] = 7;
    data.numPitchSteps = 2;
    data.pitchLoop = true;

    seq.setSampleRate (44100.0);
    seq.setRateHz (44100.0f);
    seq.setEnabled (true);
    seq.setData (&data);
    seq.trigger();

    // Tick 0: vol=15, pitch=0
    auto m0 = seq.process();
    CHECK (m0.volumeOverride == 15);
    CHECK (m0.pitchOffset == 0);

    // Tick 1: vol=10, pitch=7
    auto m1 = seq.process();
    CHECK (m1.volumeOverride == 10);
    CHECK (m1.pitchOffset == 7);

    // Tick 2: vol=5, pitch=0 (pitch wraps at 2 steps)
    auto m2 = seq.process();
    CHECK (m2.volumeOverride == 5);
    CHECK (m2.pitchOffset == 0);

    // Tick 3: vol=0, pitch=7
    auto m3 = seq.process();
    CHECK (m3.volumeOverride == 0);
    CHECK (m3.pitchOffset == 7);

    // Tick 4: vol=15 (vol wraps at 4 steps), pitch=0
    auto m4 = seq.process();
    CHECK (m4.volumeOverride == 15);
    CHECK (m4.pitchOffset == 0);
}

TEST_CASE ("StepSequencer: disabled sequencer returns no overrides", "[StepSequencer]")
{
    StepSequencer seq;
    auto data = makeVolumeData ({ 15, 10, 5 }, true);

    seq.setSampleRate (44100.0);
    seq.setRateHz (44100.0f);
    seq.setEnabled (false);  // disabled
    seq.setData (&data);
    seq.trigger();

    auto mod = seq.process();
    CHECK (mod.volumeOverride == -1);
}

TEST_CASE ("StepSequencer: null data pointer returns no overrides", "[StepSequencer]")
{
    StepSequencer seq;
    seq.setSampleRate (44100.0);
    seq.setRateHz (44100.0f);
    seq.setEnabled (true);
    seq.setData (nullptr);
    seq.trigger();

    auto mod = seq.process();
    CHECK (mod.volumeOverride == -1);
}
