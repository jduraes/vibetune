# VibeTune v0.2.8.0

VibeTune is an enhanced music player for RomWBW, evolved from the original Tune.com application. This project aims to create a more advanced and feature-rich sound player that supports multiple sound file formats and multiple sound cards.

**Current Version**: v0.2.8.0 - Based on RomWBW Tune v3.13 (28-May-2025)

## Version Numbering

VibeTune follows semantic versioning (major.minor.patch):
- **Major**: Breaking changes or significant feature additions
- **Minor**: New features, backwards compatible
- **Patch**: Bug fixes and small improvements

## Features

### Core Functionality
- **Sound File Formats**: PT2, PT3, MYM, and VGM files
- **Sound Chips**: AY-3-8910, YM2149, OPL2/OPL3, SN76489, and compatible chips
- **Multiple Platforms**: Supports various RomWBW-compatible systems
- **Hardware Detection**: Automatic sound hardware detection and configuration
- **Multiple Sound Modules**: Support for various sound expansion cards
- **Adaptive Timing**: Automatic CPU speed detection and compensation (7-20+ MHz)
- **Adaptive Timing**: Automatic CPU speed detection and compensation (7-20+ MHz)

### Architecture (v0.2.0+)
- **Modular Design**: Clean separation of file type detection, audio configuration, and UI
- **Extensible**: Easy addition of new file formats (VGM, D00, etc.)
- **Organized Codebase**: Logical module structure in `src/` directories
- **Maintainable**: Separated constants, messages, and hardware definitions

## Usage

```
VIBETUNE <filename>.[PT2|PT3|MYM|VGM] [-msx|-rc] [-delay] [--hbios] [+tn|-tn]
```

### Parameters

- `<filename>`: Sound file to play (PT2, PT3, MYM, or VGM format)
- `-msx`: Force MSX standard ports (A0H/A1H)
- `-rc`: Force RCBus standard ports (D8H/D0H)
- `-delay`: Force delay mode timing
- `--hbios`: Use HBIOS sound driver
- `+tn/-tn`: Octave adjustment (up/down by n octaves)

## Building

VibeTune includes an enhanced build system with testing and deployment automation.

### Basic Build
```bash
make vibetune.com    # Build the executable
```

### Testing & Deployment
```bash
make test           # Run emulator test with z88dk-ticks
make release        # Build + test + deploy in one command
```

### Other Commands
```bash
make help           # Show all available targets
make clean          # Remove build artifacts
```

### MAME Testing
The build system automatically deploys VibeTune to your MAME FATDISK when you run `make deploy` or `make release`. Sample music files are also copied for testing:
- `Attack.pt3` - PT3 format demo
- `Demo.mym` - MYM format demo
- `penguin.vgm` - VGM format demo

In MAME (CP/M system), you can test with:
```
VIBETUNE ATTACK.PT3
VIBETUNE DEMO.MYM
VIBETUNE PENGUIN.VGM
```

## Development Roadmap

Future enhancements planned for VibeTune:

1. **Additional Sound Formats**: Support for more music file formats
2. **Enhanced Sound Card Support**: Better multi-card detection and management
3. **Improved User Interface**: Enhanced playback controls and information display
4. **Advanced Playback Features**: Looping, playlist support, fade effects
5. **Real-time Controls**: Volume control, tempo adjustment, channel muting

## Technical Notes

- **Binary size**: 6,143 bytes (optimized with modular architecture)
- **CPU Speed Support**: Adaptive timing for 7-20+ MHz systems (tested on RC2014 @ 7.37 MHz and SC126 @ 18.43 MHz)
- **Optimal PSG clock**: ~1.77MHz (similar to MSX/ZX Spectrum standards)
- **Memory requirement**: Loads files up to available heap space (typically ~48KB)
- **Timer support**: Uses hardware timers when available, falls back to CPU delay loops
- **Architecture**: Modular design with separated concerns for easy extension
- **VGM Timing**: Automatically scales with CPU speed using adaptive multipliers (11/16 for slow CPUs, 7/6 for fast CPUs)
- **VGM Timing**: Automatically scales with CPU speed using adaptive multipliers (11/16 for slow CPUs, 7/6 for fast CPUs)

## Origins

VibeTune is based on the original Tune.com from RomWBW, which incorporates:

- **PTx Player**: Universal PT2/PT3 player by S.V.Bulba
- **MYM Player**: MYM player by Marq/Lieves!Tuore
- **VGM Player**: VGM player by J.B. Langston, Marco Maccaferri, Ed Brindley
- **RomWBW Integration**: Hardware abstraction and platform support by Wayne Warthen

## License

GNU GPL v3 (inherited from original Tune.com)

## Documentation

For comprehensive project information:

- **[PROJECT_STATUS.md](PROJECT_STATUS.md)** - Complete development status, objectives, and roadmap
- **[CONTRIBUTING.md](CONTRIBUTING.md)** - Development guide and contribution workflow  
- **[CHANGELOG.md](CHANGELOG.md)** - Version history and release notes

## Contributing

This project is in active development. See [CONTRIBUTING.md](CONTRIBUTING.md) for development setup and workflow. Contributions and suggestions are welcome!
