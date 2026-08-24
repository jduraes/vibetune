;======================================================================
; tsmodule.inc — single-card dual-AVR TurboSound module support
;
; (TS_TOPO_* defines live near the top of vibetune.asm - TASM #DEFINEs
; are order-sensitive and the switch table needs them early.)
;
; The module (two ATmega8A emulators on one DIP-40 footprint) presents ONE
; port pair (MSX standard $A0 RSEL / $A1 RDAT). Chips are selected by
; latching a TurboSound code to RSEL as if it were a register number:
;   0xFF -> chip 0 (also the default after reset)
;   0xFE -> chip 1
; Selection is sticky until the next select code. Register numbers are
; 0-15 (D7=0), so normal latches never disturb the selection.
;
; Timing (AVR-side RCA, 2026-08-24): strobes need >= ~380ns (RomWBW
; Z180_IOWAIT=3 covers it) and the AVR services each access in a ~1us ISR,
; so the select OUT must go through the slow-I/O path like PSG_WRITE_REG.
;======================================================================

;----------------------------------------------------------------------
; TSMOD_SELECT — select module chip.
; Input:  A = $FF (chip 0) or $FE (chip 1)
; Uses PSG_REG_PORT as the RSEL port (caller sets it first).
; Clobbers: AF; BC preserved (PSG_IO_SLOW_* preserve BC, we push it).
;----------------------------------------------------------------------
TSMOD_SELECT:
	PUSH	BC
	LD	C, A			; select code
	CALL	PSG_IO_SLOW_ENTER
	LD	B, C			; code
	LD	A, (PSG_REG_PORT)
	LD	C, A			; RSEL port
	LD	A, B
	OUT	(C), A			; latch 0xFF/0xFE (no RDAT write follows)
	CALL	PSG_IO_SLOW_EXIT
	POP	BC
	RET

;----------------------------------------------------------------------
; TSMOD_MUTE — mute both chips on one port pair (module-topology
; replacement for TS_MUTE_BOTH_HARDWARE).
;----------------------------------------------------------------------
TSMOD_MUTE:
	LD	A, (TS_PORT1_RSEL)
	LD	(PSG_REG_PORT), A
	LD	A, (TS_PORT1_RDAT)
	LD	(PSG_DATA_PORT), A
	LD	A, $FF
	CALL	TSMOD_SELECT
	CALL	PSG_MUTE_ROUT
	LD	A, $FE
	CALL	TSMOD_SELECT
	CALL	PSG_MUTE_ROUT
	LD	A, $FF			; leave chip 0 selected (power-on default state)
	JP	TSMOD_SELECT
