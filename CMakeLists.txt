cmake_minimum_required(VERSION 3.22)

if(APPLE)
    project(Cartridge VERSION 1.7.1 LANGUAGES C CXX OBJC OBJCXX)
else()
    project(Cartridge VERSION 1.7.1 LANGUAGES C CXX)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS deployment target")

# ─── JUCE ────────────────────────────────────────────────────────────────────
add_subdirectory(libs/JUCE)
add_subdirectory(libs/clap-juce-extensions EXCLUDE_FROM_ALL)

# ─── Plugin Target ───────────────────────────────────────────────────────────
juce_add_plugin(Cartridge
    COMPANY_NAME           "Cartridge"
    PLUGIN_MANUFACTURER_CODE Cart
    PLUGIN_CODE             Crt1
    FORMATS                 VST3 AU Standalone
    PRODUCT_NAME            "Cartridge"
    IS_SYNTH                TRUE
    NEEDS_MIDI_INPUT        TRUE
    NEEDS_MIDI_OUTPUT       FALSE
    IS_MIDI_EFFECT          FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    VST3_AUTO_MANIFEST      FALSE
)

# ─── CLAP Format ─────────────────────────────────────────────────────────────
clap_juce_extensions_plugin(TARGET Cartridge
    CLAP_ID         "com.cartridge.cartridge"
    CLAP_FEATURES   instrument synthesizer "chip tune" "8-bit"
)

# ─── Source Files ────────────────────────────────────────────────────────────
target_sources(Cartridge
    PRIVATE
        src/PluginProcessor.cpp
        src/PluginEditor.cpp
        src/Parameters.cpp
        src/PresetManager.cpp
        src/ui/ChannelStripComponent.cpp
        src/ui/TopBarComponent.cpp
        src/ui/ChannelStripArea.cpp
        src/dsp/Apu.cpp
        src/dsp/components/EnvelopeGenerator.cpp
        src/dsp/components/LengthCounter.cpp
        src/dsp/components/SweepUnit.cpp
        src/dsp/components/LinearCounter.cpp
        src/dsp/channels/PulseChannel.cpp
        src/dsp/channels/TriangleChannel.cpp
        src/dsp/channels/NoiseChannel.cpp
        src/dsp/channels/DpcmChannel.cpp
        src/dsp/DpcmSamples.cpp
        src/dsp/DpcmSampleManager.cpp
        src/dsp/channels/Vrc6PulseChannel.cpp
        src/dsp/channels/Vrc6SawChannel.cpp
        src/dsp/modern/ModernVoice.cpp
        src/dsp/modern/ModernEngine.cpp
        src/dsp/mixing/ApuMixer.cpp
        src/dsp/effects/BitCrush.cpp
        src/dsp/effects/Filter.cpp
        src/dsp/effects/Chorus.cpp
        src/dsp/effects/Delay.cpp
        src/dsp/effects/Reverb.cpp
        src/dsp/effects/EffectsChain.cpp
        src/ui/EffectsBarComponent.cpp
        src/ui/ModulationBarComponent.cpp
        src/ui/ModernPanelComponent.cpp
        src/ui/StatusBarComponent.cpp
        src/ui/WaveformDisplay.cpp
        src/midi/MidiVoiceManager.cpp
        src/midi/ModernVoiceManager.cpp
        src/midi/Arpeggiator.cpp
        src/ui/FullscreenHelper.cpp
)

if(APPLE)
    target_sources(Cartridge PRIVATE src/ui/FullscreenHelper.mm)
endif()

# ─── Include Paths ───────────────────────────────────────────────────────────
target_include_directories(Cartridge
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ─── JUCE Modules ────────────────────────────────────────────────────────────
target_link_libraries(Cartridge
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_dsp
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)

# ─── Compile Definitions ────────────────────────────────────────────────────
target_compile_definitions(Cartridge
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)
