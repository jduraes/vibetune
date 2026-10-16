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
; base channel in A (0 = AY chip 1, 3 = AY chip 2).  In TS-dual mode each
; block owns SAA channels 0-2 / 3-5.  A single-chip file instead gets a
; fixed map across both groups (A->ch0, B->ch2, C->ch3), leaving ch 1 free
; as the envelope clock source.
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
; - Volume / stereo: the SAA has per-channel left/right amplitude nibbles
;   (reg $00-$05: high nibble = left, low = right), so the AY card's
;   hardwired ABC stereo is emulated in software: AY ch A hard left, ch C
;   hard right, ch B center at ~62% per side (golden-ratio split, matching
;   the RC2014 AY card's resistor network).  AY log volumes (2 dB/step)
;   are converted to SAA linear inside the pan lookup tables
;   SAAVFULL/SAAVCTR.
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
;   The generator modulates only ch 2/5 of its group, so an enveloped AY
;   channel is remapped onto it (TS mode: swap within the block; single
;   chip: fixed maps in SAACHMAPS).  If two channels of one block use the
;   envelope in the same frame only the lowest-indexed is remapped and the
;   other plays flat (the AY has one shared envelope, so this is rare).
;   Two SAA quirks are worked around (per MAME saa1099.cpp):
;   - the envelope generator has no CPU rate control: gen 0/1 steps on
;     every toggle of tone ch 1/4 (step rate = 2*f_tone).  In single-chip
;     mode ch 1 is kept free and programmed at envper*2 as a silent clock
;     (amplitude 0), matching the AY envelope step rate AYCLK/(16*envper).
;     TS-dual mode has no spare channel: the rate stays approximate.
;   - a channel with tone AND noise disabled outputs nothing at all, so an
;     AY-style envelope sustain (mixer tone+noise off, envelope flag on)
;     would be silent.  The channel's tone is forced on as a carrier whose
;     pitch is the envelope cycle rate (period = envper<<K), matching what
;     the AY actually sounds like during such a sustain.  envper 0 (frozen
;     envelope) maps to generator off.  In TS-dual mode the sustain carrier
;     also plays with the generator off (flat) - no spare clock channel.
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
	; single-chip vs TS-dual: single chip owns all 6 channels (fixed map,
	; ch1 envelope clock, both noise gens); TS-dual splits 0-2 / 3-5
	LD	A,(TSFLAG)
	OR	A
	JR	Z,SAA_XSC1
	LD	A,(TS_DUALHW)
	OR	A
	JR	Z,SAA_XSC1
	XOR	A		; TS-dual active
	JR	SAA_XSC2
SAA_XSC1:
	LD	A,1
SAA_XSC2:
	LD	(SAASC),A
	XOR	A
	LD	(SAAENVB),A	; no envelope user in this block yet
	DEC	A
	LD	(SAAENVCH),A	; 255 = no enveloped channel found
	LD	(SAACAR),A	; 255 = no forced carrier channel
	; --- pre-scan amplitudes for the AY envelope flag (bit 4).  The SAA
	; envelope generator only modulates channel 2 of its group (2/5), so
	; an enveloped AY channel is remapped onto it (lowest index wins).
	LD	B,3
	LD	C,0
SAA_XSCAN:
	LD	A,8
	ADD	A,C
	LD	E,A
	LD	D,0
	PUSH	IX
	POP	HL
	ADD	HL,DE
	LD	A,(HL)		; AY amplitude register
	AND	$10
	JR	Z,SAA_XSCAN1
	LD	A,C
	LD	(SAAENVCH),A
	LD	A,1
	LD	(SAAENVB),A
	JR	SAA_XSCAND
SAA_XSCAN1:
	INC	C
	DJNZ	SAA_XSCAN
SAA_XSCAND:
	; --- select this frame's channel map + enable-bit permutation ---
	; records 0-3 = single chip (none/envA/envB/envC), 4-7 = TS-dual
	LD	A,(SAAENVCH)	; 255/0/1/2 -> record 0/1/2/3
	INC	A
	CP	4
	JR	C,SAA_XMS1
	XOR	A
SAA_XMS1:
	LD	B,A
	LD	A,(SAASC)
	OR	A
	JR	NZ,SAA_XMS2
	LD	A,B
	OR	4		; TS-dual records 4-7
	LD	B,A
SAA_XMS2:
	LD	C,B		; C = record index 0..7
	LD	B,0
	LD	A,C
	ADD	A,A
	ADD	A,C		; *3
	LD	E,A
	LD	D,0
	LD	HL,SAACHMAPS
	ADD	HL,DE
	LD	(SAACHMP),HL
	LD	A,C
	ADD	A,A
	ADD	A,A
	ADD	A,A		; *8
	LD	E,A
	LD	HL,SAABMAPS
	ADD	HL,DE
	LD	(SAABTMP),HL
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
	; sustain carrier: an enveloped channel with tone AND noise disabled
	; outputs nothing on the SAA, killing AY-style envelope sustains -
	; force its tone on as a carrier.  The carrier pitch is the envelope
	; cycle rate, as on the AY where the envelope IS the waveform:
	; period = envper << K, K = 5 for the 32-step triangle shapes
	; (AY 10/14), else 4 (16-step).  Single chip: envelope clocked by ch1
	; at envper*2 (exact).  TS-dual: no spare clock channel, so the
	; carrier plays flat (generator off during the sustain).
	LD	A,(SAAENVCH)
	CP	3
	JR	NC,SAA_XFC9	; none (255) this block
	LD	B,A
	INC	B
	LD	A,1
SAA_XFC1:
	DEC	B
	JR	Z,SAA_XFC2
	ADD	A,A
	JR	SAA_XFC1
SAA_XFC2:			; A = enable bit for the enveloped AY channel
	LD	B,A
	LD	A,D
	OR	E
	AND	B
	JR	NZ,SAA_XFC9	; tone or noise already on: nothing to do
	LD	A,D
	OR	B
	LD	D,A		; force tone carrier on
	LD	A,(SAAENVCH)
	LD	(SAACAR),A	; the channel loop substitutes the carrier period
	LD	L,(IX+11)	; carrier period = envper << K
	LD	H,(IX+12)
	LD	B,4		; 16-step envelope shapes
	LD	A,(SAABASE)
	OR	A
	LD	A,(SAAENVSH)
	JR	Z,SAA_XFC3
	LD	A,(SAAENVSH+1)
SAA_XFC3:
	CP	10		; 32-step triangle shapes: AY 10, 14
	JR	Z,SAA_XFC4
	CP	14
	JR	NZ,SAA_XFC5
SAA_XFC4:
	INC	B
SAA_XFC5:
	LD	A,H
	OR	L
	JR	Z,SAA_XFC6	; envper 0: period 0, stays muted
SAA_XFC7:
	ADD	HL,HL
	DJNZ	SAA_XFC7
SAA_XFC6:
	LD	(SAACARP),HL
SAA_XFC9:
	; move the 3 channel enable bits through this frame's map
	LD	HL,(SAABTMP)
	CALL	SAA_XPERM
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
	LD	A,(SAASC)
	OR	A
	JR	NZ,SAA_XM_SC	; single chip owns all 6 bits
	LD	A,%00111000	; TS chip 1: other block owns bits 3-5
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
	JR	SAA_XM3
SAA_XM_SC:
	LD	A,(SAAENVB)	; envelope active: run ch1 tone as its clock
	OR	A
	JR	Z,SAA_XM_SC1
	LD	A,D
	OR	2
	LD	D,A
SAA_XM_SC1:
	LD	A,D
	LD	(SAAS14),A
	LD	A,E
	LD	(SAAS15),A
SAA_XM3:
	; --- per-channel amplitude / frequency / octave ---
	LD	B,3		; 3 channels per block
	LD	C,0		; AY channel index 0..2
SAA_XCH:
	CALL	SAA_PERMCH	; A = SAA channel for AY channel C (env remap)
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
	CALL	SAA_XAMP_CHAN	; E = panned amplitude byte for AY channel C
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
	LD	A,(SAACAR)	; envelope carrier channel: pitch is the
	CP	C		; envelope cycle rate, not the note period
	JR	NZ,SAA_XP1
	LD	HL,(SAACARP)
SAA_XP1:
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
	LD	A,(SAASC)
	OR	A
	JR	Z,SAA_XN4T
	LD	A,B		; single chip: C sits on ch 3 (noise gen 1) -
	ADD	A,A		; mirror the rate into both generators
	ADD	A,A
	ADD	A,A
	ADD	A,A
	OR	B
	JR	SAA_XN5
SAA_XN4T:
	LD	A,(SAAS16)
	AND	$F0		; TS chip 1: keep gen 1 bits
	OR	B
SAA_XN5:
	LD	(SAAS16),A
	LD	E,A
	LD	A,$16
	CALL	SAA_WREG
	; --- envelope generator ---
	LD	A,(SAABASE)	; HL = this block's shape shadow
	OR	A
	LD	HL,SAAENVSH
	JR	Z,SAA_XE0
	INC	HL
SAA_XE0:
	LD	A,(IX+13)	; AY envelope shape register
	OR	A
	JP	M,SAA_XE_NSH	; $FF = unchanged this frame
	AND	$0F
	LD	(HL),A		; latch the shape (like the AY's reg 13)
SAA_XE_NSH:
	LD	A,(SAAENVB)
	OR	A
	JR	Z,SAA_XE_OFF	; no channel uses the envelope this frame
	LD	A,(IX+11)	; envper 0: frozen envelope on the AY - the closest
	LD	D,A		; stable emulation is the generator off (flat channel
	LD	A,(IX+12)	; at full amplitude), not free-running at the clock
	OR	D		; channel's rate
	JR	Z,SAA_XE_OFF
	; TS-dual forced carrier: no spare clock channel for a sane envelope
	; rate - play the carrier flat instead of ring-modulating it at the
	; neighbor channel's melody rate
	LD	A,(SAASC)
	OR	A
	JR	NZ,SAA_XE_ON
	LD	A,(SAACAR)
	CP	3
	JR	C,SAA_XE_OFF
SAA_XE_ON:
	LD	A,(HL)		; latched shape
	LD	HL,SAASHAPES
	LD	E,A
	LD	D,0
	ADD	HL,DE
	LD	E,(HL)		; SAA env reg value: enable + mapped shape
	LD	HL,SAAENV	; shadow of last value written (0 = off)
	LD	A,(SAABASE)
	OR	A
	JR	Z,SAA_XE1
	INC	HL
SAA_XE1:
	LD	A,(HL)
	CP	E
	RET	Z		; already programmed: leave it free-running
	LD	(HL),E
	LD	A,(SAABASE)
	OR	A
	LD	A,$18
	JR	Z,SAA_XE2
	INC	A		; block 1 -> register $19
SAA_XE2:
	CALL	SAA_WREG
	; --- envelope clock: gen 0 steps on toggles of tone ch 1 (step rate =
	; 2*f_tone), so program ch 1 at envper*2 to match the AY envelope step
	; rate AYCLK/(16*envper).  TS-dual has no spare channel: skip.
	LD	A,(SAASC)
	OR	A
	RET	Z
	LD	L,(IX+11)	; AY envelope period
	LD	H,(IX+12)
	LD	A,H
	OR	L
	RET	Z		; envper 0: leave the clock as-is
	ADD	HL,HL		; clock tone = half the AY envelope step rate
	CALL	SAA_PER2MF	; A = m, B = octave
	LD	E,A
	LD	A,9		; $08+1: ch 1 frequency register
	CALL	SAA_WREG
	LD	A,B
	LD	C,1		; ch 1 octave -> SAAS10 high nibble
	CALL	SAA_SETOCT
	LD	A,(SAAS10)	; apply now (SETOCT preserved the ch 0 nibble)
	LD	E,A
	LD	A,$10
	CALL	SAA_WREG
	LD	E,0		; ch 1 amplitude = 0: silent clock
	LD	A,1
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
; SAA_XAMP_CHAN - Build panned SAA amplitude byte for one AY channel
;===============================================================================
; E = AY amplitude value (envelope flag already converted to $0F by caller)
; C = AY channel position (0 = A left, 1 = B center, 2 = C right)
; Returns E = SAA amplitude byte (high nibble = left, low nibble = right)
; Uses: AF, DE, HL.  Preserves: B, C (loop state in SAA_XLATE).
;===============================================================================
SAA_XAMP_CHAN:
	LD	A,E
	AND	$0F		; volume nibble v
	LD	E,A
	LD	D,0
	LD	HL,SAAVFULL
	ADD	HL,DE		; HL -> SAAVFULL[v]
	LD	A,C
	OR	A
	JR	Z,SAA_XAMP_A
	DEC	A
	JR	Z,SAA_XAMP_B
	LD	A,(HL)		; C: hard right (low nibble), left = 0
	LD	E,A
	RET
SAA_XAMP_A:
	LD	A,(HL)		; A: hard left (high nibble), right = 0
	ADD	A,A
	ADD	A,A
	ADD	A,A
	ADD	A,A
	LD	E,A
	RET
SAA_XAMP_B:			; B: both sides at ~62% (golden-ratio split)
	LD	HL,SAAVCTR
	ADD	HL,DE
	LD	A,(HL)
	LD	L,A
	ADD	A,A
	ADD	A,A
	ADD	A,A
	ADD	A,A
	OR	L
	LD	E,A
	RET

;===============================================================================
; SAA_PERMCH - Channel map: SAA channel for AY channel C
;===============================================================================
; Envelope generators modulate only ch 2/5 of each group, so the enveloped
; AY channel is mapped onto it.  Returns A = SAA channel number (SAABASE +
; mapped position) using this frame's 3-byte map at (SAACHMP).
; Uses: AF, HL.  Preserves: B, C, DE.
;===============================================================================
SAA_PERMCH:
	LD	HL,(SAACHMP)
	LD	A,L
	ADD	A,C
	LD	L,A
	JR	NC,SAA_PMC1
	INC	H
SAA_PMC1:
	LD	A,(HL)
	LD	HL,SAABASE
	ADD	A,(HL)
	RET

;===============================================================================
; SAA_XPERM - Permute the low 3 bits of D and E through the 8-byte LUT at HL
;===============================================================================
; Moves the tone/noise enable bits with the envelope channel remap.
; Updates: D, E.  Uses: AF (BC preserved).
;===============================================================================
SAA_XPERM:
	PUSH	BC
	LD	A,D
	AND	7
	LD	C,A
	LD	B,0
	PUSH	HL
	ADD	HL,BC
	LD	A,(HL)
	POP	HL
	LD	D,A
	LD	A,E
	AND	7
	LD	C,A
	ADD	HL,BC
	LD	A,(HL)
	LD	E,A
	POP	BC
	RET

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
SAAENV	.DB	0,0		; envelope generator 0/1: last value written (0 = off)
SAAENVSH	.DB	0,0	; envelope generator 0/1: last latched AY shape
SAASHDL	.EQU	$-SAASHD

SAABASE	.DB	0		; SAA base channel (0 or 3)
SAACH	.DB	0		; SAA channel being translated
SAAENVB	.DB	0		; envelope used by current block this frame
SAAENVCH	.DB	0		; enveloped AY channel (0-2) this block, 255 = none
SAASC	.DB	0		; 1 = single-chip mode (fixed map, ch1 env clock)
SAACAR	.DB	0		; forced-carrier AY channel this block, 255 = none
SAACARP	.DW	0		; carrier period (envper << K) for SAACAR
SAATMP	.DB	0		; octave save slot
SAAD	.DB	0,0,0		; 24-bit tone divider work value
SAACHMP	.DW	0		; this frame's 3-byte channel map (SAACHMAPS)
SAABTMP	.DW	0		; this frame's 8-byte enable-bit LUT (SAABMAPS)

SAAVFULL:			; AY log volume (0-15) -> SAA linear
	.DB	0,1,1,1,1,2,2,2	; AY = 2 dB/step: round(15 * 10^(-0.1*(15-v)))
	.DB	3,4,5,6,8,9,12,15
SAAVCTR:			; center channel side: FULL * 0.618 (golden ratio)
	.DB	0,0,0,1,1,1,1,1
	.DB	2,2,3,4,5,6,7,9

SAACHMAPS:			; channel maps: AY channel A,B,C -> SAA channel
	.DB	0,2,3		; 0 SC: no envelope
	.DB	2,0,3		; 1 SC: envelope on A
	.DB	0,2,3		; 2 SC: envelope on B (B maps to ch 2)
	.DB	0,3,2		; 3 SC: envelope on C
	.DB	0,1,2		; 4 TS: no envelope / envelope on C
	.DB	2,1,0		; 5 TS: envelope on A (swap A/C)
	.DB	0,2,1		; 6 TS: envelope on B (swap B/C)
	.DB	0,1,2		; 7 TS: envelope on C

SAABMAPS:			; enable-bit permutation LUTs, same record order:
				; index = AY tone/noise enable bits 0-2, value = SAA bits
	.DB	0,1,4,5,8,9,12,13	; 0 SC: A->0, B->2, C->3
	.DB	0,4,1,5,8,12,9,13	; 1 SC envA: A->2, B->0, C->3
	.DB	0,1,4,5,8,9,12,13	; 2 SC envB: same as no envelope
	.DB	0,1,8,9,4,5,12,13	; 3 SC envC: A->0, B->3, C->2
	.DB	0,1,2,3,4,5,6,7		; 4 TS: identity
	.DB	0,4,2,6,1,5,3,7		; 5 TS envA: swap bits 0/2
	.DB	0,1,4,5,2,3,6,7		; 6 TS envB: swap bits 1/2
	.DB	0,1,2,3,4,5,6,7		; 7 TS envC: identity

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
