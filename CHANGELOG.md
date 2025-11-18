# Changelog

All notable changes to VibeTune will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.0] - 2025-10-05

### Added
- Initial VibeTune release based on RomWBW Tune v3.13 (28-May-2025)
- Rebranded application banner and messages
- New versioning system (semantic versioning)
- Enhanced project structure with git repository
- Comprehensive README.md with feature roadmap
- Support for PT2, PT3, and MYM music file formats
- Hardware auto-detection for various sound chips (AY-3-8910, YM2149, etc.)
- Multi-platform support for RomWBW-compatible systems
- Command-line options for port forcing, timing modes, and octave adjustment

### Changed
- Application name from "Tune Player" to "VibeTune"
- Banner message to reflect version and heritage
- Usage message to show "VIBETUNE" command
- Build system adapted for standalone repository

### Technical Details
- Source forked from `RomWBW/Source/Apps/Tune`
- Maintains full compatibility with original Tune.com functionality
- Uses z88dk-compatible build system
- Successfully tested with z88dk-ticks emulator

## [0.2.0] - 2025-10-05

### Added 
- Modular file type identification system
- File type detection module (`src/io/filetype_detection.inc`)
- File type configuration module (`src/audio/filetype_config.inc`) 
- Clear extension points for adding new file formats (VGM, D00, etc.)
- Improved code organization and maintainability
- Dynamic build date in banner message (DD-MMM-YYYY format)
- Automatic build date injection during compilation
- Enhanced build system with emulator testing
- MAME deployment automation
- Comprehensive make targets (test, deploy, release, help, clean)

### Changed
- Restructured file type handling for better modularity
- Reduced main program complexity through modularization
- Fixed include order dependency issues

### Fixed
- Console I/O functionality after modularization
- Proper function definition ordering

## [0.2.6] - 2025-10-07

### Added
- **SUCCESS**: Complete Include File Modularization
  - Organized all `.inc` files from root directory into logical modular structure
  - Created new directories: `src/system/`, `src/utils/`, `src/cli/`
  - Moved 7 include files to appropriate locations with updated paths
  - Incremental testing approach prevented regressions
  - Clean root directory with proper modular organization

### Changed
- **Include Structure**: All includes now use modular paths
  - `src/system/`: `hbios.inc`, `cpm.inc` (system interface files)
  - `src/utils/`: `printing.inc`, `strings.inc`, `timing.inc` (utility functions)
  - `src/cli/`: `cli.inc` (command-line processing)
  - `src/tune.inc`: Application-specific macros
  - `src/ui/messages.inc`: UI messages (already modularized)

### Technical
- Version updated to 0.2.6 with build date 07-Oct-2025
- Binary size maintained at 5,028 bytes (no functionality changes)
- All tests pass, full compatibility preserved
- Incremental approach: Test after each file move (successful pattern)

## [0.2.5] - 2025-10-06

### Attempted
- **FAILED**: Audio engine modularization (PTx and MYM players)
  - Attempted to extract PTx Player Engine (~1,626 lines) and MYM Player Engine (~326 lines) into separate modules
  - Successfully reduced main file from 2,749 lines to ~701 lines (74% reduction)
  - Build completed successfully with no warnings or errors
  - **Critical failure**: Program crashes with "BAD INT" errors during audio playback
  - Root cause: Broken function references and memory layout dependencies after code extraction
  - The audio players contain essential functions with specific entry points that other parts depend on
  - Cross-module function calls not properly resolved, causing infinite loop at address @054B

### Added
- **SUCCESS**: UI messages modularization
  - Properly integrated `src/ui/messages.inc` into main build
  - Removed duplicate messages from main file (22 lines eliminated)
  - Clean separation of UI strings from core application logic
  - All application messages now centralized in modular file

### Fixed
- Reverted to working monolithic version (vibetune_original.asm) after audio engine modularization failure
- Implemented safer UI modularization approach
- Maintained all functionality and compatibility
- Updated version number to reflect the attempts and lessons learned

### Lessons Learned
- Audio engine modularization requires deeper analysis of:
  - Function call dependencies and entry points
  - Memory layout and address dependencies  
  - Cross-module reference resolution
  - Assembler limitations for complex modularization
- Future modularization attempts need systematic approach to preserve function interfaces

## [Unreleased]

### Planned
- Additional sound file format support (VGM, D00, etc.)
- Enhanced user interface and playback controls
- Multi-sound card management improvements
- Real-time audio controls (volume, tempo, channel muting)
- Playlist and looping functionality
- **Deferred**: Audio engine modularization (requires more careful architectural analysis)
- v0.2.6.8 (2025-10-07) - MAME verified build
- v0.2.6.34 (2025-11-15) - VGM detection complete - Hardware verified on RC2014
- v0.2.6.35 (2025-11-15) - D00 detection complete - Hardware verified on RC2014
