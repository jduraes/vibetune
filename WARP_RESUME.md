# VibeTune VGM Playback Implementation - Current Status

## Date: 2025-11-15

## What We're Doing
Implementing VGM (Video Game Music) file playback in VibeTune. The VGM detection already works (v0.2.6.35), now adding actual playback functionality.

## Reference Implementation
The VGM playback logic exists in `../vgmplay/VGMPLAY.ASM` (TASM format, by J.B. Langston, Marco Maccaferri, Ed Brindley). This is our reference for porting the player into VibeTune.

## Work Completed So Far

### 1. Version Updated
- VERSION file changed to 0.2.6.36

### 2. VGM Variables Added
- Added to `vibetune.asm` at lines 2515-2517:
  - `vgmpos` - Current position in VGM data stream
  - `vgmdly` - Sample delay counter for VGM timing

### 3. VGM Player Module Created
- File: `src/audio/vgm_player.inc` 
- Contains:
  - `VGM_PLAY_FRAME` - Processes VGM command stream
  - `VGM_APPLY_DELAY` - Implements timing loop
  - `VGM_MUTE_ALL` - Silences all sound hardware
- Handles VGM commands:
  - 0x66: Loop/restart
  - 0x50/0x30: SN76489 (primary/secondary)
  - 0xA0: AY-3-8910 (with bit 7 for chip select)
  - 0x5A/0x5E/0x5F: OPL2/OPL3 banks 1 & 2
  - 0x61/0x62/0x63: Wait commands
  - 0x70-0x7F: Short wait (1-16 samples)

## What Still Needs To Be Done

### IMMEDIATE NEXT STEPS:

1. **Add port definitions to vibetune.asm** (BEFORE the #include statements around line 281):
```asm
; VGM-specific hardware ports (fixed addresses for multi-chip/OPL3 support)
RSEL2		.EQU	0A0H	; Secondary AY-3-8910 register select
RDAT2		.EQU	0A1H	; Secondary AY-3-8910 register data
PSGREG		.EQU	0FFH	; Primary SN76489 data port
PSG2REG		.EQU	0FBH	; Secondary SN76489 data port
OPL3ADDR1	.EQU	090H	; OPL3 bank 1 register select
OPL3DATA1	.EQU	091H	; OPL3 bank 1 data
OPL3ADDR2	.EQU	092H	; OPL3 bank 2 register select
OPL3DATA2	.EQU	093H	; OPL3 bank 2 data
```

2. **Add vgm_player.inc to includes** (line 288, after filetype_config.inc):
```asm
#include "src/audio/vgm_player.inc"
```

3. **Replace CONFIG_VGM placeholder** in `src/audio/filetype_config.inc` (lines 137-142):
Replace the current placeholder with actual VGM initialization and playback loop:
```asm
CONFIG_VGM:
	CALL	CRLF2			; Formatting
	LD	DE,MSGPLY		; Playing message
	CALL	PRTSTR			; Print message
	
	; Initialize VGM playback
	LD	HL,(vgmdata + 34H)	; Get VGM data start offset from header
	LD	A,H
	OR	L
	JR	NZ,VGM_START1
	LD	HL,000CH		; Default location if offset is 0
VGM_START1:
	LD	DE,vgmdata + 34H
	ADD	HL,DE			; Calculate absolute start position
	LD	(vgmpos),HL		; Save start position
	LD	HL,735			; Default delay (60Hz frame)
	LD	(vgmdly),HL		; Initialize delay

VGM_MAINLOOP:
	CALL	VGM_PLAY_FRAME		; Process one frame of VGM commands
	
	; Check for keypress
	CALL	GETKEY			; Check for key
	JR	NZ,VGM_EXIT		; Exit if key pressed
	
	; Apply frame delay
	CALL	VGM_APPLY_DELAY		; Wait for frame timing
	
	JR	VGM_MAINLOOP		; Loop

VGM_EXIT:
	CALL	VGM_MUTE_ALL		; Silence all hardware
	RET				; Return to main
```

4. **Build and test** on RC2014 hardware

5. **Commit** with message about VGM playback implementation

## Hardware Details
- RC2014 with Ed Brindley's YM2149 rev 6.1 (MSX ports: 0xD0/0xD8)
- RCBUS OPL3 card at port 0x90h
- Testing via file transfer to real hardware

## Key Files
- `vibetune.asm` - Main file (needs port defs + include)
- `src/audio/vgm_player.inc` - VGM player module (DONE)
- `src/audio/filetype_config.inc` - CONFIG_VGM needs implementation
- `VERSION` - Already at 0.2.6.36
- `../vgmplay/VGMPLAY.ASM` - Reference implementation

## Notes
- VGM detection already working (CHKVGM, TYPVGM=4)
- Load address already set to vgmdata
- EXIT already skips START+8 for VGM files (lines 268-269)
- Frame delay constant FRAME_DLY = 15 (for 7.3728 MHz RC2014)
- VGM header at offset 0x34 contains data start offset
- Loop offset at header 0x1C for continuous playback
