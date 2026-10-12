;===============================================================================
; SAA1099 Direct I/O Backend (-saa)
;===============================================================================
;
; Translates a 14-byte AY-3-8910 shadow register block (AYREGS layout) into
; SAA1099 register writes, once per frame.  Used by the Bulba ROUT (PT2/PT3)
; and by PSG_ROUT_BLOCK (MYM frame pump and all mute paths).
;
; Selected at runtime with the -saa command line switch.  Uses direct I/O
; (the chip is write-only), deliberately NOT the HBIOS SND API, for better
; timing fidelity.
;
; Hardware (Etched Pixels-style RC2014 SAA1099 card, 8 MHz clock, verified
; on real hardware):
;   Port $FF = register select (write register number)
;   Port $FE = data
;
; A TurboSound PT3 plays both chips on the one SAA: callers pass the SAA
; base channel in A (0 = AY chip 1 -> SAA ch 0-2, 3 = chip 2 -> ch 3-5).
;
; Translation notes & approximations:
;
; - Tone: AY period P (12-bit) at the assumed AY clock of 1.7897725 MHz gives
;   f = AYCLK / (16*P).  The SAA needs divider D = 8e6 / f = 71.52 * P, with
;   D = (511 - m) * 2^(9 - octave).  We compute D ~= 71.5 * P as
;   (P<<6)+(P<<3)-(P>>1) (< 0.03% pitch error), then pick the highest octave
;   whose Q = D / 2^(9-oct) fits in 9 bits, and m = 511 - Q.  Q is clamped to
;   [256, 511] so m always fits in 8 bits; periods beyond the SAA range
;   (~31 Hz - 7.8 kHz) clamp to the nearest extreme instead of wrapping.
;   A period of 0 (silent channel) clears the SAA frequency-enable bit.
;
; - Volume: AY amplitude nibble v -> SAA amplitude v | (v<<4) (same level on
;   left and right = centered).  The SAA amplitude resolution matches the
;   AY's 16 steps but is LINEAR where the AY is logarithmic, so quiet AY
;   passages sound relatively louder; no log correction is applied.
;
; - Mixer (AY reg 7, active-low): tone-disable bits 0-2 -> inverted into SAA
;   $14 frequency-enable; noise-disable bits 3-5 -> inverted into SAA $15
;   noise-enable.
;
; - Noise: AY noise period (reg 6, 5-bit) sets SAA noise gen rate in reg $16.
;   SAA rates are f = 8e6 / 2^(8+rate): rate 0 = 31.25 kHz, 1 = 15.6 kHz,
;   2 = 7.8 kHz (rate 3 = tone-derived, unused).  AY noise runs
;   111.9 kHz / period, so period 0-5 -> rate 0, 6-10 -> rate 1, 11+ -> rate 2.
;   Coarse approximation: the SAA noise floor (~7.8 kHz) is much higher than
;   the AY's (~3.6 kHz at period 31).  All three channels of a block share
;   one SAA noise generator (gen 0 for ch 0-2, gen 1 for ch 3-5), matching
;   the AY's single shared noise source.
;
; - Envelope: an AY channel with bit 4 of its amplitude register set is
;   routed through the block's SAA envelope generator (reg $18 for ch 0-2,
;   $19 for ch 3-5) at maximum amplitude.  AY shapes map to the nearest SAA
;   shape (see SAASHAPES): AY hold states (11, 13) have no SAA equivalent and
;   fall back to single decay/attack; AY 0-7 collapse onto decay/attack.
;   LIMITATIONS: the SAA envelope period is not programmable from the CPU in
;   this mapping - bit 5 = 0 selects the internal envelope clock, so the
;   envelope rate follows tone channel 1 (or 4) frequency; AY envelope
;   period (regs 11-12) is ignored.  Also, an enabled SAA envelope generator
;   modulates ALL THREE channels of its group, so fixed-volume channels on
;   the same group as an enveloped channel are affected (hardware limit).
;
;===============================================================================

SAA_RSEL	.EQU	$FF	; SAA1099 register select port
SAA_RDAT	.EQU	$FE	; SAA1099 data port

;===============================================================================
; SAA_INIT - Reset and enable the SAA1099
;===============================================================================
; Zeroes all 32 registers, pulses reset ($1C=$02), enables sound ($1C=$01),
; and clears the driver's shadow state.
;
; Uses: AF, BC, HL
;===============================================================================
SAA_INIT:
	LD	B,32		; registers $00-$1F
	XOR	A		; start at register 0
SAA_INIT1:
	OUT	(SAA_RSEL),A	; select register
	PUSH	AF
	XOR	A
	OUT	(SAA_RDAT),A	; write 0
	POP	AF
	INC	A
	DJNZ	SAA_INIT1
	LD	A,$1C
	OUT	(SAA_RSEL),A
	LD	A,$02		; reset pulse
	OUT	(SAA_RDAT),A
	LD	A,$01		; all channels sound enable
	OUT	(SAA_RDAT),A
	LD	HL,SAASHD	; clear shadow state
	LD	B,SAASHDL
	XOR	A
SAA_INIT2:
	LD	(HL),A
	INC	HL
	DJNZ	SAA_INIT2
	RET

;===============================================================================
; SAA_WREG - Write one SAA1099 register
;===============================================================================
; A = register number, E = value
; Uses: AF
;===============================================================================
SAA_WREG:
	OUT	(SAA_RSEL),A
	LD	A,E
	OUT	(SAA_RDAT),A
	RET

;===============================================================================
; SAA_PER2MF - Convert AY tone period to SAA frequency/octave
;===============================================================================
; HL = AY period (0..4095)
; Returns: A = SAA frequency byte m, B = octave (0..7)
; P = 0 returns m=0, octave=0 (caller clears the frequency-enable bit)
; Uses: AF, BC, DE, HL, SAAD
;===============================================================================
SAA_PER2MF:
	LD	A,H
	OR	L
	JR	NZ,SAA_PMF1
	XOR	A		; P = 0 -> m = 0, octave = 0
	LD	B,A
	RET
SAA_PMF1:
	; SAAD (24-bit) = 71.5 * P = (P<<6) + (P<<3) - (P>>1)
	PUSH	HL		; save P
	XOR	A
	LD	B,6
SAA_PMF2:			; A:HL = P << 6
	ADD	HL,HL
	ADC	A,A
	DJNZ	SAA_PMF2
	LD	(SAAD),HL
	LD	(SAAD+2),A
	POP	HL
	PUSH	HL
	XOR	A
	LD	B,3
SAA_PMF3:			; A:HL = P << 3
	ADD	HL,HL
	ADC	A,A
	DJNZ	SAA_PMF3
	LD	BC,(SAAD)	; D += P<<3
	ADD	HL,BC
	LD	(SAAD),HL
	LD	B,A
	LD	A,(SAAD+2)
	ADC	A,B
	LD	(SAAD+2),A
	POP	HL		; HL = P
	SRL	H		; HL = P >> 1
	RR	L
	EX	DE,HL		; DE = P>>1
	LD	HL,(SAAD)
	AND	A		; clear carry
	SBC	HL,DE		; D -= P>>1
	LD	(SAAD),HL
	LD	A,(SAAD+2)
	SBC	A,0
	LD	(SAAD+2),A
	; Octave search: start at octave 7 (divisor 2^(9-7) = 4) and step
	; down; per step the divisor doubles until Q = D/divisor fits 9 bits.
	LD	B,7		; octave
	LD	A,(SAAD+2)
	LD	HL,(SAAD)
	SRL	A		; D /= 4
	RR	H
	RR	L
	SRL	A
	RR	H
	RR	L
	LD	(SAAD),HL
	LD	(SAAD+2),A
SAA_PMF4:
	LD	A,(SAAD+2)
	OR	A
	JR	NZ,SAA_PMF5	; Q > 511, try next lower octave
	LD	HL,(SAAD)
	LD	A,H
	CP	2
	JR	C,SAA_PMF6	; Q fits in 9 bits (H <= 1)
SAA_PMF5:
	LD	A,B
	OR	A
	JR	Z,SAA_PMF8	; octave 0 and still too big: clamp m = 0
	LD	A,(SAAD+2)	; D /= 2 (next octave down)
	LD	HL,(SAAD)
	SRL	A
	RR	H
	RR	L
	LD	(SAAD),HL
	LD	(SAAD+2),A
	DEC	B
	JR	SAA_PMF4
SAA_PMF8:
	LD	HL,511		; Q = 511 -> m = 0 (lowest SAA frequency)
	JR	SAA_PMF7
SAA_PMF6:
	LD	A,H		; clamp Q to >= 256 so m fits in 8 bits
	OR	A
	JR	NZ,SAA_PMF7
	LD	HL,256		; Q = 256 -> m = 255 (highest at this octave)
SAA_PMF7:
	EX	DE,HL
	LD	HL,511
	AND	A
	SBC	HL,DE		; m = 511 - Q
	LD	A,L
	RET			; A = m, B = octave

;===============================================================================
; SAA_SETOCT - Set one channel's octave nibble in the $10-$12 shadow
;===============================================================================
; A = octave (0..7), C = SAA channel (0..5)
; Uses: AF, B, DE, HL
;===============================================================================
SAA_SETOCT:
	LD	B,A		; octave
	LD	A,C
	SRL	A		; A = shadow index (ch/2), carry = odd channel
	JR	C,SAA_SO_HI	; test carry FIRST: ADD HL,DE below clears it
	LD	HL,SAAS10
	LD	E,A
	LD	D,0
	ADD	HL,DE
	LD	A,(HL)		; even channel: low nibble
	AND	$F0
	OR	B
	LD	(HL),A
	RET
SAA_SO_HI:
	LD	HL,SAAS10
	LD	E,A
	LD	D,0
	ADD	HL,DE
	LD	A,B		; odd channel: high nibble
	ADD	A,A
	ADD	A,A
	ADD	A,A
	ADD	A,A
	LD	B,A
	LD	A,(HL)
	AND	$0F
	OR	B
	LD	(HL),A
	RET

;===============================================================================
; SAA_XLATE - Translate one 14-byte AY register image to the SAA1099
;===============================================================================
; HL = pointer to AY image (AYREGS layout: 6 tone bytes, noise, mixer,
;      3 amplitude bytes, 2 envelope period bytes, envelope shape)
; A  = SAA base channel: 0 (ch 0-2, env gen 0) or 3 (ch 3-5, env gen 1)
; Uses: AF, BC, DE, HL, IX
;===============================================================================
SAA_XLATE:
	LD	(SAABASE),A
	PUSH	HL
	POP	IX		; IX = AY image
	XOR	A
	LD	(SAAENVB),A	; no envelope user in this block yet
	; --- mixer: build freq-enable (D) and noise-enable (E) bitmasks ---
	LD	A,(IX+7)
	CPL			; AY mixer is active-low
	LD	H,A
	AND	%00000111	; tone enable, bits 0-2
	LD	D,A
	LD	A,H
	RRCA			; noise enable -> bits 0-2
	RRCA
	RRCA
	AND	%00000111
	LD	E,A
	LD	A,(SAABASE)	; second block? shift masks to bits 3-5
	OR	A
	JR	Z,SAA_XM1
	LD	A,D
	ADD	A,A
	ADD	A,A
	ADD	A,A
	LD	D,A
	LD	A,E
	ADD	A,A
	ADD	A,A
	ADD	A,A
	LD	E,A
	LD	A,%00000111	; other block owns bits 0-2
	JR	SAA_XM2
SAA_XM1:
	LD	A,%00111000	; other block owns bits 3-5
SAA_XM2:
	LD	B,A		; B = bits belonging to the other block
	LD	A,(SAAS14)
	AND	B
	OR	D
	LD	(SAAS14),A
	LD	A,(SAAS15)
	AND	B
	OR	E
	LD	(SAAS15),A
	; --- per-channel amplitude / frequency / octave ---
	LD	B,3		; 3 channels per block
	LD	C,0		; AY channel index 0..2
SAA_XCH:
	LD	A,(SAABASE)
	ADD	A,C
	LD	(SAACH),A	; SAACH = SAA channel number
	; amplitude
	LD	A,8
	ADD	A,C
	LD	E,A
	LD	D,0
	PUSH	IX
	POP	HL
	ADD	HL,DE
	LD	A,(HL)		; AY amplitude register
	LD	E,A
	AND	$10		; envelope flag?
	JR	Z,SAA_XAMP1
	LD	A,1
	LD	(SAAENVB),A	; this block uses the envelope generator
	LD	E,$0F		; envelope-driven channel: max amplitude
SAA_XAMP1:
	LD	A,E
	AND	$0F		; volume nibble
	LD	E,A
	ADD	A,A		; duplicate into both nibbles (centered)
	ADD	A,A
	ADD	A,A
	ADD	A,A
	OR	E
	LD	E,A
	LD	A,(SAACH)	; amplitude register = channel number
	CALL	SAA_WREG
	; tone period
	LD	A,C
	ADD	A,A		; offset 2*i into tone periods
	LD	E,A
	LD	D,0
	PUSH	IX
	POP	HL
	ADD	HL,DE
	LD	A,(HL)
	INC	HL
	LD	H,(HL)
	LD	L,A
	LD	A,H
	AND	$0F		; 12-bit period
	LD	H,A
	PUSH	HL		; save period for zero check
	PUSH	BC
	CALL	SAA_PER2MF	; A = m, B = octave
	LD	E,A		; m
	LD	A,B
	LD	(SAATMP),A	; save octave
	POP	BC
	POP	HL		; HL = period
	; period of 0 silences the channel
	PUSH	BC
	LD	A,H
	OR	L
	JR	NZ,SAA_XW1
	LD	A,(SAACH)	; clear this channel's freq-enable bit
	LD	B,A
	LD	A,1
	INC	B
SAA_XT0:
	DEC	B
	JR	Z,SAA_XT1
	ADD	A,A
	JR	SAA_XT0
SAA_XT1:
	CPL
	LD	HL,SAAS14
	AND	(HL)
	LD	(HL),A
SAA_XW1:
	LD	A,(SAACH)
	ADD	A,8		; frequency register = channel + 8
	CALL	SAA_WREG	; write m
	LD	A,(SAACH)
	LD	C,A		; C = SAA channel
	LD	A,(SAATMP)	; octave
	CALL	SAA_SETOCT
	POP	BC
	INC	C
	DJNZ	SAA_XCH
	; --- write packed octave / enable / noise registers from shadows ---
	LD	A,(SAAS10)
	LD	E,A
	LD	A,$10
	CALL	SAA_WREG
	LD	A,(SAAS11)
	LD	E,A
	LD	A,$11
	CALL	SAA_WREG
	LD	A,(SAAS12)
	LD	E,A
	LD	A,$12
	CALL	SAA_WREG
	LD	A,(SAAS14)
	LD	E,A
	LD	A,$14
	CALL	SAA_WREG
	LD	A,(SAAS15)
	LD	E,A
	LD	A,$15
	CALL	SAA_WREG
	; --- noise rate from AY noise period (reg 6, 5-bit) ---
	LD	A,(IX+6)
	AND	$1F
	CP	6		; 0-5 -> rate 0 (31.25 kHz)
	JR	C,SAA_XN1
	CP	11		; 6-10 -> rate 1 (15.6 kHz)
	JR	C,SAA_XN2
	LD	A,2		; 11+ -> rate 2 (7.8 kHz)
	JR	SAA_XN3
SAA_XN2:
	LD	A,1
	JR	SAA_XN3
SAA_XN1:
	XOR	A
SAA_XN3:
	LD	B,A		; rate 0..2
	LD	A,(SAABASE)
	OR	A
	LD	A,B
	JR	Z,SAA_XN4
	ADD	A,A		; block 1 drives noise gen 1: rate << 4
	ADD	A,A
	ADD	A,A
	ADD	A,A
	LD	B,A
	LD	A,(SAAS16)
	AND	$0F		; keep gen 0 bits
	OR	B
	JR	SAA_XN5
SAA_XN4:
	LD	B,A
	LD	A,(SAAS16)
	AND	$F0		; keep gen 1 bits
	OR	B
SAA_XN5:
	LD	(SAAS16),A
	LD	E,A
	LD	A,$16
	CALL	SAA_WREG
	; --- envelope generator ---
	LD	A,(SAAENVB)
	OR	A
	JR	Z,SAA_XE_OFF
	LD	A,(IX+13)	; AY envelope shape register
	OR	A
	RET	M		; bit 7 set ($FF) = no new shape this frame
	AND	$0F
	LD	HL,SAASHAPES
	LD	E,A
	LD	D,0
	ADD	HL,DE
	LD	E,(HL)		; SAA env reg value: enable + mapped shape
	LD	HL,SAAENV	; envelope-in-use flag for this block
	LD	A,(SAABASE)
	OR	A
	LD	A,$18
	JR	Z,SAA_XE1
	INC	A		; block 1 -> register $19
	INC	HL		; ... and SAAENV+1
SAA_XE1:
	LD	(HL),1		; mark generator in use
	CALL	SAA_WREG
	RET
SAA_XE_OFF:
	; no envelope user this frame: disable the generator if it was on
	LD	HL,SAAENV
	LD	A,(SAABASE)
	OR	A
	JR	Z,SAA_XE3
	INC	HL
SAA_XE3:
	LD	A,(HL)
	OR	A
	RET	Z		; already off
	LD	(HL),0
	LD	E,0
	LD	A,(SAABASE)	; recompute register from base channel
	OR	A
	LD	A,$18
	JR	Z,SAA_XE4
	INC	A
SAA_XE4:
	JP	SAA_WREG	; write 0 = generator off, and return

;===============================================================================
; SAA1099 backend storage and tables
;===============================================================================
; All storage is emitted (.DB) — no .DS reservations in the .COM image region
; (they shift load addresses; see AGENTS.md).  SAA_INIT zeroes the shadows.

SAASHD:				; shadow state cleared by SAA_INIT
SAAS10	.DB	0		; shadow of SAA reg $10 (octaves ch 0/1)
SAAS11	.DB	0		; shadow of SAA reg $11 (octaves ch 2/3)
SAAS12	.DB	0		; shadow of SAA reg $12 (octaves ch 4/5)
SAAS14	.DB	0		; shadow of SAA reg $14 (frequency enable)
SAAS15	.DB	0		; shadow of SAA reg $15 (noise enable)
SAAS16	.DB	0		; shadow of SAA reg $16 (noise gen params)
SAAENV	.DB	0,0		; envelope generator 0/1 in-use flags
SAASHDL	.EQU	$-SAASHD

SAABASE	.DB	0		; SAA base channel (0 or 3)
SAACH	.DB	0		; SAA channel being translated
SAAENVB	.DB	0		; envelope used by current block this frame
SAATMP	.DB	0		; octave save slot
SAAD	.DB	0,0,0		; 24-bit tone divider work value

SAASHAPES:			; AY envelope shape (reg 13) -> SAA env reg value
				; bit7=enable, bits3-1=mode, bit5=0 (internal clock)
	.DB	$84,$84,$84,$84	; AY 0-3: \___ -> single decay
	.DB	$8C,$8C,$8C,$8C	; AY 4-7: /___ -> single attack
	.DB	$86		; AY 8:    \\\\ -> repetitive decay
	.DB	$84		; AY 9:    \___ -> single decay
	.DB	$8A		; AY 10:   \/\/ -> repetitive triangle
	.DB	$84		; AY 11:   \--- -> single decay (no hold, approx)
	.DB	$8E		; AY 12:   //// -> repetitive attack
	.DB	$8C		; AY 13:   /--- -> single attack (no hold, approx)
	.DB	$8A		; AY 14:   /\/\ -> repetitive triangle
	.DB	$8C		; AY 15:   /___ -> single attack
