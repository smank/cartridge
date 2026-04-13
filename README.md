# Cartridge

Chiptune synthesizer plugin emulating the Ricoh 2A03 APU with optional VRC6 Konami expansion. Available as VST3, AU, CLAP, and Standalone.

## Features

### Classic Engine (2A03)
- **2 Pulse channels** — PolyBLEP antialiasing, 4 duty cycles (12.5%, 25%, 50%, 75%), sweep unit, envelope generator
- **Triangle channel** — 32-step waveform, linear counter
- **Noise channel** — 15-bit LFSR, long/short mode, 16 period presets (NTSC/PAL), envelope generator
- **DPCM channel** — 4 built-in drum samples (Kick, Snare, Hi-Hat, Tom) + 16 user sample slots (.wav import), note-to-sample mapping
- **VRC6 Konami expansion** — 2 Pulse (8-level duty), 1 Sawtooth (accumulator-based), individual per-channel enable toggles
- **Nonlinear NES DAC mixing** — authentic lookup tables matching hardware output curves
- **Per-channel stereo panning** — constant-power pan law on all 8 channels

### Modern Engine
- **Dual oscillator** — Osc A + Osc B with independent chiptune waveform selection
- **Osc B controls** — level, detune, enable/disable
- **Polyphonic** — configurable voice count
- **ADSR envelope** — attack, decay, sustain, release
- **Unison** — configurable voice count with detune spread
- **Portamento** — enable/disable with adjustable glide time
- **Velocity-to-filter** modulation

### Step Sequencer
- **16-step per-channel sequencer** — FamiTracker-style instrument macros
- **3 lanes** — Volume (0–15), Pitch (-12 to +12 semitones), Duty cycle
- **Per-channel** — each of the 8 Classic channels has independent sequence data
- **Modern engine support** — channel 0 sequencer drives global polyphonic engine
- **Loop mode** — per-lane loop toggle; non-looping sequences hold last value
- **Tempo sync** — syncs to host BPM with 9 division options (1/1 through 1/16T)
- **Triggers on note-on** — sequences reset and play from step 0 on each new note
- **Interactive grid UI** — click/drag to draw, right-click to reset, beat group markers
- **Serialized in presets** — step data saved/loaded alongside APVTS state

### VGM Import
- **Import .vgm/.vgz files** — extract instrument envelopes from Video Game Music files
- **NES APU register analysis** — tracks per-channel volume, pitch, and duty cycle changes
- **Automatic instrument extraction** — detects note boundaries, captures attack envelopes
- **Deduplication** — skips identical instruments, auto-names by character (Lead, Slide, Morph)
- **GD3 tag parsing** — uses game/track name for preset naming
- **One preset per instrument** — saved with embedded step sequence data, categorized as "VGM: GameName"

### MIDI
- **4 modes** — Split (fixed channel routing), Auto (round-robin polyphony across all enabled channels), Mono (single voice), Layer (all enabled channels play every note)
- **Layer mode** — thick chiptune stacks: every enabled channel fires on each keypress with its own timbre
- **Split mode routing** — Ch1→Pulse 1, Ch2→Pulse 2, Ch3→Triangle, Ch10→Noise, Ch4→DPCM, Ch5→VRC6 P1, Ch6→VRC6 P2, Ch7→VRC6 Saw
- **MIDI Learn** — right-click any custom CC knob → "Learn" → send a CC to auto-assign
- **MPE support** — per-voice pitch bend, slide, and pressure in Modern engine
- **Pitch bend** — configurable range (1–24 semitones)
- **Mod wheel (CC1)** — vibrato depth
- **Sustain pedal (CC64)** — hold notes
- **Expression (CC11)** — master volume
- **Custom CC mappings** — 4 user-configurable slots, each mapping any CC to one of 15 targets
- **Hardcoded CCs** — CC71 (filter resonance), CC74 (filter cutoff), CC91 (reverb mix), CC93 (chorus mix)

### Effects
- **Bitcrush** — bit depth and sample rate reduction with dry/wet mix
- **Filter** — LP/HP/BP with cutoff and resonance
- **Chorus** — rate, depth, mix
- **Delay** — time, feedback, mix (with optional DAW tempo sync)
- **Reverb** — room size, damping, width, mix

### Modulation
- **LFO** — vibrato (pitch) and tremolo (amplitude) with adjustable rate and depth
- **Arpeggiator** — Up, Down, Up/Down, Random, As Played patterns with rate, octave range, and gate control (with optional DAW tempo sync)
- **Portamento** — enable/disable with adjustable glide time
- **Tempo sync** — arp rate, delay time, and step sequencer rate can lock to DAW BPM

### Tuning
- **4 temperaments** — Equal, Just, Pythagorean, Meantone
- **NTSC/PAL region** — affects noise period and DPCM rate tables
- **Per-channel transpose** — semitone offset for melodic channels (Pulse 1/2, Triangle, VRC6 channels)
- **Master tune** — global fine-tuning in cents

### Presets
- **36 factory presets** — leads, bass, drums, FX, VRC6, arps, pads, and Modern engine presets
- **User presets** — save, delete, import (.cartpreset), export, bank import (.cartbank)
- **VGM import** — drag in .vgm/.vgz files to extract NES instrument envelopes as presets
- **A/B comparison** — toggle between two parameter snapshots
- **Engine-aware browsing** — preset list filters by Classic/Modern mode

### UI
- **Waveform display** — real-time oscilloscope with glow effect and launch animation
- **Live status bar** — active voice count, engine mode, MIDI mode, octave, velocity, sustain indicator
- **Channel activity LEDs** — per-channel note indicators with glow
- **FX & Mod bars** — accordion-style panels, mutually exclusive expansion
- **Tooltips** — hover any control for a description
- **Resizable** — 100%, 125%, 150%, 200% scaling with auto-detect on first launch
- **Fullscreen support** — content fills screen, macOS native fullscreen via green button
- **QWERTY-to-MIDI keyboard** — built-in on-screen keyboard with velocity control
- **Hold/latch mode** — notes sustain until toggled off

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `1`–`5` | Toggle APU channels (Pulse 1/2, Triangle, Noise, DPCM) |
| `6`–`8` | Toggle VRC6 channels (Pulse 1/2, Sawtooth) |
| `Left` / `Right` | Navigate presets |
| `Tab` / `Shift+Tab` | Cycle focused channel |
| `[` / `]` | Octave down / up |
| `-` / `=` | Velocity down / up |
| `\` | Toggle hold/latch mode |
| `Cmd+V` | Toggle VRC6 expansion |
| `Cmd+S` | Solo focused channel |
| `Cmd+M` | Mute focused channel |
| `Escape` | Unsolo (restore all mixes) |
| `Space` | All notes off (panic) |
| `Ctrl+Cmd+F` | Toggle fullscreen |

## Building from Source

Requires CMake 3.22+, a C++17 compiler, and Git. Uses [JUCE 8](https://juce.com/) and [clap-juce-extensions](https://github.com/free-audio/clap-juce-extensions) as submodules.

```bash
git clone --recursive https://github.com/smank/cartridge.git
cd cartridge
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Artifacts land in `build/Cartridge_artefacts/Release/{AU,CLAP,Standalone,VST3}`.

**macOS** — AU, VST3, CLAP, Standalone (CI builds are signed and notarized)
**Windows** — VST3, CLAP, Standalone
**Linux** — VST3, CLAP, Standalone (requires ALSA, X11, FreeType, WebKit2GTK, GL dev packages)

## Architecture

```
src/
├── dsp/
│   ├── channels/      Pulse, Triangle, Noise, DPCM, VRC6 Pulse, VRC6 Saw
│   ├── components/    Envelope, Length Counter, Sweep, Linear Counter
│   ├── effects/       Bitcrush, Filter, Chorus, Delay, Reverb
│   ├── mixing/        Nonlinear DAC mixer with per-channel stereo panning
│   └── modern/        Modern polyphonic engine (dual osc, ADSR, unison)
├── import/            VGM/VGZ parser, instrument extractor, preset generator
├── midi/              Voice managers (Classic split/auto/mono/layer, Modern poly),
│                      Arpeggiator, Tuning tables, CC routing, MIDI Learn
└── ui/                Top bar, Channel strips, Effects/Modulation bars,
                       Step sequencer grid, Modern panel, Waveform display, Status bar
```

## License

[GPL-3.0](LICENSE)
