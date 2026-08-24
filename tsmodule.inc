;======================================================================
; tsmodule.inc — single-card dual-AVR TurboSound module support (SKELETON)
;
; Branch turbosound-module. NOT YET WIRED INTO vibetune.asm — see
; docs/TurboSound-Module.md for the design. Nothing in this file is
; referenced by the build until the work items there are done.
;
; The module presents ONE port pair (MSX standard $A0 RSEL / $A1 RDAT).
; The two emulated chips are selected by latching a TurboSound code to
; RSEL as if it were a register number:
;   0xFF -> chip 0 (also the default after reset)
;   0xFE -> chip 1
; Selection is sticky until the next select code. All normal latch/write
; cycles after a select go to the selected chip.
;
; Timing (from the AVR-side RCA, 2026-08-24): strobes must be >= ~380ns
; (RomWBW Z180_IOWAIT=3 covers this) and each OUT must go through the
; slow-I/O path so the AVR's ~1us ISR can keep up. TSMOD_SELECT therefore
; wraps its OUT exactly like PSG_WRITE_REG does.
;======================================================================

#DEFINE TS_TOPO_DUALCARD 0	; two real chips, two port pairs (current behavior)
#DEFINE TS_TOPO_MODULE   1	; one port pair, AVR module, 0xFF/0xFE select

; TS_TOPOLOGY: runtime topology flag (set from VTUNE.CFG byte 5 / CLI switch).
;TS_TOPOLOGY:	.DB	TS_TOPO_DUALCARD

;----------------------------------------------------------------------
; TSMOD_SELECT — select module chip.
; Input:  A = 0xFF (chip 0) or 0xFE (chip 1)
; Clobbers: AF, BC (matches PSG_WRITE_REG conventions: BC restored)
;----------------------------------------------------------------------
TSMOD_SELECT:
	PUSH	BC
	CALL	PSG_IO_SLOW_ENTER
	LD	C, A			; save select code
	LD	A, (PSG_REG_PORT)	; RSEL port
	LD	B, 0
	LD	D, C			; TODO(desk-check): free reg for code; D used?
	LD	C, A
	LD	A, D
	OUT	(C), A			; latch 0xFF/0xFE (no RDAT write follows)
	CALL	PSG_IO_SLOW_EXIT
	POP	BC
	RET

;----------------------------------------------------------------------
; TSMOD_SETPORTS1 / TSMOD_SETPORTS2 — module topology replacements for
; TS_SETPORTS1/2: one shared port pair, differing only in select code.
; Called from TS_PLAYQUARK when TS_TOPOLOGY == TS_TOPO_MODULE.
;----------------------------------------------------------------------
TSMOD_SETPORTS1:
	LD	A, (TS_PORT1_RSEL)	; module: PORT1 pair == the single pair
	LD	(PSG_REG_PORT), A
	LD	A, (TS_PORT1_RDAT)
	LD	(PSG_DATA_PORT), A
	LD	A, $FF
	JR	TSMOD_SELECT

TSMOD_SETPORTS2:
	LD	A, (TS_PORT1_RSEL)	; same pair as chip 0 — NOT TS_PORT2
	LD	(PSG_REG_PORT), A
	LD	A, (TS_PORT1_RDAT)
	LD	(PSG_DATA_PORT), A
	LD	A, $FE
	JR	TSMOD_SELECT

;----------------------------------------------------------------------
; TSMOD_MUTE — mute both chips on one port pair (module topology
; replacement for TS_MUTE_BOTH_HARDWARE).
;----------------------------------------------------------------------
TSMOD_MUTE:
	LD	A, $FF
	CALL	TSMOD_SELECT
	CALL	PSG_MUTE_ROUT
	LD	A, $FE
	CALL	TSMOD_SELECT
	JP	PSG_MUTE_ROUT
