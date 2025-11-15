# VibeTune - Resume Work Document
**Created**: 2025-11-15  
**For**: Continuing VGM format extension debugging on a different computer  
**Status**: VGM architecture complete, detection bug needs fixing

---

## 🎯 CURRENT TASK: Fix VGM Detection Bug

**Problem**: VGM file extension detection works in debug builds but fails with "Sound filename invalid" error in clean builds.

**Goal**: Complete VGM format support following Wayne Warthen's dispatch pattern, verify in MAME, and commit.

---

## 📍 PROJECT STATE SNAPSHOT

### Version & Branch
- **Current Version**: `0.2.6.27`
- **Branch**: `phase2-modularization`
- **Binary Size**: ~5,158 bytes
- **Last Committed Tag**: `v0.2.6.8`

### Uncommitted Changes (DO NOT COMMIT YET)
7 modified files with VGM implementation:
- `PROJECT_STATUS.md`
- `README.md`
- `VERSION`
- `WARP.md`
- `src/audio/filetype_config.inc`
- `src/io/filetype_detection.inc`
- `src/ui/messages.inc`

2 untracked test files:
- `test.vgm`
- `test_vgm.sh`

### Critical WARP Rules (from WARP.md)
1. **NEVER commit until MAME verification is complete** ✅
2. Build number (x.y.z.**a**) auto-increments on every successful build+test
3. After MAME verification, run `make commit` (handles docs, tags, changelog)
4. Follow Wayne Warthen's file extension dispatch pattern for new formats
5. Maintain CP/M 8.3 filename conventions (letters preferred, lowercase OK)

---

## 🚀 STEP-BY-STEP RESUMPTION PLAN

### Phase 1: Environment Setup on New Computer

#### Step 1: Navigate and Verify Branch
```bash
cd /Users/mduraes/GitHub/vibetune
git rev-parse --abbrev-ref HEAD  # Must show: phase2-modularization
git --no-pager status             # Should show 7 modified, 2 untracked
cat VERSION                       # Should show: 0.2.6.27
ls -l vibetune.com                # Should be ~5158 bytes
```

#### Step 2: Verify Toolchain Exists
```bash
# Check z88dk
command -v zcc || echo "z88dk not found - install: brew install z88dk"
zcc -v

# Check RomWBW reference
ls ../RomWBW || echo "RomWBW not at ../RomWBW (adjust Makefile if needed)"

# Check MAME + FATDISK
command -v mame || echo "MAME not found - install: brew install mame"
ls /Volumes/FATDISK || echo "FATDISK not mounted"
```

#### Step 3: Re-orient with Project Documentation
**Read in this exact order** (15-20 minutes):
1. `WARP.md` - Development rules (CRITICAL)
2. `PROJECT_STATUS.md` - Current status and VGM debugging notes
3. `README.md` - Usage and supported formats
4. `CHANGELOG.md` - Version history
5. `CODE_ARCHITECTURE.md` - Module structure

**While reading, note:**
- The error message: "Sound filename invalid"
- Wayne's dispatch pattern for file type detection
- Versioning workflow: build → test → MAME verify → commit

#### Step 4: Run Analysis Script
```bash
bash analyze_code.sh
```
**Expected output highlights:**
- Total lines: ~2,599
- Binary size: ~5,158 bytes
- PTx Player: lines 595-2219 (~1,624 lines)
- MYM Player: lines 2219-2513 (~294 lines)

---

### Phase 2: Reproduce and Diagnose the Bug

#### Step 5: Reproduce Clean Build Failure
```bash
make clean
make vibetune.com
make test
```
**Expected**: Build succeeds, but VGM files trigger "Sound filename invalid" error

#### Step 6: Try Debug Build (if available)
```bash
make clean
make DEBUG=1 vibetune.com  # Or whatever debug target exists
make test
```
**Expected**: Debug build shows VGM detection working (prints "VGM!04T" or similar)

**Key Insight**: The bug is a **clean-build-only issue**, suggesting:
- Conditional assembly that excludes VGM in release builds
- An early validation check that wasn't updated for VGM
- A dispatch table index/count mismatch

#### Step 7: Locate the Error Message
```bash
grep -Rn "Sound filename invalid" src vibetune.asm
```
**Goal**: Find where this error is triggered. Most likely it's in:
- Early filename validation (before file type detection)
- File extension allowlist that needs VGM added

#### Step 8: Map the Execution Flow
**Trace the path from command line to error:**

1. **User runs**: `VIBETUNE TEST.VGM`
2. **CP/M FCB** is populated with filename and extension
3. **Early validation** checks if extension is in allowlist (PT2/PT3/MYM)
   - **HYPOTHESIS**: This is where VGM fails in clean builds
4. **File type detection** (`src/io/filetype_detection.inc`)
   - CHKVGM should run here
5. **File type configuration** (`src/audio/filetype_config.inc`)
   - CONFIG_VGM should dispatch here
6. **Playback** begins

**Find the early validation** and check if VGM is missing from the allowlist.

---

### Phase 3: Fix the Bug

#### Step 9: Most Likely Fix - Update Extension Allowlist

**Location to check**: `vibetune.asm` or file I/O validation code

Look for code like this:
```asm
; Check if extension is PT2, PT3, or MYM
; If not, jump to ERRFIL (filename invalid)
```

**Fix**: Add VGM to this allowlist check:
```asm
; Check if extension is PT2, PT3, MYM, or VGM
; If not, jump to ERRFIL (filename invalid)
```

**After editing:**
```bash
make vibetune.com
make test
```

#### Step 10: Validate Detection Plumbing

If the allowlist was already correct, check these files:

**In `src/io/filetype_detection.inc`:**
- [ ] `CHKVGM` function exists
- [ ] `CHKVGM` is in the detection table/loop
- [ ] Extension comparison handles "VGM" (case-insensitive)

**In `src/audio/filetype_config.inc`:**
- [ ] `CONFIG_VGM` function exists
- [ ] `CONFIG_VGM` is in the dispatch table at correct index
- [ ] `TYPVGM` constant matches the table index

**In `src/io/filetypes.inc`:**
- [ ] `TYPVGM` constant is unique (e.g., `TYPVGM .EQU 3`)
- [ ] No collision with `TYPPT2`, `TYPPT3`, `TYPMYM`

**Constants to verify:**
```asm
TYPPT2  .EQU 0
TYPPT3  .EQU 1
TYPMYM  .EQU 2
TYPVGM  .EQU 3  ; Must be present
```

#### Step 11: Compare Debug vs Clean Listings

If the fix is not obvious, generate assembly listings and compare:
```bash
# Clean build
make clean && make vibetune.com
cp vibetune.lst vibetune_clean.lst

# Debug build (if available)
make clean && make DEBUG=1 vibetune.com
cp vibetune.lst vibetune_debug.lst

# Compare
diff -u vibetune_clean.lst vibetune_debug.lst | less
```

**Look for:**
- Conditional assembly (`#ifdef DEBUG`) around VGM code
- Missing CHKVGM or CONFIG_VGM in clean build
- Table size/count mismatches

#### Step 12: Add Minimal Debug Prints (if needed)

If the issue is still unclear, add lightweight breadcrumbs:

**In file validation section:**
```asm
; Debug: Print FCB extension bytes
LD A, (FCB+8)   ; First extension char
CALL PRTHEX
LD A, (FCB+9)   ; Second extension char
CALL PRTHEX
LD A, (FCB+10)  ; Third extension char
CALL PRTHEX
; ... continue validation
```

**After detection:**
```asm
; Debug: Print detected file type
LD A, (FILTYP)
CALL PRTHEX
; ... continue configuration
```

**Remove these after fixing!**

---

### Phase 4: Test and Verify

#### Step 13: Local Emulator Testing
```bash
make release  # Builds, tests, deploys to MAME, increments build number
```

**Expected**: No "filename invalid" errors for VGM files

#### Step 14: MAME Real-World Testing

**Deploy test files:**
```bash
cp test.vgm /Volumes/FATDISK/TEST.VGM
cp Tunes/Attack.pt3 /Volumes/FATDISK/ATTACK.PT3
cp Tunes/Demo.mym /Volumes/FATDISK/DEMO.MYM
```

**In MAME CP/M:**
```
A>VIBETUNE ATTACK.PT3  ← Should work (regression test)
A>VIBETUNE DEMO.MYM    ← Should work (regression test)
A>VIBETUNE TEST.VGM    ← Should work (NEW!)
```

**Capture output** - copy-paste the successful execution for commit notes.

**Success criteria:**
- ✅ No "Sound filename invalid" error for VGM
- ✅ PT2, PT3, MYM still work (backward compatibility)
- ✅ VGM is detected and configured (even if playback is placeholder)
- ✅ No crashes or hangs

---

### Phase 5: Commit After MAME Verification

#### Step 15: Automated Commit (WARP Workflow)
```bash
make commit
```

**This will automatically:**
- Update documentation timestamps
- Update version references across files
- Add changelog entry
- Create git commit with detailed message
- Create version tag (likely `v0.2.6.28`)

**Manually include MAME output** in PROJECT_STATUS.md or commit notes if prompted.

#### Step 16: Verify Commit
```bash
git --no-pager log --oneline -3
git --no-pager show HEAD
```

**Check:**
- Tag created (e.g., `v0.2.6.28`)
- Changelog entry added
- All version numbers consistent

---

## 🔍 DEBUGGING REFERENCE

### Common VGM Detection Issues

**Issue 1: Extension not in early allowlist**
- **Symptom**: "Filename invalid" before detection runs
- **Fix**: Add VGM to the pre-validation check
- **Location**: Early in main program, before `DETECT_FILE_TYPE` call

**Issue 2: CHKVGM not in detection table**
- **Symptom**: Always defaults to PT3 for VGM files
- **Fix**: Add `CHKVGM` to the detection loop/table
- **Location**: `src/io/filetype_detection.inc`

**Issue 3: CONFIG_VGM not in dispatch table**
- **Symptom**: Detection works, but crashes or wrong behavior during config
- **Fix**: Add `CONFIG_VGM` to configuration jump table
- **Location**: `src/audio/filetype_config.inc`

**Issue 4: TYPVGM value collision**
- **Symptom**: VGM triggers PT2/PT3/MYM behavior
- **Fix**: Ensure `TYPVGM` has unique value (e.g., 3)
- **Location**: `src/io/filetypes.inc`

**Issue 5: Conditional assembly excludes VGM**
- **Symptom**: Works in debug, fails in clean builds
- **Fix**: Remove or correct `#ifdef DEBUG` around VGM code
- **Location**: All VGM-related files

### Key Files to Check

| File | Purpose | VGM Requirements |
|------|---------|------------------|
| `vibetune.asm` | Main program | Early validation includes VGM |
| `src/io/filetypes.inc` | Constants | `TYPVGM` defined and unique |
| `src/io/filetype_detection.inc` | Detection | `CHKVGM` exists and is called |
| `src/audio/filetype_config.inc` | Configuration | `CONFIG_VGM` in dispatch table |
| `src/ui/messages.inc` | Messages | VGM mentioned in help/errors |

### Wayne's Dispatch Pattern (Reference)

From the working PT2/PT3/MYM implementation:

**1. Define constant:**
```asm
TYPVGM  .EQU 3
```

**2. Add detector:**
```asm
CHKVGM:
    ; Check if FCB extension is "VGM"
    ; If yes, set FILTYP to TYPVGM and return success
    ; If no, return failure
```

**3. Add configurator:**
```asm
CONFIG_VGM:
    ; Set load address for VGM file
    ; Set playback configuration
    ; Initialize VGM player state
    ; Return
```

**4. Update messages:**
```asm
MSGBAN: .DB "VibeTune v0.2.6.28 for RomWBW"
MSGUSE: .DB "Supports: PT2, PT3, MYM, VGM"
```

---

## 📊 PROJECT CONTEXT

### Architecture Overview
```
vibetune/
├── vibetune.asm           # Main program (~2,599 lines)
├── src/
│   ├── audio/
│   │   └── filetype_config.inc    # Format-specific playback config
│   ├── io/
│   │   ├── filetype_detection.inc # Extension parsing & detection
│   │   └── filetypes.inc          # TYPPT2, TYPPT3, TYPMYM, TYPVGM
│   ├── ui/
│   │   └── messages.inc           # Banner, help, errors
│   ├── hardware/
│   │   └── constants.inc          # Hardware definitions
│   └── [system, cli, utils...]    # Other modules
└── Tunes/                  # Sample music files
```

### Extension Points (For Future Formats)

**To add a new format (e.g., D00, SID, MOD):**
1. Add `TYPxxx` constant to `src/io/filetypes.inc`
2. Add `CHKxxx` function to `src/io/filetype_detection.inc`
3. Add `CONFIG_xxx` function to `src/audio/filetype_config.inc`
4. Update messages in `src/ui/messages.inc`
5. Test and commit per WARP workflow

### Known Risks (From v0.2.5 Failure)

**Do NOT attempt audio engine modularization yet:**
- PTx Player (lines 595-2219, ~1,624 lines) and MYM Player (lines 2219-2513, ~294 lines) are tightly coupled
- Previous attempt caused "BAD INT" errors and crashes
- Function entry points and memory layout are critical
- Requires deep dependency analysis before extracting

---

## 📝 EXPECTED END STATE

### After VGM Fix is Complete

**Build:**
- `make vibetune.com` succeeds
- Binary size: ~5,150-5,250 bytes (±50 bytes acceptable)

**Tests:**
- `make test` passes
- PT2, PT3, MYM work identically (regression check)
- VGM files don't trigger "filename invalid"

**Code:**
- `CHKVGM` exists and is invoked
- `CONFIG_VGM` is in dispatch table
- `TYPVGM` constant is unique and consistent
- Early validation includes VGM in allowlist

**Git:**
- New tag created (e.g., `v0.2.6.28`)
- Changelog updated with VGM fix details
- All documentation version numbers match

**MAME Verification:**
- `VIBETUNE TEST.VGM` works without errors
- Captured output in commit notes or PROJECT_STATUS.md

---

## 🎯 AFTER VGM FIX: NEXT PRIORITIES

### Recommended (Low Risk)
1. **Add more formats** using the proven extension pattern:
   - D00 (Atari SAP format)
   - SID (Commodore 64)
   - MOD (Amiga/PC tracker)
   
2. **System improvements**:
   - Better error messages
   - Configuration file support
   - Enhanced hardware detection

### Medium Risk
- Playlist support
- Looping and repeat modes
- Channel muting
- Real-time controls (volume, tempo)

### Avoid For Now (High Risk)
- **Audio engine modularization** - requires extensive dependency analysis after v0.2.5 failure

---

## 🚨 CRITICAL REMINDERS

1. **DO NOT COMMIT** until MAME verification is complete
2. **DO allow** build number auto-increment during testing
3. **DO use** `make release` for build+test+deploy
4. **DO use** `make commit` after MAME verification
5. **DO respect** CP/M 8.3 filenames (letters preferred)
6. **DO NOT** manually edit VERSION file
7. **DO NOT** force uppercase in filenames unnecessarily
8. **DO NOT** attempt audio engine refactoring during this fix

---

## 📞 ONE-GLANCE RESUME

**When you open this on the new computer:**

1. ✅ Verify environment (Step 1-2)
2. 📖 Read WARP.md + PROJECT_STATUS.md (Step 3)
3. 🔍 Reproduce bug: `make clean && make test` (Step 5)
4. 🔧 Fix: Most likely add VGM to early extension allowlist (Step 9)
5. ✅ Test: `make release` (Step 13)
6. 🎮 Verify in MAME (Step 14)
7. 💾 Commit: `make commit` (Step 15)

**Current task**: Fix "Sound filename invalid" for VGM in clean builds (likely missing from early validation allowlist).

---

**Document created**: 2025-11-15  
**Purpose**: Enable seamless continuation of VibeTune VGM debugging across computers  
**Status**: Ready for use on new machine
