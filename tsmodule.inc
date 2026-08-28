;======================================================================
; tsmodule.inc — single-card dual-AVR TurboSound module support
;
; One port pair (MSX $A0/$A1). Chips selected by latching 0xFF (chip 0) or
; 0xFE (chip 1) to RSEL. Selection is sticky; normal regs 0-15 never clear it.
; Select OUT shares the Bulba ROUT slow-I/O window (TSMOD_SLOWIO).
;======================================================================

;----------------------------------------------------------------------
; TSMOD_SLOWIO — SLOWIO hook: DCNTL slow enter, then optional module select.
; TSMOD_CURSEL: 0 = dual-card/single; $FF/$FE = module chip for this ROUT.
; Clobbers AF; BC preserved.
;----------------------------------------------------------------------
TSMOD_SLOWIO:
	CALL	PSG_IO_SLOW_ENTER
	LD	A, (TS_TOPOLOGY)
	CP	TS_TOPO_MODULE
	RET	NZ
	LD	A, (TSMOD_CURSEL)
	OR	A
	RET	Z
	PUSH	BC
	LD	B, A			; B = select code
	LD	A, (BULBA_PORTS)	; RSEL
	LD	C, A
	LD	A, B
	LD	B, 0			; OUT (C),A uses full BC as port
	OUT	(C), A
	POP	BC
	RET

;----------------------------------------------------------------------
; TSMOD_SELECT — select module chip (mute / non-ROUT paths).
; Input: A = $FF (chip 0) or $FE (chip 1). Sets TSMOD_CURSEL.
;----------------------------------------------------------------------
TSMOD_SELECT:
	LD	(TSMOD_CURSEL), A
	PUSH	BC
	LD	C, A
	CALL	PSG_IO_SLOW_ENTER
	LD	A, (PSG_REG_PORT)
	LD	B, C
	LD	C, A
	LD	A, B
	LD	B, 0
	OUT	(C), A
	CALL	PSG_IO_SLOW_EXIT
	POP	BC
	RET

;----------------------------------------------------------------------
; TSMOD_MUTE — mute both module chips on the single port pair.
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
	LD	A, $FF			; leave chip 0 selected
	CALL	TSMOD_SELECT
	JP	RESTORE_PLAY_PSG_PORTS

;----------------------------------------------------------------------
; TS_END_DUAL_SESSION — before single-chip (non-TS) playback after a
; TurboSound track. Re-select module chip 0 (sticky FF/FE latch).
; Keep MODULE topology for the session — this hardware needs FF/FE on
; every ROUT; reverting to AUTO leaves chip 2 latched and adds noise.
;----------------------------------------------------------------------
TS_END_DUAL_SESSION:
	CALL	RESTORE_PLAY_PSG_PORTS
	LD	A, (TS_TOPOLOGY)
	CP	TS_TOPO_MODULE
	RET	NZ
	LD	A, $FF
	JP	TSMOD_SELECT
