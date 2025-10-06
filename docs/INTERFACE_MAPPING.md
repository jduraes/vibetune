# VibeTune Interface Mapping

**Analysis Date**: 2025-10-06  
**Purpose**: Document function interfaces and dependencies for safe modularization  
**Source**: `vibetune.asm` (2,559 lines)

## 📋 Current State Analysis

### Phase 2.1 Progress
- ✅ Git branch: `phase2-modularization` 
- ✅ Directory structure: `src/{core,io,hardware,audio,ui}/`
- ✅ Backup: `src/vibetune_original.asm`
- 🔄 **Partial extractions completed:**
  - `src/ui/messages.inc` - UI messages and strings
  - `src/hardware/constants.inc` - Hardware constants and configuration
  - `src/io/filetypes.inc` - File type detection constants
  - `src/io/filetype_detection.inc` - File type detection logic
  - `src/audio/filetype_config.inc` - Audio format configuration

### Critical Finding: Version Inconsistency
- `VERSION` file: v0.2.5
- `src/ui/messages.inc` MSGBAN: v0.2.0 ⚠️ **NEEDS UPDATE**

## 🔄 Main Program Flow Analysis

Based on `vibetune.asm` lines 84-200, here's the main program flow:

### 1. Initialization Sequence (Lines 84-94)
```asm
PRTCRLF                     ; Print newline
PRTSTRDE(MSGBAN)           ; Print banner → ui/messages.inc
CALL CLI_ABRT_IF_OPT_FIRST ; CLI parsing → core module candidate
CALL CLI_PORTS             ; Port parsing → core module candidate  
CALL CLI_HAVE_HBIOS_SWITCH ; HBIOS switch → core module candidate
CALL CLI_HAVE_DELAY_SWITCH ; Delay switch → core module candidate
CALL CLI_OCTAVE_ADJST      ; Octave adjustment → core module candidate
JP CONTINUE                ; Continue main flow
```

### 2. BIOS Validation (Lines 96-104)
```asm
CALL IDBIO                 ; Identify BIOS → hardware module
CP 1                       ; Check for RomWBW HBIOS
JP NZ, ERRBIO             ; Error handling → ui module
; Version checking logic
```

### 3. Hardware Configuration (Lines 106-175)
```asm
; Either forced port selection or auto-detection
; Hardware probing and sound chip detection
; Platform matching from config table
```

### 4. File Processing (Lines 198-200+)
```asm
CALL DETECT_FILE_TYPE      ; File type detection → io module
```

## 🎯 Key Interface Points

### CLI Module Interface (Core)
**Functions to extract:**
- `CLI_ABRT_IF_OPT_FIRST` - Command line abort check
- `CLI_PORTS` - Port option parsing  
- `CLI_HAVE_HBIOS_SWITCH` - HBIOS option parsing
- `CLI_HAVE_DELAY_SWITCH` - Delay option parsing
- `CLI_OCTAVE_ADJST` - Octave adjustment parsing

**Dependencies:**
- Uses hardware constants for port definitions
- Uses UI messages for error reporting
- Modifies global state variables

### Hardware Module Interface
**Functions to extract:**
- `IDBIO` - BIOS identification
- `SLOWIO`/`NORMIO` - I/O speed control
- `PROBETIMER` - Timer detection
- Hardware probing and configuration logic

**Dependencies:**
- Hardware constants (already extracted to `src/hardware/constants.inc`)
- Global configuration variables
- Platform-specific settings

### File I/O Module Interface  
**Functions to extract:**
- `DETECT_FILE_TYPE` - File type detection (partially extracted)
- File loading and validation routines
- CP/M BDOS interface functions

**Dependencies:**
- File type constants (already extracted)
- Memory management
- Error handling messages

### UI Module Interface
**Already Extracted:**
- Message constants in `src/ui/messages.inc`

**Still Needed:**
- Error display functions (`ERRBIO`, `ERRHW`, etc.)
- Status display functions
- Banner and information display

## 🛠️ Safe Extraction Strategy

### Phase 2.2: Update Extracted Files (CRITICAL)
1. **Fix version inconsistency** in `src/ui/messages.inc` (v0.2.0 → v0.2.5)
2. **Verify extracted constants** match current `vibetune.asm`
3. **Test compilation** with existing extracted files

### Phase 2.3: Extract CLI Module (Medium Risk)
**Target:** Lines with CLI_* functions
**Interface:** Clear parameter passing, well-defined boundaries
**Risk Level:** Medium - affects command line processing

### Phase 2.4: Extract Hardware Module (Medium Risk)  
**Target:** Hardware detection and configuration logic
**Interface:** Uses configuration tables and constants
**Risk Level:** Medium - critical for hardware compatibility

### Phase 2.5: Extract File I/O Module (Low-Medium Risk)
**Target:** File operations and type detection
**Interface:** Standard CP/M BDOS calls
**Risk Level:** Low-Medium - well-bounded functionality

## ⚠️ Critical Dependencies

### Global Variables (Shared State)
These variables are used across modules and need careful handling:
- `CURPLT` - Current platform ID
- `HBIOSMD` - HBIOS mode flag
- `USEPORTS` - Port selection override
- `CFG` - Active hardware configuration
- `QDLY` - Quark delay factor

### Include File Order
Current include order in `vibetune.asm`:
1. `../RomWBW/Source/ver.inc`
2. `hbios.inc`
3. `cpm.inc`
4. `tune.inc`

**Must preserve this order** when integrating extracted modules.

## 📋 Next Steps for Phase 2.1 Completion

1. **Fix version numbers** in extracted files
2. **Verify consistency** between extracted files and main source
3. **Create integration test** to ensure extracted files work
4. **Document any missing dependencies**

## 🔍 Risk Assessment

### Current Extraction Status
- **Low Risk:** Message constants (completed)
- **Low Risk:** Hardware constants (completed, needs verification)
- **Medium Risk:** File type detection (partially completed)
- **High Risk:** Core program logic (not started)
- **High Risk:** Audio engines (not touched)

The foundation is good, but **version consistency** and **testing integration** are critical before proceeding to Phase 2.2.