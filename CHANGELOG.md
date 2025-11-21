## [0.2.7.25] - 2025-11-21

### Fixed
- **VGM Playback Timing**: Corrected VGM playback speed on RC2014 to match proper tempo (128 BPM target)
  - Implemented proportional timing formula: `vgmfdly = ((CPU_KHz * 2) / 1024) * 11/16`
  - Timing now automatically scales correctly for all CPU speeds (Z80, Z180, faster systems)
  - Empirically tuned for RC2014 @ 7.3728 MHz (vgmfdly ≈ 10)
  - Tested with penguin.vgm: playback tempo now matches PC VGM player

- **RC2014 Crash on Startup**: Removed heap clear operation that caused "BAD INT" crashes
  - Large LDIR operation (50KB) was triggering interrupt issues on RC2014 7.37MHz Z80
  - Heap clearing is unnecessary in CP/M as memory starts in clean state
  - PT3/PT2/MYM playback continues to work correctly without heap clear

### Technical Notes
- VGM timing formula derived empirically: target 128 BPM on penguin.vgm
- Formula uses 11/16 multiplier for optimal balance across CPU speeds
- Minimum vgmfdly value set to 5 for very slow CPUs
- No impact on PT3/PT2/MYM playback which uses different timing mechanism

# Changelog

All notable changes to VibeTune will be documented in this file.

## [0.2.7.3] - 2025-11-18

### Added
- **Dual AY chip detection** in VGM display
  - Shows "2x AY-3-8910" for files using dual chips
  - Scans first 64 bytes of command stream with early exit optimization
  - Detects chip 2 by finding 0xA0 commands with register bit 7 set

### Changed
- Optimized chip detection for minimal startup delay

## [0.2.7.2] - 2025-11-18

### Added
- **Configurable dual AY chip support** for VGM playback
  - Added RSEL2/RDAT2 to config table structure (now 11 bytes per entry)
  - Secondary AY chip ports now configurable via PORTS2 variables
  - VGM chip 2 writes now use `LD DE,(PORTS2)` for dynamic port addressing
  - Infrastructure for dual AY card configurations (e.g., 0xA0/0xA1 + 0xA8/0xA9)
  - All config entries default to $FF (no chip 2) for backward compatibility

### Fixed
- **VGM_MUTE_ALL** now uses proper port indirection for AY chips
  - AY mute correctly silences both chips when exiting playback
  - No more hanging notes when aborting AY VGM files

## [0.2.7.0] - 2025-11-18

### Added
- **VGM AY-3-8910 playback fully working!**
  - Fixed AY port writes to use proper indirection via `LD DE,(PORTS)`
  - AY register writes now go to actual chip ports (0xA0/0xA1) not RAM addresses
  - Tiger.vgm and penguin.vgm play correctly with looping support
  - OPL2/OPL3 playback continues working with volume boost and timing

### Fixed
- Critical AY port addressing bug - was writing to RAM address 0x22 instead of chip port 0xA0
- Now uses PT3-style port loading: `LD DE,(PORTS)` then `OUT (C),A`

## [0.2.6.48] - 2025-11-18

### Added
- **VGM info display** with adplay-style formatting
  - Shows "Playing 'filename'..." with actual VGM filename
  - Shows "Type : VGM (chip)" with detected chip type
  - Supports OPL2, OPL3, AY-3-8910, SN76489 detection

### Fixed
- Chip detection order (OPL2 → OPL3 → AY → PSG) prevents misidentification

## [0.2.6.47] - 2025-11-17

### Added
- **VGM OPL volume boost** - carrier operators only, zero attenuation
  - Targets registers 0x43,0x44,0x45,0x4B,0x4C,0x4D,0x53,0x54,0x55
  - Maximum volume with no distortion

### Changed
- VGM playback feature complete with perfect volume and timing

## [0.2.6.46] - 2025-11-17

### Added
- VGM volume boost with carrier-only targeting

## [0.2.6.45] - 2025-11-17

### Added
- **VGM timing perfected** for 18.432 MHz CPU
  - Dynamic frame delay: `(CPU_KHz / 8) * 1.125 * 4`
  - Playback timing matches vgmplay reference

## [0.2.6.44] - 2025-11-17

### Added
- VGM playback working (low volume, needs boost)

## [0.2.6.41-43] - 2025-11-17

### Added
- VGM OPL playback implementation with timing adjustments

## [0.2.6.35] - 2025-11-15

### Added
- **D00 detection complete** - Hardware verified on RC2014

## [0.2.6.34] - 2025-11-15

### Added
- **VGM detection complete** - Hardware verified on RC2014

## [0.2.6.8] - 2025-10-07

### Added
- MAME verified build
- Binary size: 5,160 bytes

## [0.2.6] - 2025-10-07

### Added
- **Complete Include File Modularization**
  - Organized all `.inc` files into logical structure
  - Created `src/system/`, `src/utils/`, `src/cli/` directories
  - Clean root directory with proper modular organization

## [0.2.5] - 2025-10-06

### Added
- UI messages modularization (`src/ui/messages.inc`)

### Attempted
- Audio engine modularization (failed - broke function references)

## [0.2.0] - 2025-10-05

### Added
- Modular file type identification system
- File type detection and configuration modules
- Dynamic build date in banner
- Enhanced build system with MAME deployment

## [0.1.0] - 2025-10-05

### Added
- Initial VibeTune release based on RomWBW Tune v3.13
- Support for PT2, PT3, and MYM formats
- Hardware auto-detection for AY-3-8910, YM2149
- Multi-platform support for RomWBW systems

### Changed
- Rebranded from "Tune Player" to "VibeTune"

## Future Plans

### In Progress
- SN76489 (PSG) playback testing

### Planned
- Enhanced multi-chip support
- Real-time audio controls
- Playlist functionality
- Additional VGM chip types (YM2612, YM2151)
