#include "VgmImporter.h"
#include "VgmParser.h"
#include "VgmInstrumentExtractor.h"
#include "../PresetManager.h"
#include "../Parameters.h"
#include "../dsp/StepSequencer.h"

namespace cart {

namespace
{
    juce::String sanitize (const juce::String& name)
    {
        return name.replaceCharacters ("/\\:*?\"<>|", "---------")
                   .trimCharactersAtEnd (".")
                   .trim();
    }

    // Build a preset XML with step sequence data embedded
    std::unique_ptr<juce::XmlElement> buildPresetXml (
        const juce::String& name,
        const juce::String& category,
        const ExtractedInstrument& inst)
    {
        auto xml = std::make_unique<juce::XmlElement> ("CartridgePreset");
        xml->setAttribute ("name", name);
        if (category.isNotEmpty())
            xml->setAttribute ("category", category);

        // Helper to add a param
        auto addParam = [&] (const char* id, float value)
        {
            auto* el = xml->createNewChildElement ("Param");
            el->setAttribute ("id", id);
            el->setAttribute ("value", static_cast<double> (value));
        };

        // Global
        addParam (ParamIDs::MasterVolume, 0.8f);
        addParam (ParamIDs::MasterTune, 0.0f);
        addParam (ParamIDs::Region, 0.0f);
        addParam (ParamIDs::EngineMode, 0.0f); // Classic
        addParam (ParamIDs::MidiMode, 1.0f);   // Auto
        addParam (ParamIDs::VelocitySens, 0.0f);
        addParam (ParamIDs::PitchBendRange, 2.0f);

        // Enable only the extracted channel, disable others
        addParam (ParamIDs::P1Enabled,   inst.channelIndex == 0 ? 1.0f : 0.0f);
        addParam (ParamIDs::P2Enabled,   inst.channelIndex == 1 ? 1.0f : 0.0f);
        addParam (ParamIDs::TriEnabled,  inst.channelIndex == 2 ? 1.0f : 0.0f);
        addParam (ParamIDs::NoiseEnabled, inst.channelIndex == 3 ? 1.0f : 0.0f);
        addParam (ParamIDs::DpcmEnabled, 0.0f);
        addParam (ParamIDs::Vrc6Enabled, 0.0f);

        // Channel-specific settings
        switch (inst.channelIndex)
        {
            case 0: // Pulse 1
                addParam (ParamIDs::P1Duty, static_cast<float> (inst.duty));
                addParam (ParamIDs::P1Volume, static_cast<float> (inst.volume));
                addParam (ParamIDs::P1ConstVol, 1.0f);
                addParam (ParamIDs::P1Mix, 1.0f);
                break;
            case 1: // Pulse 2
                addParam (ParamIDs::P2Duty, static_cast<float> (inst.duty));
                addParam (ParamIDs::P2Volume, static_cast<float> (inst.volume));
                addParam (ParamIDs::P2ConstVol, 1.0f);
                addParam (ParamIDs::P2Mix, 1.0f);
                break;
            case 2: // Triangle
                addParam (ParamIDs::TriLinearReload, 127.0f);
                addParam (ParamIDs::TriLinearControl, 1.0f);
                addParam (ParamIDs::TriMix, 1.0f);
                break;
            case 3: // Noise
                addParam (ParamIDs::NoiseVolume, static_cast<float> (inst.volume));
                addParam (ParamIDs::NoiseConstVol, 1.0f);
                addParam (ParamIDs::NoiseMode, 0.0f);
                addParam (ParamIDs::NoiseMix, 1.0f);
                break;
        }

        // Step sequencer enabled at 60 Hz (NTSC frame rate)
        addParam (ParamIDs::SeqEnabled, 1.0f);
        addParam (ParamIDs::SeqRate, 60.0f);
        addParam (ParamIDs::SeqSyncEnabled, 0.0f);

        // Effects off
        addParam (ParamIDs::BcEnabled, 0.0f);
        addParam (ParamIDs::FltEnabled, 0.0f);
        addParam (ParamIDs::ChEnabled, 0.0f);
        addParam (ParamIDs::DlEnabled, 0.0f);
        addParam (ParamIDs::RvEnabled, 0.0f);

        // Modulation off
        addParam (ParamIDs::LfoEnabled, 0.0f);
        addParam (ParamIDs::ArpEnabled, 0.0f);
        addParam (ParamIDs::PortaEnabled, 0.0f);

        // Panning centered
        addParam (ParamIDs::P1Pan, 0.0f);
        addParam (ParamIDs::P2Pan, 0.0f);
        addParam (ParamIDs::TriPan, 0.0f);
        addParam (ParamIDs::NoisePan, 0.0f);

        // Embed step sequence data — map to the appropriate channel index
        // In Classic mode, channel 0-3 map to P1, P2, Tri, Noise
        const auto& sd = inst.seqData;
        bool hasData = sd.numVolumeSteps > 0 || sd.numPitchSteps > 0 || sd.numDutySteps > 0;

        if (hasData)
        {
            auto* seqEl = xml->createNewChildElement ("StepSeq");
            seqEl->setAttribute ("ch", inst.channelIndex);
            seqEl->setAttribute ("numVol", sd.numVolumeSteps);
            seqEl->setAttribute ("numPitch", sd.numPitchSteps);
            seqEl->setAttribute ("numDuty", sd.numDutySteps);
            seqEl->setAttribute ("volLoop", sd.volumeLoop);
            seqEl->setAttribute ("pitchLoop", sd.pitchLoop);
            seqEl->setAttribute ("dutyLoop", sd.dutyLoop);

            auto joinInts = [] (const int* arr, int count) -> juce::String
            {
                juce::String s;
                for (int i = 0; i < count; ++i)
                    s += (i > 0 ? "," : "") + juce::String (arr[i]);
                return s;
            };

            if (sd.numVolumeSteps > 0) seqEl->setAttribute ("vol",   joinInts (sd.volumeSteps, sd.numVolumeSteps));
            if (sd.numPitchSteps > 0)  seqEl->setAttribute ("pitch", joinInts (sd.pitchSteps, sd.numPitchSteps));
            if (sd.numDutySteps > 0)   seqEl->setAttribute ("duty",  joinInts (sd.dutySteps, sd.numDutySteps));
        }

        return xml;
    }
}

VgmImporter::Result VgmImporter::importFile (const juce::File& file, PresetManager& presetManager)
{
    Result result;

    // Parse VGM/VGZ
    VgmParser parser;
    if (!parser.parse (file))
    {
        result.error = "Failed to parse VGM file.\n\n"
                       "File: " + file.getFullPathName()
                       + "\nSize: " + juce::String (file.getSize()) + " bytes"
                       + "\n\nThe file may be corrupt or not a valid VGM/VGZ.";
        return result;
    }

    auto& hdr = parser.getHeader();

    if (!parser.hasNesApu())
    {
        result.error = "No NES APU data found.\n\n"
                       "VGM version: " + juce::String::toHexString (static_cast<int> (hdr.version))
                       + "\nNES clock: " + juce::String (hdr.nesApuClock)
                       + "\nCommands: " + juce::String (static_cast<int> (parser.getCommands().size()))
                       + "\n\nThis file may be for a different chip.";
        return result;
    }

    // Extract instruments
    VgmInstrumentExtractor extractor;
    uint32_t clock = hdr.nesApuClock;
    if (clock == 0) clock = 1789773; // Default NTSC
    extractor.extract (parser.getCommands(), clock);

    auto& instruments = extractor.getInstruments();
    if (instruments.empty())
    {
        result.error = "No instruments with envelope variation found.\n\n"
                       "The file contains " + juce::String (static_cast<int> (parser.getCommands().size()))
                       + " NES APU commands. Instruments are only extracted when\n"
                       "volume, pitch, or duty cycle changes during a note.";
        return result;
    }

    // Determine folder name from GD3 tags
    auto& gd3 = parser.getGd3();
    juce::String folderName;
    if (gd3.gameName.isNotEmpty())
        folderName = sanitize (gd3.gameName);
    else if (gd3.trackName.isNotEmpty())
        folderName = sanitize (gd3.trackName);
    else
        folderName = sanitize (file.getFileNameWithoutExtension());

    result.gameName = folderName;

    // Category prefix for organizing in the preset list
    juce::String category = "VGM: " + folderName;

    // Get the user preset directory and create a subfolder
    auto presetDir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                         .getChildFile ("Cartridge")
                         .getChildFile ("Presets");
    presetDir.createDirectory();

    // Save each instrument as a preset
    for (const auto& inst : instruments)
    {
        juce::String presetName = folderName + " - " + inst.name;
        auto xml = buildPresetXml (presetName, category, inst);

        auto destFile = presetDir.getChildFile (sanitize (presetName) + ".xml");
        xml->writeTo (destFile);
        ++result.numInstruments;
    }

    // Refresh preset list so the new presets appear
    presetManager.refreshUserPresets();

    result.success = true;
    return result;
}

} // namespace cart
