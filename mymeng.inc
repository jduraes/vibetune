;===============================================================================
; MYMENG.INC - MYMPlay 0.4 depack/play engine (optional; assemble with -dMYM)
;===============================================================================
; MYMPlay 0.4 by Marq/Lieves!Tuore & Fit. Ported from VibeTune-old.
; Included from vibetune.asm only when MYM is defined (tasm -dMYM).
;===============================================================================

ENGINE_INIT_MYM:
	CALL	TS_END_DUAL_SESSION
	CALL	MYM_INIT
	CALL	MYM_EXTRACT		; prime depack window A
	LD	(MYM_NEXT_SRC), IY
	CALL	MYM_EXTRACT		; prime depack window B
	LD	(MYM_NEXT_SRC), IY
	XOR	A
	RET

;-------------------------------------------------------------------------------
; MYM depacker (MYMPlay 0.4 core by Marq/Lieves!Tuore & Fit, ported from VibeTune-old).
; Unpacks one 128-frame fragment per MYM_EXTRACT call into a rotating pair of
; register windows overlaid at MUSIC_BUF; the packed file image sits at
; MYM_ROWS (2-byte row count, then packed data at MYM_DATA).
; Bit-reader state lives in the EXX bank between calls (HL' = packed pointer,
; E'/D' = bit shifter); nothing else in this build touches the EXX bank.
; MYM_EXTRACT returns IY = play position of the fragment just depacked, and
; CF=1 while more frames remain (CF=0 at end of tune).
;-------------------------------------------------------------------------------
MYM_FRAG	.EQU	128		; fragment size (frames)
MYM_REGS	.EQU	14		; PSG registers per frame
MYM_FBITS	.EQU	7		; bits per back-reference offset/length

; Bits per PSG register
MYM_REGBITS:
	.DB	8,4,8,4,8,4,5,8,5,5,5,8,8,8

MYM_INIT:
	EXX				; Starting values for MYM_READBITS
	LD	E, 1
	LD	D, 0
	LD	HL, MYM_DATA
	EXX
	LD	HL, MYM_UNCOMP + MYM_FRAG	; Starting play/depack pointers
	LD	(MYM_DEST1), HL
	LD	(MYM_DEST2), HL
	LD	(MYM_PSOURCE), HL
	LD	A, MYM_FRAG
	LD	(MYM_PLAYED), A
	LD	HL, 0
	LD	(MYM_PROWS), HL
	RET

MYM_EXTRACT:
	LD	A, 0
MYM_REGLOOP:
	PUSH	AF
	LD	C, A
	LD	B, 0
	LD	HL, MYM_REGBITS
	ADD	HL, BC
	LD	D, (HL)		; D = bits in this PSG register
	LD	HL, MYM_CURRENT
	ADD	HL, BC
	LD	E, (HL)		; E = current value of register
	LD	BC, MYM_FRAG*3
	LD	HL, (MYM_DEST1)
	LD	IX, (MYM_DEST1)	; IX = destination 1
	ADD	HL, BC
	LD	(MYM_DEST1), HL
	LD	HL, (MYM_DEST2)	; HL = destination 2
	PUSH	HL
	ADD	HL, BC
	LD	(MYM_DEST2), HL
	POP	HL
	EX	AF, AF'		; AF' = fragment end counter
	LD	A, MYM_FRAG
	EX	AF, AF'
	LD	A, 1			; Get fragment bit
	CALL	MYM_READBITS
	OR	A
	JR	NZ, MYM_COMPFRAG	; 1=compressed fragment, 0=unchanged

	LD	B, MYM_FRAG		; Unchanged fragment: just set all to E
MYM_SWEEP:
	LD	(HL), E
	INC	HL
	LD	(IX+0), E
	INC	IX
	DJNZ	MYM_SWEEP
	JP	MYM_NEXTREG

MYM_COMPFRAG:				; Compressed fragment
	LD	A, 1
	CALL	MYM_READBITS
	OR	A
	JR	NZ, MYM_NOTPREV		; 0=previous register value, 1=raw/compressed

	LD	(HL), E			; Unchanged register
	INC	HL
	LD	(IX+0), E
	INC	IX
	EX	AF, AF'
	DEC	A
	EX	AF, AF'
	JP	MYM_NEXTBIT

MYM_NOTPREV:
	LD	A, 1
	CALL	MYM_READBITS
	OR	A
	JR	Z, MYM_PACKED		; 0=compressed data, 1=raw data

	LD	A, D			; Raw data, read regbits[i] bits
	CALL	MYM_READBITS
	LD	E, A
	LD	(HL), A
	INC	HL
	LD	(IX+0), A
	INC	IX
	EX	AF, AF'
	DEC	A
	EX	AF, AF'
	JP	MYM_NEXTBIT

MYM_PACKED:
	LD	A, MYM_FBITS		; Reference to previous data: read the offset
	CALL	MYM_READBITS
	LD	C, A
	LD	A, MYM_FBITS		; Read the number of bytes
	CALL	MYM_READBITS
	LD	B, A
	PUSH	HL
	PUSH	BC
	LD	BC, -MYM_FRAG
	ADD	HL, BC
	POP	BC
	LD	A, B
	LD	B, 0
	ADD	HL, BC
	LD	B, A
	PUSH	HL
	POP	IY			; IY = source address
	POP	HL
	INC	B
MYM_COPY:
	LD	A, (IY+0)		; Copy from previous data
	INC	IY
	LD	E, A			; Set current value
	LD	(HL), A
	INC	HL
	LD	(IX+0), A
	INC	IX
	EX	AF, AF'
	DEC	A
	EX	AF, AF'
	DJNZ	MYM_COPY

MYM_NEXTBIT:
	EX	AF, AF'		; If AF'=0 then fragment is done
	LD	C, A
	EX	AF, AF'
	LD	A, C
	OR	A
	JP	NZ, MYM_COMPFRAG

MYM_NEXTREG:
	POP	AF
	LD	B, 0			; Save the current value of PSG reg
	LD	C, A
	PUSH	HL
	LD	HL, MYM_CURRENT
	ADD	HL, BC
	LD	(HL), E
	POP	HL
	INC	A			; Check if all registers are done
	CP	MYM_REGS
	JP	NZ, MYM_REGLOOP

	OR	A			; Check if dest2 must be wrapped
	LD	BC, MYM_ROWS
	SBC	HL, BC
	JR	NZ, MYM_NOWRAP

	LD	IX, MYM_UNCOMP + MYM_FRAG
	LD	HL, MYM_UNCOMP + MYM_FRAG
	LD	IY, MYM_UNCOMP + (2*MYM_FRAG)
	JR	MYM_ENDEXT

MYM_NOWRAP:
	LD	IX, MYM_UNCOMP
	LD	HL, MYM_UNCOMP + (2*MYM_FRAG)
	LD	IY, MYM_UNCOMP + MYM_FRAG

MYM_ENDEXT:
	LD	(MYM_DEST1), IX
	LD	(MYM_DEST2), HL
	LD	BC, MYM_FRAG		; Rows played so far += FRAG
	LD	HL, (MYM_PROWS)
	ADD	HL, BC
	LD	(MYM_PROWS), HL
	LD	BC, (MYM_ROWS)		; CF=1 while rows > played rows
	OR	A
	SBC	HL, BC
	RET				; CF=0 at end of tune

; Reads A bits from packed stream (EXX bank), returns bits in A
MYM_READBITS:
	EXX
	LD	B, A
	LD	C, 0
MYM_ONEBIT:
	SLA	C			; Get one bit at a time
	RRC	E
	JR	NC, MYM_NONEW		; Wrap the AND value
	LD	D, (HL)
	INC	HL
MYM_NONEW:
	LD	A, E
	AND	D
	JR	Z, MYM_ZERO
	INC	C
MYM_ZERO:
	DJNZ	MYM_ONEBIT
	LD	A, C
	EXX
	RET

;-------------------------------------------------------------------------------
; MYM engine tick. Plays one frame (14 PSG registers) from the depacked
; windows each call. When a fragment (MYM_FRAG frames) is consumed, swaps to
; the pre-depacked window and depacks the next one. Stops (silences) at end
; of tune (MYM_EXTRACT returns CF=0).
;-------------------------------------------------------------------------------
MYM_ENGINE_TICK:
	LD	A, (MYM_PLAYED)
	OR	A
	JR	NZ, MYM_TICK_FRAME
	; Fragment consumed: swap to pre-depacked window, depack the next.
	LD	HL, (MYM_NEXT_SRC)
	LD	(MYM_PSOURCE), HL
	LD	A, MYM_FRAG
	LD	(MYM_PLAYED), A
	CALL	MYM_EXTRACT
	JR	NC, MYM_TICK_STOP
	LD	(MYM_NEXT_SRC), IY
MYM_TICK_FRAME:
	; Gather 14 register bytes (stride 3*FRAG) into MYM_FRAME scratch.
	LD	HL, (MYM_PSOURCE)
	LD	DE, MYM_FRAME
	LD	B, MYM_REGS
MYM_TICK_GATHER:
	LD	A, (HL)
	LD	(DE), A
	INC	DE
	LD	BC, 3*MYM_FRAG
	ADD	HL, BC
	DJNZ	MYM_TICK_GATHER
	LD	HL, MYM_FRAME
	CALL	PSG_ROUT_BLOCK
	LD	HL, (MYM_PSOURCE)
	INC	HL
	LD	(MYM_PSOURCE), HL
	LD	A, (MYM_PLAYED)
	DEC	A
	LD	(MYM_PLAYED), A
	JP	PLAYBACK_TICK_DONE

MYM_TICK_STOP:
	CALL	PSG_SILENCE
	LD	A, PLAY_STOPPED
	LD	(PLAY_STATE), A
	JP	PLAYBACK_TICK_DONE
