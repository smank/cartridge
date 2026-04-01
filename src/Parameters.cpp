#include "Parameters.h"

namespace cart {

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // ─── Global ─────────────────────────────────────────────────────────
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::MasterVolume, 1 },
        "Master Volume",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
        0.8f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::MasterTune, 1 },
        "Master Tune",
        juce::NormalisableRange<float> (-100.0f, 100.0f, 0.1f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("cents")));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamIDs::Region, 1 },
        "Region",
        juce::StringArray { "NTSC", "PAL" },
        0));

    // ─── Helper lambdas ─────────────────────────────────────────────────
    auto makePulseParams = [&] (const char* prefix, const juce::String& label)
    {
        juce::String p (prefix);

        params.push_back (std::make_unique<juce::AudioParameterBool> (
            juce::ParameterID { p + "Enabled", 1 }, label + " Enabled", true));

        params.push_back (std::make_unique<juce::AudioParameterChoice> (
            juce::ParameterID { p + "Duty", 1 }, label + " Duty",
            juce::StringArray { "12.5%", "25%", "50%", "75%" }, 1));

        params.push_back (std::make_unique<juce::AudioParameterInt> (
            juce::ParameterID { p + "Volume", 1 }, label + " Volume", 0, 15, 15));

        params.push_back (std::make_unique<juce::AudioParameterBool> (
            juce::ParameterID { p + "ConstVol", 1 }, label + " Constant Volume", true));

        params.push_back (std::make_unique<juce::AudioParameterBool> (
            juce::ParameterID { p + "EnvLoop", 1 }, label + " Envelope Loop", false));

        params.push_back (std::make_unique<juce::AudioParameterBool> (
            juce::ParameterID { p + "SweepEnable", 1 }, label + " Sweep Enable", false));

        params.push_back (std::make_unique<juce::AudioParameterInt> (
            juce::ParameterID { p + "SweepPeriod", 1 }, label + " Sweep Period", 0, 7, 0));

        params.push_back (std::make_unique<juce::AudioParameterBool> (
            juce::ParameterID { p + "SweepNegate", 1 }, label + " Sweep Negate", false));

        params.push_back (std::make_unique<juce::AudioParameterInt> (
            juce::ParameterID { p + "SweepShift", 1 }, label + " Sweep Shift", 0, 7, 0));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { p + "Mix", 1 }, label + " Mix",
            juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));
    };

    makePulseParams ("p1", "Pulse 1");
    makePulseParams ("p2", "Pulse 2");

    // ─── Triangle ───────────────────────────────────────────────────────
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::TriEnabled, 1 }, "Triangle Enabled", true));

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { ParamIDs::TriLinearReload, 1 }, "Triangle Linear Counter Reload", 0, 127, 127));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::TriLinearControl, 1 }, "Triangle Linear Counter Control", true));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::TriMix, 1 }, "Triangle Mix",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));

    // ─── Noise ──────────────────────────────────────────────────────────
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::NoiseEnabled, 1 }, "Noise Enabled", true));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamIDs::NoiseMode, 1 }, "Noise Mode",
        juce::StringArray { "Long", "Short" }, 0));

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { ParamIDs::NoisePeriod, 1 }, "Noise Period Index", 0, 15, 0));

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { ParamIDs::NoiseVolume, 1 }, "Noise Volume", 0, 15, 15));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::NoiseConstVol, 1 }, "Noise Constant Volume", true));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::NoiseEnvLoop, 1 }, "Noise Envelope Loop", false));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::NoiseMix, 1 }, "Noise Mix",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));

    // ─── DPCM ───────────────────────────────────────────────────────────
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::DpcmEnabled, 1 }, "DPCM Enabled", false));

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { ParamIDs::DpcmRate, 1 }, "DPCM Rate Index", 0, 15, 0));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::DpcmLoop, 1 }, "DPCM Loop", false));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::DpcmMix, 1 }, "DPCM Mix",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));

    {
        juce::StringArray dpcmSampleNames { "Kick", "Snare", "Hi-Hat", "Tom" };
        for (int i = 0; i < 16; ++i)
            dpcmSampleNames.add("User " + juce::String(i + 1));

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID { ParamIDs::DpcmSample, 1 }, "DPCM Sample",
            dpcmSampleNames, 0));
    }

    // ─── VRC6 Expansion ──────────────────────────────────────────────────
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::Vrc6Enabled, 1 }, "VRC6 Expansion Enabled", false));

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { ParamIDs::Vrc6P1Duty, 1 }, "VRC6 Pulse 1 Duty", 0, 7, 4));

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { ParamIDs::Vrc6P1Volume, 1 }, "VRC6 Pulse 1 Volume", 0, 15, 15));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::Vrc6P1Mix, 1 }, "VRC6 Pulse 1 Mix",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { ParamIDs::Vrc6P2Duty, 1 }, "VRC6 Pulse 2 Duty", 0, 7, 4));

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { ParamIDs::Vrc6P2Volume, 1 }, "VRC6 Pulse 2 Volume", 0, 15, 15));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::Vrc6P2Mix, 1 }, "VRC6 Pulse 2 Mix",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { ParamIDs::Vrc6SawRate, 1 }, "VRC6 Saw Rate", 0, 63, 42));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::Vrc6SawMix, 1 }, "VRC6 Saw Mix",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));

    // ─── Per-Channel Transpose ────────────────────────────────────────
    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { ParamIDs::P1Transpose, 1 }, "Pulse 1 Transpose",
        -24, 24, 0, juce::AudioParameterIntAttributes().withLabel ("st")));

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { ParamIDs::P2Transpose, 1 }, "Pulse 2 Transpose",
        -24, 24, 0, juce::AudioParameterIntAttributes().withLabel ("st")));

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { ParamIDs::TriTranspose, 1 }, "Triangle Transpose",
        -24, 24, 0, juce::AudioParameterIntAttributes().withLabel ("st")));

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { ParamIDs::Vrc6P1Transpose, 1 }, "VRC6 Pulse 1 Transpose",
        -24, 24, 0, juce::AudioParameterIntAttributes().withLabel ("st")));

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { ParamIDs::Vrc6P2Transpose, 1 }, "VRC6 Pulse 2 Transpose",
        -24, 24, 0, juce::AudioParameterIntAttributes().withLabel ("st")));

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { ParamIDs::Vrc6SawTranspose, 1 }, "VRC6 Saw Transpose",
        -24, 24, 0, juce::AudioParameterIntAttributes().withLabel ("st")));

    // ─── Modern Enhancements ────────────────────────────────────────────
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::VelocitySens, 1 },
        "Velocity Sensitivity",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
        0.0f));

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { ParamIDs::PitchBendRange, 1 },
        "Pitch Bend Range",
        1, 24, 2,
        juce::AudioParameterIntAttributes().withLabel ("semitones")));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamIDs::MidiMode, 1 },
        "MIDI Mode",
        juce::StringArray { "Split", "Auto", "Mono" },
        1));

    // ─── Engine Mode ──────────────────────────────────────────────────
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamIDs::EngineMode, 1 },
        "Engine Mode",
        juce::StringArray { "Classic", "Modern" },
        0));

    // ─── Modern Engine ─────────────────────────────────────────────────
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamIDs::ModWaveform, 1 },
        "Mod Waveform",
        juce::StringArray { "Pulse 25%", "Pulse 50%", "Pulse 75%", "Pulse 12.5%", "Triangle", "Sawtooth", "Noise" },
        1));

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { ParamIDs::ModVoices, 1 }, "Mod Voices", 1, 16, 8));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::ModAttack, 1 }, "Mod Attack",
        juce::NormalisableRange<float> (0.001f, 4.0f, 0.001f, 0.35f), 0.005f,
        juce::AudioParameterFloatAttributes().withLabel ("s")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::ModDecay, 1 }, "Mod Decay",
        juce::NormalisableRange<float> (0.001f, 4.0f, 0.001f, 0.35f), 0.1f,
        juce::AudioParameterFloatAttributes().withLabel ("s")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::ModSustain, 1 }, "Mod Sustain",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::ModRelease, 1 }, "Mod Release",
        juce::NormalisableRange<float> (0.001f, 4.0f, 0.001f, 0.35f), 0.1f,
        juce::AudioParameterFloatAttributes().withLabel ("s")));

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { ParamIDs::ModUnison, 1 }, "Mod Unison", 1, 7, 1));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::ModDetune, 1 }, "Mod Detune",
        juce::NormalisableRange<float> (0.0f, 50.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("cents")));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::ModPortaEnabled, 1 }, "Mod Portamento", false));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::ModPortaTime, 1 }, "Mod Portamento Time",
        juce::NormalisableRange<float> (0.01f, 2.0f, 0.01f, 0.4f), 0.1f,
        juce::AudioParameterFloatAttributes().withLabel ("s")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::ModVelToFilter, 1 }, "Mod Vel→Filter",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::ModVolume, 1 }, "Mod Volume",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.8f));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::ModOscAEnabled, 1 }, "Mod Osc A Enabled", true));

    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::ModOscBEnabled, 1 }, "Mod Osc B Enabled", false));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamIDs::ModWaveformB, 1 },
        "Mod Waveform B",
        juce::StringArray { "Pulse 25%", "Pulse 50%", "Pulse 75%", "Pulse 12.5%", "Triangle", "Sawtooth", "Noise" },
        1));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::ModOscBLevel, 1 }, "Mod Osc B Level",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.8f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::ModOscBDetune, 1 }, "Mod Osc B Detune",
        juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("semi")));

    // ─── Portamento ─────────────────────────────────────────────────────
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::PortaEnabled, 1 }, "Portamento Enabled", false));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::PortaTime, 1 }, "Portamento Time",
        juce::NormalisableRange<float> (0.01f, 1.0f, 0.01f), 0.1f,
        juce::AudioParameterFloatAttributes().withLabel ("s")));

    // ─── LFO ───────────────────────────────────────────────────────────
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::LfoEnabled, 1 }, "LFO Enabled", false));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::LfoRate, 1 }, "LFO Rate",
        juce::NormalisableRange<float> (0.1f, 20.0f, 0.1f), 5.0f,
        juce::AudioParameterFloatAttributes().withLabel ("Hz")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::LfoVibratoDepth, 1 }, "Vibrato Depth",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::LfoTremoloDepth, 1 }, "Tremolo Depth",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));

    // ─── Arpeggiator ────────────────────────────────────────────────────
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::ArpEnabled, 1 }, "Arpeggiator Enabled", false));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamIDs::ArpPattern, 1 }, "Arp Pattern",
        juce::StringArray { "Up", "Down", "Up-Down", "Random", "As Played" }, 0));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::ArpRate, 1 }, "Arp Rate",
        juce::NormalisableRange<float> (1.0f, 30.0f, 0.1f), 8.0f,
        juce::AudioParameterFloatAttributes().withLabel ("Hz")));

    params.push_back (std::make_unique<juce::AudioParameterInt> (
        juce::ParameterID { ParamIDs::ArpOctaves, 1 }, "Arp Octaves", 1, 4, 1));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::ArpGate, 1 }, "Arp Gate Length",
        juce::NormalisableRange<float> (0.1f, 1.0f, 0.01f), 0.8f));

    // ─── Effects — BitCrush ─────────────────────────────────────────────
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::BcEnabled, 1 }, "BitCrush Enabled", false));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::BcBitDepth, 1 }, "BitCrush Bit Depth",
        juce::NormalisableRange<float> (1.0f, 16.0f, 0.1f), 16.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::BcRateReduce, 1 }, "BitCrush Rate Reduce",
        juce::NormalisableRange<float> (1.0f, 50.0f, 0.1f, 0.4f), 1.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::BcMix, 1 }, "BitCrush Mix",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));

    // ─── Effects — Filter ───────────────────────────────────────────────
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::FltEnabled, 1 }, "Filter Enabled", false));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamIDs::FltType, 1 }, "Filter Type",
        juce::StringArray { "LP", "BP", "HP" }, 0));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::FltCutoff, 1 }, "Filter Cutoff",
        juce::NormalisableRange<float> (20.0f, 20000.0f, 0.1f, 0.3f), 1000.0f,
        juce::AudioParameterFloatAttributes().withLabel ("Hz")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::FltResonance, 1 }, "Filter Resonance",
        juce::NormalisableRange<float> (0.1f, 5.0f, 0.01f), 0.707f));

    // ─── Effects — Chorus ───────────────────────────────────────────────
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::ChEnabled, 1 }, "Chorus Enabled", false));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::ChRate, 1 }, "Chorus Rate",
        juce::NormalisableRange<float> (0.1f, 10.0f, 0.01f), 0.5f,
        juce::AudioParameterFloatAttributes().withLabel ("Hz")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::ChDepth, 1 }, "Chorus Depth",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.25f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::ChMix, 1 }, "Chorus Mix",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.3f));

    // ─── Effects — Delay ────────────────────────────────────────────────
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::DlEnabled, 1 }, "Delay Enabled", false));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::DlTime, 1 }, "Delay Time",
        juce::NormalisableRange<float> (10.0f, 2000.0f, 0.1f, 0.4f), 375.0f,
        juce::AudioParameterFloatAttributes().withLabel ("ms")));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::DlFeedback, 1 }, "Delay Feedback",
        juce::NormalisableRange<float> (0.0f, 0.95f, 0.01f), 0.4f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::DlMix, 1 }, "Delay Mix",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.3f));

    // ─── Effects — Reverb ───────────────────────────────────────────────
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { ParamIDs::RvEnabled, 1 }, "Reverb Enabled", false));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::RvSize, 1 }, "Reverb Size",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.4f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::RvDamping, 1 }, "Reverb Damping",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::RvWidth, 1 }, "Reverb Width",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.8f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::RvMix, 1 }, "Reverb Mix",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.2f));

    // ─── Custom CC Mappings ─────────────────────────────────────────────
    // Target choices: 0=None, 1=MasterVolume, 2=P1Volume, 3=P2Volume, 4=NoiseVolume,
    // 5=Vrc6P1Volume, 6=Vrc6P2Volume, 7=FilterCutoff, 8=FilterResonance,
    // 9=ChorusMix, 10=DelayMix, 11=ReverbMix, 12=LfoRate, 13=VibratoDepth, 14=TremoloDepth
    juce::StringArray ccTargets { "None", "Master Volume", "Pulse 1 Volume", "Pulse 2 Volume",
                                   "Noise Volume", "VRC6 P1 Volume", "VRC6 P2 Volume",
                                   "Filter Cutoff", "Filter Resonance", "Chorus Mix",
                                   "Delay Mix", "Reverb Mix", "LFO Rate", "Vibrato Depth", "Tremolo Depth" };

    for (int i = 1; i <= 4; ++i)
    {
        juce::String num = juce::String(i);
        params.push_back(std::make_unique<juce::AudioParameterInt>(
            juce::ParameterID { juce::String("userCC") + num + "Num", 1 },
            "User CC " + num + " Number", 0, 127, 0));

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID { juce::String("userCC") + num + "Target", 1 },
            "User CC " + num + " Target", ccTargets, 0));
    }

    // ─── Tuning System ──────────────────────────────────────────────────
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { ParamIDs::TuningSystem, 1 },
        "Tuning System",
        juce::StringArray { "Equal", "Just", "Pythagorean", "Meantone" },
        0));

    return { params.begin(), params.end() };
}

} // namespace cart
