# VibeTune Code Architecture Analysis

**Analysis Date**: 2025-10-05  
**Version**: v0.1.0  
**Source File**: `vibetune.asm` (2,749 lines)

## 📊 Code Statistics

- **Total Lines**: 2,749
- **Code Lines**: 2,364 (86%)
- **Comment Lines**: 385 (14%)  
- **Binary Size**: 5,028 bytes
- **Architecture**: Well-structured monolithic assembly

## 🏗️ Major Code Sections

### 1. Main Program (Lines 63-749)
- **Purpose**: Core application logic and initialization
- **Key Functions**:
  - Command line parsing
  - Hardware detection and configuration  
  - File loading and type detection
  - BIOS compatibility checking
- **Size**: ~686 lines
- **Modularization Potential**: HIGH

### 2. PTx Player Routines (Lines 750-2373)
- **Purpose**: Universal PT2/PT3 player by S.V.Bulba
- **Key Functions**:
  - Pattern decoding and playback
  - Note and volume table generation
  - Sound register manipulation
- **Size**: ~1,623 lines (59% of codebase)
- **Modularization Potential**: MEDIUM (complex interdependencies)

### 3. MYM Player Routines (Lines 2374-2667)
- **Purpose**: MYM player by Marq/Lieves!Tuore
- **Key Functions**:
  - MYM format decompression
  - Fragment extraction and playback
  - PSG register updates
- **Size**: ~293 lines
- **Modularization Potential**: HIGH

### 4. Shared Heap Storage (Lines 2668-2748)
- **Purpose**: Memory allocation for player data
- **Key Functions**:
  - Variable definitions for PTx and MYM
  - Memory layout management
- **Size**: ~80 lines
- **Modularization Potential**: LOW (shared resources)

## 🎯 Identified Functional Modules

Based on the analysis, the following logical modules emerge:

### Core System (`core/`)
**Lines**: 63-320 (~257 lines)
- Command line interface
- BIOS detection and version checking
- Hardware configuration selection
- Error handling and messaging

### File Management (`io/`)
**Lines**: 224-306 (~82 lines)
- File type detection (.PT2/.PT3/.MYM)
- File loading and validation  
- CP/M BDOS interface
- Memory management for file data

### Hardware Abstraction (`hardware/`)
**Lines**: 150-223, 506-583 (~150 lines)
- Sound chip detection and configuration
- Hardware platform identification
- I/O port management
- Timing and CPU speed handling

### Audio Engines (`audio/`)
#### PTx Engine
**Lines**: 750-2373 (~1,623 lines)
- PT2/PT3 format support
- Pattern decoding
- Note/volume table generation
- Audio playback logic

#### MYM Engine  
**Lines**: 2374-2667 (~293 lines)
- MYM format decompression
- Fragment-based playback
- PSG register management

### User Interface (`ui/`)
**Lines**: 308-372, 713-747 (~100 lines)
- Banner and version display
- Status messages and error reporting
- Song information display
- Usage instructions

## 🔄 Safe Modularization Strategy

### Phase 2.1: Preparation (Non-invasive)
1. ✅ **Code Analysis** - Complete (this document)
2. **Create Module Directories** - Prepare structure
3. **Document Interfaces** - Map function calls between sections
4. **Backup Original** - Preserve working baseline

### Phase 2.2: Extract Utilities (Low Risk)
1. **String/Message Module** - Extract all message constants
2. **Constants Module** - Move hardware definitions and equates
3. **Macros Module** - Extract common assembly macros
4. **Test Each Step** - Validate functionality after each change

### Phase 2.3: Extract Self-Contained Modules (Medium Risk)
1. **MYM Player Module** - Relatively isolated (293 lines)
2. **Hardware Detection Module** - Clear boundaries
3. **File I/O Module** - Well-defined BDOS interactions
4. **Test Thoroughly** - Validate all file formats work

### Phase 2.4: Extract Core Modules (Higher Risk)
1. **PTx Player Module** - Large, complex interdependencies
2. **Main Program Logic** - Central orchestration
3. **Integration Testing** - Comprehensive validation

## 🚨 Risk Assessment

### Low Risk Extractions
- **Message strings and constants** (no logic changes)
- **Hardware configuration tables** (data-only)
- **MYM player routines** (relatively isolated)

### Medium Risk Extractions  
- **File I/O routines** (clear interfaces but system-critical)
- **Hardware detection logic** (well-bounded but essential)
- **Error handling routines** (used throughout)

### High Risk Extractions
- **PTx player engine** (large, complex, performance-critical)
- **Memory management** (shared between all modules)
- **Main program flow** (orchestrates everything)

## 📋 Recommended First Steps

### Step 1: Create Safe Test Environment
```bash
git checkout -b phase2-analysis
mkdir -p src/{core,io,hardware,audio,ui}
cp vibetune.asm src/vibetune_original.asm  # Backup
```

### Step 2: Extract Constants (Safest Start)
- Move hardware port definitions to `hardware/ports.inc`
- Move message strings to `ui/messages.inc`  
- Move file type constants to `io/filetypes.inc`
- Test: `make test` should pass identically

### Step 3: Extract MYM Player (Self-Contained)
- Move MYM routines to `audio/mym_player.asm`
- Create clean interface for main program
- Test: MYM files should play identically

### Step 4: Validate and Document
- Ensure all tests pass
- Document interface changes
- Update build system for multiple files
- Commit incremental progress

## ✅ Success Criteria

Each modularization step must pass:
1. **Build Success** - `make vibetune.com` completes
2. **Emulator Test** - `make test` shows identical banner/behavior  
3. **Functional Test** - All file formats play correctly
4. **Size Validation** - Binary size remains similar (±50 bytes)
5. **Performance** - No significant timing changes

This approach ensures VibeTune maintains functionality while gradually becoming more modular and maintainable.

## ✅ Completed Modularization (Phase 2.2 - 2.3)

### Phase 2.2: Constants and Messages (Completed)
- **`src/ui/messages.inc`** - All user interface and error messages
- **`src/io/filetypes.inc`** - File type constants and port selections
- **`src/hardware/constants.inc`** - Hardware constants and conditional assembly flags

### Phase 2.3: File Type System (Completed)
- **`src/io/filetype_detection.inc`** - File type identification logic
  - `DETECT_FILE_TYPE` function: Analyzes FCB filename/extension and sets FILTYP
  - Handles .PT2, .PT3, .MYM extensions with default PT3 fallback
  - 57 lines extracted from main program

- **`src/audio/filetype_config.inc`** - File-specific audio configuration and playback
  - `CONFIGURE_FILE_LOAD_ADDRESS` function: Sets appropriate DMA load address
  - `CONFIGURE_FILE_PLAYBACK` function: Complete file-specific player dispatch
  - Includes all PT2/PT3/MYM timing configurations and player loops
  - 120 lines extracted from main program

**Benefits Achieved:**
- Clear separation of file type identification and audio configuration
- Easy extension point for adding new file formats (VGM, D00, etc.)
- Main program reduced to high-level orchestration calls
- All file type logic is now contained in logical modules

**Current Status:**
- Main file: 2,558 lines (reduced from original 2,749)
- Modular includes: 175 lines (detection + configuration)
- Binary size: 5,039 bytes (11 bytes increase due to function call overhead)
- Functionality: ✅ Identical behavior validated via emulator testing
