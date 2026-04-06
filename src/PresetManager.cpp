#include "PresetManager.h"

namespace cart {

static const juce::String emptyName = "---";

// Sanitize preset name for use as a filename (cross-platform safe)
static juce::String sanitizeFilename (const juce::String& name)
{
    juce::String safe;
    for (auto c : name)
    {
        // Strip characters illegal on Windows/macOS/Linux filesystems
        if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?'
            || c == '"' || c == '<' || c == '>' || c == '|')
            safe += '_';
        else
            safe += c;
    }
    // Trim leading/trailing whitespace and dots (Windows disallows trailing dots)
    safe = safe.trim();
    while (safe.endsWithChar ('.'))
        safe = safe.dropLastCharacters (1);
    if (safe.isEmpty())
        safe = "Untitled";
    return safe;
}

juce::File PresetManager::getUserPresetDirectory() const
{
    auto dir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                   .getChildFile ("Cartridge")
                   .getChildFile ("Presets");
    dir.createDirectory();
    return dir;
}

void PresetManager::saveUserPreset (const juce::String& name, juce::AudioProcessorValueTreeState& apvts)
{
    Preset p;
    p.name = name;

    // Capture all current parameter values
    for (const auto& [paramID, defaultVal] : defaults)
    {
        if (auto* param = apvts.getParameter (paramID))
            p.values.push_back ({ paramID, param->convertFrom0to1 (param->getValue()) });
    }

    auto file = getUserPresetDirectory().getChildFile (sanitizeFilename (name) + ".xml");
    savePresetToFile (p, file);
    refreshUserPresets();
}

void PresetManager::savePresetToFile (const Preset& preset, const juce::File& file) const
{
    auto xml = std::make_unique<juce::XmlElement> ("CartridgePreset");
    xml->setAttribute ("name", preset.name);

    if (preset.category.isNotEmpty())
        xml->setAttribute ("category", preset.category);

    for (const auto& [paramID, value] : preset.values)
    {
        auto* paramEl = xml->createNewChildElement ("Param");
        paramEl->setAttribute ("id", paramID);
        paramEl->setAttribute ("value", static_cast<double> (value));
    }

    xml->writeTo (file);
}

std::optional<Preset> PresetManager::loadPresetFromFile (const juce::File& file) const
{
    auto xml = juce::XmlDocument::parse (file);
    if (xml == nullptr || !xml->hasTagName ("CartridgePreset"))
        return std::nullopt;

    Preset p;
    p.name = xml->getStringAttribute ("name", file.getFileNameWithoutExtension());
    p.category = xml->getStringAttribute ("category", "");

    for (auto* paramEl : xml->getChildIterator())
    {
        if (paramEl->hasTagName ("Param"))
        {
            auto id = paramEl->getStringAttribute ("id");
            auto val = static_cast<float> (paramEl->getDoubleAttribute ("value"));
            p.values.push_back ({ id, val });
        }
    }

    return p;
}

void PresetManager::loadUserPresets()
{
    auto dir = getUserPresetDirectory();
    for (const auto& file : dir.findChildFiles (juce::File::findFiles, false, "*.xml"))
    {
        if (auto preset = loadPresetFromFile (file))
            presets.push_back (std::move (*preset));
    }
}

void PresetManager::refreshUserPresets()
{
    // Remove user presets, keep factory
    presets.resize (factoryCount);
    loadUserPresets();
}

void PresetManager::deleteUserPreset (int index)
{
    if (!isFactoryPreset (index) && index < static_cast<int> (presets.size()))
    {
        auto file = getUserPresetDirectory().getChildFile (sanitizeFilename (presets[static_cast<size_t> (index)].name) + ".xml");
        file.deleteFile();
        refreshUserPresets();
    }
}

PresetManager::PresetManager()
{
    // Default values: all channels on, sensible starting points
    defaults = {
        // Global
        { ParamIDs::MasterVolume,     0.8f },
        { ParamIDs::MasterTune,       0.0f },
        { ParamIDs::Region,           0.0f },  // NTSC

        // Pulse 1
        { ParamIDs::P1Enabled,        1.0f },
        { ParamIDs::P1Duty,           1.0f },  // 25%
        { ParamIDs::P1Volume,         15.0f },
        { ParamIDs::P1ConstVol,       1.0f },
        { ParamIDs::P1EnvLoop,        0.0f },
        { ParamIDs::P1SweepEnable,    0.0f },
        { ParamIDs::P1SweepPeriod,    0.0f },
        { ParamIDs::P1SweepNegate,    0.0f },
        { ParamIDs::P1SweepShift,     0.0f },
        { ParamIDs::P1Mix,            1.0f },

        // Pulse 2
        { ParamIDs::P2Enabled,        0.0f },  // Off by default
        { ParamIDs::P2Duty,           1.0f },
        { ParamIDs::P2Volume,         15.0f },
        { ParamIDs::P2ConstVol,       1.0f },
        { ParamIDs::P2EnvLoop,        0.0f },
        { ParamIDs::P2SweepEnable,    0.0f },
        { ParamIDs::P2SweepPeriod,    0.0f },
        { ParamIDs::P2SweepNegate,    0.0f },
        { ParamIDs::P2SweepShift,     0.0f },
        { ParamIDs::P2Mix,            1.0f },

        // Triangle
        { ParamIDs::TriEnabled,       0.0f },
        { ParamIDs::TriLinearReload,  127.0f },
        { ParamIDs::TriLinearControl, 1.0f },
        { ParamIDs::TriMix,           1.0f },

        // Noise
        { ParamIDs::NoiseEnabled,     0.0f },
        { ParamIDs::NoiseMode,        0.0f },  // Long
        { ParamIDs::NoisePeriod,      0.0f },
        { ParamIDs::NoiseVolume,      15.0f },
        { ParamIDs::NoiseConstVol,    1.0f },
        { ParamIDs::NoiseEnvLoop,     0.0f },
        { ParamIDs::NoiseMix,         1.0f },

        // DPCM
        { ParamIDs::DpcmEnabled,      0.0f },
        { ParamIDs::DpcmRate,         0.0f },
        { ParamIDs::DpcmLoop,         0.0f },
        { ParamIDs::DpcmMix,          1.0f },
        { ParamIDs::DpcmSample,       0.0f },  // Kick

        // VRC6
        { ParamIDs::Vrc6Enabled,      0.0f },
        { ParamIDs::Vrc6P1Duty,       4.0f },
        { ParamIDs::Vrc6P1Volume,     15.0f },
        { ParamIDs::Vrc6P1Mix,        1.0f },
        { ParamIDs::Vrc6P2Duty,       4.0f },
        { ParamIDs::Vrc6P2Volume,     15.0f },
        { ParamIDs::Vrc6P2Mix,        1.0f },
        { ParamIDs::Vrc6SawRate,      42.0f },
        { ParamIDs::Vrc6SawMix,       1.0f },

        // Per-channel transpose
        { ParamIDs::P1Transpose,      0.0f },
        { ParamIDs::P2Transpose,      0.0f },
        { ParamIDs::TriTranspose,     0.0f },
        { ParamIDs::Vrc6P1Transpose,  0.0f },
        { ParamIDs::Vrc6P2Transpose,  0.0f },
        { ParamIDs::Vrc6SawTranspose, 0.0f },

        // Engine Mode
        { ParamIDs::EngineMode,       0.0f },  // Classic (default)

        // Modern Engine
        { ParamIDs::ModWaveform,      1.0f },  // Pulse 50%
        { ParamIDs::ModVoices,        8.0f },
        { ParamIDs::ModAttack,        0.005f },
        { ParamIDs::ModDecay,         0.1f },
        { ParamIDs::ModSustain,       1.0f },
        { ParamIDs::ModRelease,       0.1f },
        { ParamIDs::ModUnison,        1.0f },
        { ParamIDs::ModDetune,        0.0f },
        { ParamIDs::ModPortaEnabled,  0.0f },
        { ParamIDs::ModPortaTime,     0.1f },
        { ParamIDs::ModVelToFilter,   0.0f },
        { ParamIDs::ModVolume,        0.8f },
        { ParamIDs::ModOscAEnabled,   1.0f },
        { ParamIDs::ModOscBEnabled,   0.0f },
        { ParamIDs::ModWaveformB,     1.0f },  // Pulse 50%
        { ParamIDs::ModOscBLevel,     0.8f },
        { ParamIDs::ModOscBDetune,    0.0f },

        // Modern
        { ParamIDs::VelocitySens,     0.0f },
        { ParamIDs::PitchBendRange,   2.0f },
        { ParamIDs::MidiMode,         1.0f },  // Auto

        // Portamento
        { ParamIDs::PortaEnabled,     0.0f },
        { ParamIDs::PortaTime,        0.1f },

        // LFO
        { ParamIDs::LfoEnabled,       0.0f },
        { ParamIDs::LfoRate,           5.0f },
        { ParamIDs::LfoVibratoDepth,   0.0f },
        { ParamIDs::LfoTremoloDepth,   0.0f },

        // Arpeggiator
        { ParamIDs::ArpEnabled,       0.0f },
        { ParamIDs::ArpPattern,       0.0f },  // Up
        { ParamIDs::ArpRate,          8.0f },
        { ParamIDs::ArpOctaves,       1.0f },
        { ParamIDs::ArpGate,          0.8f },

        // Effects — all off by default
        { ParamIDs::BcEnabled,        0.0f },
        { ParamIDs::BcBitDepth,       16.0f },
        { ParamIDs::BcRateReduce,     1.0f },
        { ParamIDs::BcMix,            1.0f },

        { ParamIDs::FltEnabled,       0.0f },
        { ParamIDs::FltType,          0.0f },  // LP
        { ParamIDs::FltCutoff,        1000.0f },
        { ParamIDs::FltResonance,     0.707f },

        { ParamIDs::ChEnabled,        0.0f },
        { ParamIDs::ChRate,           0.5f },
        { ParamIDs::ChDepth,          0.25f },
        { ParamIDs::ChMix,            0.3f },

        { ParamIDs::DlEnabled,        0.0f },
        { ParamIDs::DlTime,           375.0f },
        { ParamIDs::DlFeedback,       0.4f },
        { ParamIDs::DlMix,            0.3f },

        { ParamIDs::RvEnabled,        0.0f },
        { ParamIDs::RvSize,           0.4f },
        { ParamIDs::RvDamping,        0.5f },
        { ParamIDs::RvWidth,          0.8f },
        { ParamIDs::RvMix,            0.2f },
    };

    buildPresets();
    factoryCount = presets.size();
    loadUserPresets();
}

const juce::String& PresetManager::getPresetName (int i) const
{
    if (i >= 0 && i < static_cast<int> (presets.size()))
        return presets[static_cast<size_t> (i)].name;
    return emptyName;
}

Preset PresetManager::makePreset (const juce::String& name,
                                  std::initializer_list<std::pair<juce::String, float>> overrides) const
{
    Preset p;
    p.name = name;
    p.values = defaults;

    // Apply overrides
    for (const auto& ov : overrides)
    {
        bool found = false;
        for (auto& v : p.values)
        {
            if (v.first == ov.first)
            {
                v.second = ov.second;
                found = true;
                break;
            }
        }
        if (!found)
            p.values.push_back (ov);
    }

    return p;
}

void PresetManager::applyPreset (int index, juce::AudioProcessorValueTreeState& apvts) const
{
    if (index < 0 || index >= static_cast<int> (presets.size()))
        return;

    const auto& preset = presets[static_cast<size_t> (index)];
    for (const auto& [paramID, value] : preset.values)
    {
        if (auto* param = apvts.getParameter (paramID))
            param->setValueNotifyingHost (param->convertTo0to1 (value));
    }
}

void PresetManager::buildPresets()
{
    // ═══════════════════════════════════════════════════════════════════════
    //  1. INIT
    // ═══════════════════════════════════════════════════════════════════════
    presets.push_back (makePreset ("Init", {}));

    // ═══════════════════════════════════════════════════════════════════════
    //  LEADS (2–6)
    // ═══════════════════════════════════════════════════════════════════════

    // 2. Pipe Dream — clean 50% duty square + light reverb
    presets.push_back (makePreset ("Pipe Dream", {
        { ParamIDs::P1Duty,       2.0f },   // 50%
        { ParamIDs::P1Volume,     15.0f },
        { ParamIDs::P1ConstVol,   1.0f },
        { ParamIDs::RvEnabled,    1.0f },
        { ParamIDs::RvSize,       0.2f },
        { ParamIDs::RvMix,        0.1f },
    }));

    // 3. Stage Select — bright 25% duty + subtle bitcrush
    presets.push_back (makePreset ("Stage Select", {
        { ParamIDs::P1Duty,       1.0f },   // 25%
        { ParamIDs::P1Volume,     15.0f },
        { ParamIDs::P1ConstVol,   1.0f },
        { ParamIDs::BcEnabled,    1.0f },
        { ParamIDs::BcBitDepth,   12.0f },
        { ParamIDs::BcRateReduce, 2.0f },
        { ParamIDs::BcMix,        0.5f },
    }));

    // 4. Hidden Grotto — thin 12.5% pulse + sweep up for mysterious feel
    presets.push_back (makePreset ("Hidden Grotto", {
        { ParamIDs::P1Duty,           0.0f },   // 12.5%
        { ParamIDs::P1Volume,         15.0f },
        { ParamIDs::P1ConstVol,       1.0f },
        { ParamIDs::P1SweepEnable,    1.0f },
        { ParamIDs::P1SweepPeriod,    3.0f },
        { ParamIDs::P1SweepNegate,    1.0f },   // Pitch up
        { ParamIDs::P1SweepShift,     5.0f },
    }));

    // 5. Midnight Requiem — dual pulse (P1 25% + P2 50%) + chorus + reverb
    presets.push_back (makePreset ("Midnight Requiem", {
        { ParamIDs::P1Duty,       1.0f },   // 25%
        { ParamIDs::P1Volume,     15.0f },
        { ParamIDs::P2Enabled,    1.0f },
        { ParamIDs::P2Duty,       2.0f },   // 50%
        { ParamIDs::P2Volume,     13.0f },
        { ParamIDs::ChEnabled,    1.0f },
        { ParamIDs::ChRate,       0.4f },
        { ParamIDs::ChDepth,      0.15f },
        { ParamIDs::ChMix,        0.2f },
        { ParamIDs::RvEnabled,    1.0f },
        { ParamIDs::RvSize,       0.25f },
        { ParamIDs::RvMix,        0.12f },
    }));

    // 6. Lunar Bounce — 25% duty + light chorus FX
    presets.push_back (makePreset ("Lunar Bounce", {
        { ParamIDs::P1Duty,       1.0f },   // 25%
        { ParamIDs::P1Volume,     15.0f },
        { ParamIDs::P1ConstVol,   1.0f },
        { ParamIDs::ChEnabled,    1.0f },
        { ParamIDs::ChRate,       0.6f },
        { ParamIDs::ChDepth,      0.2f },
        { ParamIDs::ChMix,        0.25f },
    }));

    // ═══════════════════════════════════════════════════════════════════════
    //  BASS (7–9)
    // ═══════════════════════════════════════════════════════════════════════

    // 7. Triangle Bass — tri only, full linear counter
    presets.push_back (makePreset ("Triangle Bass", {
        { ParamIDs::P1Enabled,        0.0f },
        { ParamIDs::TriEnabled,       1.0f },
        { ParamIDs::TriLinearReload,  127.0f },
        { ParamIDs::TriLinearControl, 1.0f },
        { ParamIDs::MidiMode,         1.0f },   // Auto — routes keyboard to triangle
    }));

    // 8. Pulse Bass — 50% duty, vol 12, punchy + LP filter
    presets.push_back (makePreset ("Pulse Bass", {
        { ParamIDs::P1Duty,       2.0f },   // 50%
        { ParamIDs::P1Volume,     12.0f },
        { ParamIDs::FltEnabled,   1.0f },
        { ParamIDs::FltType,      0.0f },   // LP
        { ParamIDs::FltCutoff,    800.0f },
        { ParamIDs::FltResonance, 1.2f },
    }));

    // 9. VRC6 Saw Bass — Demon Castle-style saw bass
    presets.push_back (makePreset ("VRC6 Saw Bass", {
        { ParamIDs::P1Enabled,    0.0f },
        { ParamIDs::Vrc6Enabled,  1.0f },
        { ParamIDs::Vrc6P1Mix,    0.0f },   // Mute VRC6 Pulse 1
        { ParamIDs::Vrc6P2Mix,    0.0f },   // Mute VRC6 Pulse 2
        { ParamIDs::Vrc6SawRate,  42.0f },
        { ParamIDs::Vrc6SawMix,   1.0f },
    }));

    // ═══════════════════════════════════════════════════════════════════════
    //  PERCUSSION (10–13)
    // ═══════════════════════════════════════════════════════════════════════

    // 10. Kick Drum — noise long mode, period 1, envelope decay
    presets.push_back (makePreset ("Kick Drum", {
        { ParamIDs::P1Enabled,     0.0f },
        { ParamIDs::NoiseEnabled,  1.0f },
        { ParamIDs::NoiseMode,     0.0f },   // Long
        { ParamIDs::NoisePeriod,   1.0f },
        { ParamIDs::NoiseVolume,   15.0f },
        { ParamIDs::NoiseConstVol, 0.0f },   // Envelope decay
        { ParamIDs::NoiseEnvLoop,  0.0f },
        { ParamIDs::MidiMode,      1.0f },   // Auto
    }));

    // 11. Snare — noise long mode, period 5
    presets.push_back (makePreset ("Snare", {
        { ParamIDs::P1Enabled,     0.0f },
        { ParamIDs::NoiseEnabled,  1.0f },
        { ParamIDs::NoiseMode,     0.0f },
        { ParamIDs::NoisePeriod,   5.0f },
        { ParamIDs::NoiseVolume,   15.0f },
        { ParamIDs::NoiseConstVol, 0.0f },
        { ParamIDs::NoiseEnvLoop,  0.0f },
        { ParamIDs::MidiMode,      1.0f },   // Auto
    }));

    // 12. Hi-Hat Closed — noise long mode, period 12, fast decay
    presets.push_back (makePreset ("Hi-Hat Closed", {
        { ParamIDs::P1Enabled,     0.0f },
        { ParamIDs::NoiseEnabled,  1.0f },
        { ParamIDs::NoiseMode,     0.0f },
        { ParamIDs::NoisePeriod,   12.0f },
        { ParamIDs::NoiseVolume,   13.0f },
        { ParamIDs::NoiseConstVol, 0.0f },
        { ParamIDs::NoiseEnvLoop,  0.0f },
        { ParamIDs::MidiMode,      1.0f },   // Auto
    }));

    // 13. Hi-Hat Metallic — noise short mode, period 10, buzzy
    presets.push_back (makePreset ("Hi-Hat Metallic", {
        { ParamIDs::P1Enabled,     0.0f },
        { ParamIDs::NoiseEnabled,  1.0f },
        { ParamIDs::NoiseMode,     1.0f },   // Short (93-step, metallic)
        { ParamIDs::NoisePeriod,   10.0f },
        { ParamIDs::NoiseVolume,   10.0f },
        { ParamIDs::NoiseConstVol, 0.0f },
        { ParamIDs::NoiseEnvLoop,  0.0f },
        { ParamIDs::MidiMode,      1.0f },   // Auto
    }));

    // ═══════════════════════════════════════════════════════════════════════
    //  SFX (14–16)
    // ═══════════════════════════════════════════════════════════════════════

    // 14. Coin Pickup — 50% pulse, sweep up fast
    presets.push_back (makePreset ("Coin Pickup", {
        { ParamIDs::P1Duty,           2.0f },   // 50%
        { ParamIDs::P1Volume,         15.0f },
        { ParamIDs::P1SweepEnable,    1.0f },
        { ParamIDs::P1SweepPeriod,    1.0f },
        { ParamIDs::P1SweepNegate,    1.0f },   // Pitch goes UP
        { ParamIDs::P1SweepShift,     6.0f },
    }));

    // 15. Jump — 50% pulse, sweep down
    presets.push_back (makePreset ("Jump", {
        { ParamIDs::P1Duty,           2.0f },
        { ParamIDs::P1Volume,         15.0f },
        { ParamIDs::P1SweepEnable,    1.0f },
        { ParamIDs::P1SweepPeriod,    4.0f },
        { ParamIDs::P1SweepNegate,    0.0f },   // Pitch goes DOWN
        { ParamIDs::P1SweepShift,     5.0f },
    }));

    // 16. Laser Shot — 25% duty, fast sweep down, aggressive
    presets.push_back (makePreset ("Laser Shot", {
        { ParamIDs::P1Duty,           1.0f },   // 25%
        { ParamIDs::P1Volume,         15.0f },
        { ParamIDs::P1SweepEnable,    1.0f },
        { ParamIDs::P1SweepPeriod,    2.0f },
        { ParamIDs::P1SweepNegate,    0.0f },
        { ParamIDs::P1SweepShift,     3.0f },
    }));

    // ═══════════════════════════════════════════════════════════════════════
    //  FULL SETUPS (17–19)
    // ═══════════════════════════════════════════════════════════════════════

    // 17. Full Band — P1 lead + P2 harmony + Tri bass + Noise drums + reverb
    presets.push_back (makePreset ("Full Band", {
        { ParamIDs::P1Duty,           1.0f },   // Lead: 25%
        { ParamIDs::P1Volume,         15.0f },
        { ParamIDs::P2Enabled,        1.0f },
        { ParamIDs::P2Duty,           2.0f },   // Harmony: 50%
        { ParamIDs::P2Volume,         12.0f },
        { ParamIDs::TriEnabled,       1.0f },   // Bass
        { ParamIDs::NoiseEnabled,     1.0f },   // Drums
        { ParamIDs::NoisePeriod,      5.0f },
        { ParamIDs::NoiseVolume,      15.0f },
        { ParamIDs::NoiseConstVol,    0.0f },
        { ParamIDs::RvEnabled,        1.0f },
        { ParamIDs::RvSize,           0.2f },
        { ParamIDs::RvMix,            0.1f },
    }));

    // 18. Demon Castle — all 8 channels, VRC6 on, full expansion setup
    presets.push_back (makePreset ("Demon Castle", {
        { ParamIDs::P1Duty,           1.0f },   // Lead: 25%
        { ParamIDs::P1Volume,         15.0f },
        { ParamIDs::P2Enabled,        1.0f },
        { ParamIDs::P2Duty,           0.0f },   // Thin countermelody: 12.5%
        { ParamIDs::P2Volume,         13.0f },
        { ParamIDs::TriEnabled,       1.0f },   // Mid voice
        { ParamIDs::NoiseEnabled,     1.0f },   // Drums
        { ParamIDs::NoisePeriod,      5.0f },
        { ParamIDs::NoiseVolume,      15.0f },
        { ParamIDs::NoiseConstVol,    0.0f },
        { ParamIDs::Vrc6Enabled,      1.0f },
        { ParamIDs::Vrc6P1Duty,       3.0f },   // VRC6 harmony: 25%
        { ParamIDs::Vrc6P1Volume,     14.0f },
        { ParamIDs::Vrc6P2Duty,       5.0f },   // VRC6 pad: 37.5%
        { ParamIDs::Vrc6P2Volume,     11.0f },
        { ParamIDs::Vrc6SawRate,      42.0f },  // Saw bass
        { ParamIDs::Vrc6SawMix,       1.0f },
        { ParamIDs::MidiMode,         0.0f },   // Split
    }));

    // 19. Expansion Strings — 4 pulse channels (2 APU + 2 VRC6), velocity sensitive + chorus
    presets.push_back (makePreset ("Expansion Strings", {
        { ParamIDs::P1Duty,           2.0f },   // 50%
        { ParamIDs::P1Volume,         14.0f },
        { ParamIDs::P2Enabled,        1.0f },
        { ParamIDs::P2Duty,           1.0f },   // 25%
        { ParamIDs::P2Volume,         12.0f },
        { ParamIDs::Vrc6Enabled,      1.0f },
        { ParamIDs::Vrc6P1Duty,       6.0f },   // 43.75%
        { ParamIDs::Vrc6P1Volume,     13.0f },
        { ParamIDs::Vrc6P2Duty,       3.0f },   // 25%
        { ParamIDs::Vrc6P2Volume,     11.0f },
        { ParamIDs::VelocitySens,     0.5f },
        { ParamIDs::MidiMode,         0.0f },   // Split
        { ParamIDs::ChEnabled,        1.0f },
        { ParamIDs::ChRate,           0.3f },
        { ParamIDs::ChDepth,          0.3f },
        { ParamIDs::ChMix,            0.25f },
    }));

    // ═══════════════════════════════════════════════════════════════════════
    //  WITH EFFECTS (20–21)
    // ═══════════════════════════════════════════════════════════════════════

    // 20. Lo-Fi Chiptune — P1 25% + bitcrush (8-bit, rate 4) + reverb
    presets.push_back (makePreset ("Lo-Fi Chiptune", {
        { ParamIDs::P1Duty,       1.0f },   // 25%
        { ParamIDs::P1Volume,     15.0f },
        { ParamIDs::BcEnabled,    1.0f },
        { ParamIDs::BcBitDepth,   8.0f },
        { ParamIDs::BcRateReduce, 4.0f },
        { ParamIDs::BcMix,        1.0f },
        { ParamIDs::RvEnabled,    1.0f },
        { ParamIDs::RvSize,       0.3f },
        { ParamIDs::RvDamping,    0.6f },
        { ParamIDs::RvMix,        0.15f },
    }));

    // 21. Ambient Pad — Triangle + chorus + delay + large reverb
    presets.push_back (makePreset ("Ambient Pad", {
        { ParamIDs::P1Enabled,        0.0f },
        { ParamIDs::TriEnabled,       1.0f },
        { ParamIDs::TriLinearReload,  127.0f },
        { ParamIDs::TriLinearControl, 1.0f },
        { ParamIDs::MidiMode,         1.0f },   // Auto — routes to triangle
        { ParamIDs::ChEnabled,        1.0f },
        { ParamIDs::ChRate,           0.3f },
        { ParamIDs::ChDepth,          0.35f },
        { ParamIDs::ChMix,            0.4f },
        { ParamIDs::DlEnabled,        1.0f },
        { ParamIDs::DlTime,           500.0f },
        { ParamIDs::DlFeedback,       0.5f },
        { ParamIDs::DlMix,            0.35f },
        { ParamIDs::RvEnabled,        1.0f },
        { ParamIDs::RvSize,           0.8f },
        { ParamIDs::RvDamping,        0.3f },
        { ParamIDs::RvWidth,          1.0f },
        { ParamIDs::RvMix,            0.4f },
    }));

    // ═══════════════════════════════════════════════════════════════════════
    //  ARPEGGIATOR & SHOWCASE (22–25)
    // ═══════════════════════════════════════════════════════════════════════

    // 22. Chiptune Arp — P1 25% + Arp Up, Rate=12Hz, Octaves=2, gate 50%
    presets.push_back (makePreset ("Chiptune Arp", {
        { ParamIDs::P1Duty,       1.0f },   // 25%
        { ParamIDs::P1Volume,     15.0f },
        { ParamIDs::ArpEnabled,   1.0f },
        { ParamIDs::ArpPattern,   0.0f },   // Up
        { ParamIDs::ArpRate,      12.0f },
        { ParamIDs::ArpOctaves,   2.0f },
        { ParamIDs::ArpGate,      0.5f },
    }));

    // 23. Retro Arp Pad — P1+P2 + Arp UpDown, Rate=6Hz, Oct=2, Chorus+Reverb
    presets.push_back (makePreset ("Retro Arp Pad", {
        { ParamIDs::P1Duty,       1.0f },   // 25%
        { ParamIDs::P1Volume,     15.0f },
        { ParamIDs::P2Enabled,    1.0f },
        { ParamIDs::P2Duty,       2.0f },   // 50%
        { ParamIDs::P2Volume,     12.0f },
        { ParamIDs::ArpEnabled,   1.0f },
        { ParamIDs::ArpPattern,   2.0f },   // UpDown
        { ParamIDs::ArpRate,      6.0f },
        { ParamIDs::ArpOctaves,   2.0f },
        { ParamIDs::ArpGate,      0.7f },
        { ParamIDs::ChEnabled,    1.0f },
        { ParamIDs::ChRate,       0.4f },
        { ParamIDs::ChDepth,      0.2f },
        { ParamIDs::ChMix,        0.25f },
        { ParamIDs::RvEnabled,    1.0f },
        { ParamIDs::RvSize,       0.5f },
        { ParamIDs::RvMix,        0.2f },
    }));

    // 24. VRC6 Lead — VRC6 Pulse 1 (duty 3) + delay + reverb
    presets.push_back (makePreset ("VRC6 Lead", {
        { ParamIDs::P1Enabled,    0.0f },
        { ParamIDs::Vrc6Enabled,  1.0f },
        { ParamIDs::Vrc6P1Duty,   3.0f },   // 25%
        { ParamIDs::Vrc6P1Volume, 15.0f },
        { ParamIDs::Vrc6P2Mix,    0.0f },   // Mute VRC6 Pulse 2
        { ParamIDs::Vrc6SawMix,   0.0f },   // Mute VRC6 Saw
        { ParamIDs::DlEnabled,    1.0f },
        { ParamIDs::DlTime,       300.0f },
        { ParamIDs::DlFeedback,   0.35f },
        { ParamIDs::DlMix,        0.25f },
        { ParamIDs::RvEnabled,    1.0f },
        { ParamIDs::RvSize,       0.35f },
        { ParamIDs::RvMix,        0.15f },
    }));

    // 25. Filtered Noise — Noise + Filter LP sweep + delay
    presets.push_back (makePreset ("Filtered Noise", {
        { ParamIDs::P1Enabled,     0.0f },
        { ParamIDs::NoiseEnabled,  1.0f },
        { ParamIDs::NoiseMode,     0.0f },   // Long
        { ParamIDs::NoisePeriod,   3.0f },
        { ParamIDs::NoiseVolume,   15.0f },
        { ParamIDs::NoiseConstVol, 1.0f },
        { ParamIDs::FltEnabled,    1.0f },
        { ParamIDs::FltType,       0.0f },   // LP
        { ParamIDs::FltCutoff,     600.0f },
        { ParamIDs::FltResonance,  1.5f },
        { ParamIDs::DlEnabled,     1.0f },
        { ParamIDs::DlTime,        400.0f },
        { ParamIDs::DlFeedback,    0.45f },
        { ParamIDs::DlMix,         0.3f },
    }));

    // ═══════════════════════════════════════════════════════════════════════
    //  GAME INSPIRED (26–30)
    // ═══════════════════════════════════════════════════════════════════════

    // 26. Planet Surface — Exploration atmosphere
    presets.push_back (makePreset ("Planet Surface", {
        { ParamIDs::P1Duty,           0.0f },   // 12.5%
        { ParamIDs::P1Volume,         13.0f },
        { ParamIDs::P1ConstVol,       1.0f },
        { ParamIDs::TriEnabled,       1.0f },
        { ParamIDs::VelocitySens,     0.3f },
        { ParamIDs::ChEnabled,        1.0f },
        { ParamIDs::ChRate,           0.3f },
        { ParamIDs::ChDepth,          0.3f },
        { ParamIDs::ChMix,            0.3f },
        { ParamIDs::RvEnabled,        1.0f },
        { ParamIDs::RvSize,           0.6f },
        { ParamIDs::RvMix,            0.25f },
        { ParamIDs::DlEnabled,        1.0f },
        { ParamIDs::DlTime,           450.0f },
        { ParamIDs::DlFeedback,       0.4f },
        { ParamIDs::DlMix,            0.2f },
    }));

    // 27. Underground Lair — Dark dungeon tension
    presets.push_back (makePreset ("Underground Lair", {
        { ParamIDs::P1Duty,           1.0f },   // 25%
        { ParamIDs::P1Volume,         10.0f },
        { ParamIDs::P1ConstVol,       0.0f },   // Envelope decay
        { ParamIDs::NoiseEnabled,     1.0f },
        { ParamIDs::NoiseMode,        0.0f },   // Long
        { ParamIDs::NoisePeriod,      8.0f },
        { ParamIDs::NoiseVolume,      6.0f },
        { ParamIDs::NoiseConstVol,    0.0f },   // Envelope decay
        { ParamIDs::FltEnabled,       1.0f },
        { ParamIDs::FltType,          0.0f },   // LP
        { ParamIDs::FltCutoff,        500.0f },
        { ParamIDs::FltResonance,     1.5f },
        { ParamIDs::RvEnabled,        1.0f },
        { ParamIDs::RvSize,           0.7f },
        { ParamIDs::RvDamping,        0.7f },
        { ParamIDs::RvMix,            0.3f },
    }));

    // 28. Emergency Exit — Urgent action
    presets.push_back (makePreset ("Emergency Exit", {
        { ParamIDs::P1Duty,           2.0f },   // 50%
        { ParamIDs::P1Volume,         15.0f },
        { ParamIDs::P1ConstVol,       1.0f },
        { ParamIDs::P1SweepEnable,    1.0f },
        { ParamIDs::P1SweepPeriod,    3.0f },
        { ParamIDs::P1SweepNegate,    0.0f },   // Pitch down
        { ParamIDs::P1SweepShift,     4.0f },
        { ParamIDs::BcEnabled,        1.0f },
        { ParamIDs::BcBitDepth,       10.0f },
        { ParamIDs::BcRateReduce,     3.0f },
        { ParamIDs::BcMix,            0.6f },
        { ParamIDs::DlEnabled,        1.0f },
        { ParamIDs::DlTime,           200.0f },
        { ParamIDs::DlFeedback,       0.3f },
        { ParamIDs::DlMix,            0.2f },
    }));

    // 29. Hidden Chamber — Mysterious discovery
    presets.push_back (makePreset ("Hidden Chamber", {
        { ParamIDs::P1Enabled,        0.0f },
        { ParamIDs::TriEnabled,       1.0f },
        { ParamIDs::ChEnabled,        1.0f },
        { ParamIDs::ChRate,           0.2f },
        { ParamIDs::ChDepth,          0.4f },
        { ParamIDs::ChMix,            0.35f },
        { ParamIDs::DlEnabled,        1.0f },
        { ParamIDs::DlTime,           600.0f },
        { ParamIDs::DlFeedback,       0.55f },
        { ParamIDs::DlMix,            0.3f },
        { ParamIDs::RvEnabled,        1.0f },
        { ParamIDs::RvSize,           0.9f },
        { ParamIDs::RvDamping,        0.2f },
        { ParamIDs::RvWidth,          1.0f },
        { ParamIDs::RvMix,            0.45f },
    }));

    // 30. Surface Tension — Overworld action theme
    presets.push_back (makePreset ("Surface Tension", {
        { ParamIDs::P1Duty,           1.0f },   // 25%
        { ParamIDs::P1Volume,         15.0f },
        { ParamIDs::P2Enabled,        1.0f },
        { ParamIDs::P2Duty,           2.0f },   // 50%
        { ParamIDs::P2Volume,         12.0f },
        { ParamIDs::TriEnabled,       1.0f },
        { ParamIDs::NoiseEnabled,     1.0f },
        { ParamIDs::NoisePeriod,      5.0f },
        { ParamIDs::NoiseVolume,      15.0f },
        { ParamIDs::NoiseConstVol,    0.0f },   // Envelope decay
        { ParamIDs::Vrc6Enabled,      1.0f },
        { ParamIDs::Vrc6P1Mix,        0.0f },   // Mute VRC6 Pulse 1
        { ParamIDs::Vrc6P2Mix,        0.0f },   // Mute VRC6 Pulse 2
        { ParamIDs::Vrc6SawRate,      42.0f },  // Saw bass
        { ParamIDs::Vrc6SawMix,       1.0f },
        { ParamIDs::VelocitySens,     0.4f },
        { ParamIDs::MidiMode,         0.0f },   // Split
    }));

    // ═══════════════════════════════════════════════════════════════════════
    //  MODERN ENGINE (31–35)
    // ═══════════════════════════════════════════════════════════════════════

    // 31. Modern Pulse Pad — Pulse 50% + Triangle octave layer + chorus
    presets.push_back (makePreset ("Modern Pulse Pad", {
        { ParamIDs::EngineMode,       1.0f },   // Modern
        { ParamIDs::ModWaveform,      1.0f },   // Pulse 50% (Osc A)
        { ParamIDs::ModOscBEnabled,   1.0f },   // Enable Osc B
        { ParamIDs::ModWaveformB,     4.0f },   // Triangle (Osc B)
        { ParamIDs::ModOscBLevel,     0.5f },
        { ParamIDs::ModOscBDetune,    12.0f },  // Octave up
        { ParamIDs::ModVoices,        8.0f },
        { ParamIDs::ModAttack,        0.05f },
        { ParamIDs::ModDecay,         0.3f },
        { ParamIDs::ModSustain,       0.7f },
        { ParamIDs::ModRelease,       0.8f },
        { ParamIDs::ModUnison,        3.0f },
        { ParamIDs::ModDetune,        12.0f },
        { ParamIDs::ModVolume,        0.8f },
        { ParamIDs::VelocitySens,     0.4f },
        { ParamIDs::ChEnabled,        1.0f },
        { ParamIDs::ChRate,           0.4f },
        { ParamIDs::ChDepth,          0.2f },
        { ParamIDs::ChMix,            0.3f },
    }));

    // 32. Chiptune Poly Lead — Punchy polyphonic 25% pulse
    presets.push_back (makePreset ("Chiptune Poly Lead", {
        { ParamIDs::EngineMode,       1.0f },   // Modern
        { ParamIDs::ModWaveform,      0.0f },   // Pulse 25%
        { ParamIDs::ModVoices,        6.0f },
        { ParamIDs::ModAttack,        0.002f },
        { ParamIDs::ModDecay,         0.15f },
        { ParamIDs::ModSustain,       0.8f },
        { ParamIDs::ModRelease,       0.15f },
        { ParamIDs::ModVolume,        0.85f },
        { ParamIDs::VelocitySens,     0.5f },
    }));

    // 33. Thick Saw Unison — Saw + Pulse 50% layered, full unison spread
    presets.push_back (makePreset ("Thick Saw Unison", {
        { ParamIDs::EngineMode,       1.0f },   // Modern
        { ParamIDs::ModWaveform,      5.0f },   // Sawtooth (Osc A)
        { ParamIDs::ModOscBEnabled,   1.0f },   // Enable Osc B
        { ParamIDs::ModWaveformB,     1.0f },   // Pulse 50% (Osc B)
        { ParamIDs::ModOscBLevel,     0.6f },
        { ParamIDs::ModOscBDetune,    0.0f },
        { ParamIDs::ModVoices,        8.0f },
        { ParamIDs::ModAttack,        0.01f },
        { ParamIDs::ModDecay,         0.2f },
        { ParamIDs::ModSustain,       0.9f },
        { ParamIDs::ModRelease,       0.3f },
        { ParamIDs::ModUnison,        5.0f },
        { ParamIDs::ModDetune,        20.0f },
        { ParamIDs::ModVolume,        0.7f },
        { ParamIDs::VelocitySens,     0.3f },
        { ParamIDs::RvEnabled,        1.0f },
        { ParamIDs::RvSize,           0.3f },
        { ParamIDs::RvMix,            0.15f },
    }));

    // 34. Triangle Bells — Poly triangle with fast decay for bell-like tones
    presets.push_back (makePreset ("Triangle Bells", {
        { ParamIDs::EngineMode,       1.0f },   // Modern
        { ParamIDs::ModWaveform,      4.0f },   // Triangle
        { ParamIDs::ModVoices,        12.0f },
        { ParamIDs::ModAttack,        0.001f },
        { ParamIDs::ModDecay,         0.5f },
        { ParamIDs::ModSustain,       0.0f },
        { ParamIDs::ModRelease,       0.4f },
        { ParamIDs::ModVolume,        0.85f },
        { ParamIDs::DlEnabled,        1.0f },
        { ParamIDs::DlTime,           400.0f },
        { ParamIDs::DlFeedback,       0.4f },
        { ParamIDs::DlMix,            0.25f },
        { ParamIDs::RvEnabled,        1.0f },
        { ParamIDs::RvSize,           0.6f },
        { ParamIDs::RvMix,            0.3f },
    }));

    // 35. Noise Texture — Polyphonic noise with long attack for ambient
    presets.push_back (makePreset ("Noise Texture", {
        { ParamIDs::EngineMode,       1.0f },   // Modern
        { ParamIDs::ModWaveform,      6.0f },   // Noise
        { ParamIDs::ModVoices,        4.0f },
        { ParamIDs::ModAttack,        0.3f },
        { ParamIDs::ModDecay,         0.5f },
        { ParamIDs::ModSustain,       0.5f },
        { ParamIDs::ModRelease,       1.0f },
        { ParamIDs::ModVolume,        0.6f },
        { ParamIDs::FltEnabled,       1.0f },
        { ParamIDs::FltType,          0.0f },   // LP
        { ParamIDs::FltCutoff,        2000.0f },
        { ParamIDs::FltResonance,     1.0f },
        { ParamIDs::RvEnabled,        1.0f },
        { ParamIDs::RvSize,           0.8f },
        { ParamIDs::RvMix,            0.4f },
    }));

}

// ═══════════════════════════════════════════════════════════════════════════
//  Import / Export
// ═══════════════════════════════════════════════════════════════════════════

void PresetManager::exportPreset (const juce::String& name,
                                  juce::AudioProcessorValueTreeState& apvts,
                                  const juce::File& destFile)
{
    Preset p;
    p.name = name;

    for (const auto& [paramID, defaultVal] : defaults)
    {
        if (auto* param = apvts.getParameter (paramID))
            p.values.push_back ({ paramID, param->convertFrom0to1 (param->getValue()) });
    }

    savePresetToFile (p, destFile);
}

void PresetManager::exportBank (const juce::String& bankName,
                                const std::vector<int>& presetIndices,
                                const juce::File& destFile) const
{
    auto xml = std::make_unique<juce::XmlElement> ("CartridgeBank");
    xml->setAttribute ("name", bankName);

    for (int idx : presetIndices)
    {
        if (idx < 0 || idx >= static_cast<int> (presets.size()))
            continue;

        const auto& preset = presets[static_cast<size_t> (idx)];
        auto* presetEl = xml->createNewChildElement ("CartridgePreset");
        presetEl->setAttribute ("name", preset.name);

        if (preset.category.isNotEmpty())
            presetEl->setAttribute ("category", preset.category);

        for (const auto& [paramID, value] : preset.values)
        {
            auto* paramEl = presetEl->createNewChildElement ("Param");
            paramEl->setAttribute ("id", paramID);
            paramEl->setAttribute ("value", static_cast<double> (value));
        }
    }

    xml->writeTo (destFile);
}

int PresetManager::importPreset (const juce::File& file)
{
    auto xml = juce::XmlDocument::parse (file);
    if (xml == nullptr)
        return -1;

    // Single preset file
    if (xml->hasTagName ("CartridgePreset"))
    {
        Preset p;
        p.name = xml->getStringAttribute ("name", file.getFileNameWithoutExtension());
        p.category = xml->getStringAttribute ("category", "");

        for (auto* paramEl : xml->getChildIterator())
        {
            if (paramEl->hasTagName ("Param"))
            {
                auto id = paramEl->getStringAttribute ("id");
                auto val = static_cast<float> (paramEl->getDoubleAttribute ("value"));
                p.values.push_back ({ id, val });
            }
        }

        // Save to user preset directory and reload
        auto destFile = getUserPresetDirectory().getChildFile (sanitizeFilename (p.name) + ".xml");
        savePresetToFile (p, destFile);
        refreshUserPresets();
        return static_cast<int> (presets.size()) - 1;
    }

    // Bank file — delegate to importBank
    if (xml->hasTagName ("CartridgeBank"))
    {
        int count = importBank (file);
        return count > 0 ? static_cast<int> (presets.size()) - 1 : -1;
    }

    return -1;
}

int PresetManager::importBank (const juce::File& file)
{
    auto xml = juce::XmlDocument::parse (file);
    if (xml == nullptr || ! xml->hasTagName ("CartridgeBank"))
        return 0;

    juce::String bankName = xml->getStringAttribute ("name", file.getFileNameWithoutExtension());
    int imported = 0;

    for (auto* presetEl : xml->getChildIterator())
    {
        if (! presetEl->hasTagName ("CartridgePreset"))
            continue;

        Preset p;
        p.name = presetEl->getStringAttribute ("name", "Imported " + juce::String (imported + 1));
        p.category = presetEl->getStringAttribute ("category", bankName);

        // If no category was set on the preset, use the bank name
        if (p.category.isEmpty())
            p.category = bankName;

        for (auto* paramEl : presetEl->getChildIterator())
        {
            if (paramEl->hasTagName ("Param"))
            {
                auto id = paramEl->getStringAttribute ("id");
                auto val = static_cast<float> (paramEl->getDoubleAttribute ("value"));
                p.values.push_back ({ id, val });
            }
        }

        auto destFile = getUserPresetDirectory().getChildFile (sanitizeFilename (p.name) + ".xml");
        savePresetToFile (p, destFile);
        ++imported;
    }

    if (imported > 0)
        refreshUserPresets();

    return imported;
}

std::vector<juce::String> PresetManager::getUserCategories() const
{
    std::vector<juce::String> cats;

    for (size_t i = factoryCount; i < presets.size(); ++i)
    {
        const auto& cat = presets[i].category;
        if (cat.isNotEmpty())
        {
            // Add only if not already present (preserve order)
            bool found = false;
            for (const auto& c : cats)
            {
                if (c == cat) { found = true; break; }
            }
            if (! found)
                cats.push_back (cat);
        }
    }

    return cats;
}

const Preset* PresetManager::getPreset (int index) const
{
    if (index >= 0 && index < static_cast<int> (presets.size()))
        return &presets[static_cast<size_t> (index)];
    return nullptr;
}

int PresetManager::getPresetEngineMode (int index) const
{
    if (auto* p = getPreset (index))
    {
        for (const auto& [paramID, value] : p->values)
        {
            if (paramID == ParamIDs::EngineMode)
                return value >= 0.5f ? 1 : 0;
        }
    }
    return 0;  // Default to Classic
}

} // namespace cart
