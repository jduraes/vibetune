;===============================================================================
; VGM Player Module
;===============================================================================
; Ported from VGMPLAY.ASM by J.B. Langston, Marco Maccaferri, Ed Brindley
;
; Processes VGM (Video Game Music) command stream and outputs to sound hardware
;===============================================================================

; Frame delay calibration - scales with CPU speed
; Base: 15 for 7.3728 MHz, 38 for 18.432 MHz
; Approximates 1/44100 second delay
FRAME_DLY	.EQU	38

;===============================================================================
; VGM_PLAY_FRAME - Process one frame of VGM commands
;===============================================================================
; Processes VGM command stream until a wait command is encountered
; Returns when frame delay should be applied
;
; Uses: AF, BC, DE, HL
; Modifies: vgmpos, vgmdly
;===============================================================================
VGM_PLAY_FRAME:
	LD	HL,(vgmpos)		; Start processing VGM commands
VGM_NEXT:
	LD	A,(HL)			; Get command byte
	INC	HL			; Advance pointer
	LD	(vgmpos),HL		; Save position
	CP	66H			; 0x66: Restart VGM cmd
	JR	NZ,VGM_NEXT1
	LD	HL,(vgmdata + 1CH)	; Loop offset from header
	LD	A,H
	OR	L
	RET	Z			; No loop offset = end playback
	LD	DE,vgmdata + 1CH
	ADD	HL,DE			; Calculate absolute loop position
	LD	(vgmpos),HL		; Reset position to loop point
	JR	VGM_NEXT

VGM_NEXT1:
	CP	4FH			; 0x4F: Game Gear stereo (ignored)
	JR	NZ,VGM_OPL2
	INC	HL			; Skip one data byte
	LD	(vgmpos),HL		; Save position
	JR	VGM_NEXT

VGM_OPL2:
	CP	5AH			; 0x5A: Write to OPL2
	JR	NZ,VGM_OPL31
	LD	A,(HL)			; Get register
	INC	HL
	PUSH	AF			; Save register
	OUT	(OPL3ADDR1),A		; Select register
	PUSH	BC
	LD	B,3
	DJNZ	$
	POP	BC
	LD	E,(HL)			; Get data into E
	POP	AF			; Restore register to A
	CALL	VGM_BOOST_VOLUME	; Boost carrier only	; Boost if volume register
	LD	A,E			; Get (possibly boosted) data
	INC	HL
	OUT	(OPL3DATA1),A		; Write data
	LD	(vgmpos),HL		; Save position after data bytes
	JR	VGM_NEXT

VGM_OPL31:
	CP	5EH			; 0x5E: Write to OPL3 bank 1
	JR	NZ,VGM_OPL32
	LD	A,(HL)			; Get register
	INC	HL
	PUSH	AF
	OUT	(OPL3ADDR1),A		; Select register
	PUSH	BC
	LD	B,3
	DJNZ	$
	POP	BC
	LD	E,(HL)
	POP	AF
	CALL	VGM_BOOST_VOLUME	; Boost carrier only
	LD	A,E
	INC	HL
	OUT	(OPL3DATA1),A		; Write data
	LD	(vgmpos),HL		; Save position after data bytes
	JR	VGM_NEXT

VGM_OPL32:
	CP	5FH			; 0x5F: Write to OPL3 bank 2
	JR	NZ,VGM_PSG
	LD	A,(HL)			; Get register
	INC	HL
	PUSH	AF
	OUT	(OPL3ADDR2),A		; Select register
	PUSH	BC
	LD	B,3
	DJNZ	$
	POP	BC
	LD	E,(HL)
	POP	AF
	CALL	VGM_BOOST_VOLUME	; Boost carrier only
	LD	A,E
	INC	HL
	OUT	(OPL3DATA2),A		; Write data
	LD	(vgmpos),HL		; Save position after data bytes
	JR	VGM_NEXT

VGM_PSG:
	CP	50H			; 0x50: Write to SN76489
	JR	NZ,VGM_PSG2
	LD	A,(HL)			; Get data byte
	INC	HL
	OUT	(PSGREG),A		; Write to PSG
	LD	(vgmpos),HL		; Save position after data byte
	JR	VGM_NEXT

VGM_PSG2:
	CP	30H			; 0x30: Write to second SN76489
	JR	NZ,VGM_AY
	LD	A,(HL)			; Get data byte
	INC	HL
	OUT	(PSG2REG),A		; Write to second PSG
	LD	(vgmpos),HL		; Save position after data byte
	JR	VGM_NEXT

VGM_AY:
	CP	0A0H			; 0xA0: Write to AY-3-8910
	JR	NZ,VGM_WAITNN
	LD	A,(HL)			; Get register
	INC	HL
	BIT	7,A			; Bit 7=1 for second AY chip
	JR	Z,VGM_AY1
	AND	7FH			; Clear chip select bit
	OUT	(RSEL2),A		; Select register on chip 2
	LD	A,(HL)			; Get data
	INC	HL
	LD	(vgmpos),HL		; Save position after data bytes
	OUT	(RDAT2),A		; Write data to chip 2
	JP	VGM_NEXT

VGM_AY1:
	LD	C,A		; Save register number
	PUSH	HL		; Save data pointer
	LD	DE,(PORTS)	; E=RSEL, D=RDAT  
	LD	A,C		; Restore register
	LD	C,E		; C = RSEL port
	OUT	(C),A		; Select register
	POP	HL		; Restore data pointer
	LD	A,(HL)		; Get data
	INC	HL
	LD	(vgmpos),HL	; Save position
	LD	C,D		; C = RDAT port
	OUT	(C),A		; Write data
	JP	VGM_NEXT

VGM_WAITNN:
	CP	61H			; 0x61: Wait nn samples
	JR	NZ,VGM_WAIT60
	LD	A,(HL)			; Get low byte of wait count
	INC	HL
	LD	D,(HL)			; Get high byte of wait count
	INC	HL
	LD	(vgmpos),HL		; Save position
	LD	L,A
	LD	H,D			; HL = wait count
	LD	(vgmdly),HL		; Set delay
	OR	1			; Clear Z flag (not end of file)
	RET				; Return to apply delay

VGM_WAIT60:
	CP	62H			; 0x62: Wait 735 samples (60Hz)
	JR	NZ,VGM_WAIT50
	LD	(vgmpos),HL		; Save position
	LD	HL,735
	LD	(vgmdly),HL		; Set 60Hz frame delay
	OR	1			; Clear Z flag
	RET

VGM_WAIT50:
	CP	63H			; 0x63: Wait 882 samples (50Hz)
	JR	NZ,VGM_WAIT1
	LD	(vgmpos),HL		; Save position
	LD	HL,882
	LD	(vgmdly),HL		; Set 50Hz frame delay
	OR	1			; Clear Z flag
	RET

VGM_WAIT1:
	CP	70H			; 0x70-0x7F: Wait 1-16 samples
	JP	C,VGM_NEXT		; < 0x70: ignore unknown command
	CP	80H
	JP	NC,VGM_NEXT		; >= 0x80: ignore unknown command
	SUB	6FH			; Calculate wait count (1-16)
	LD	L,A
	LD	H,0
	LD	(vgmdly),HL		; Set short delay
	OR	1			; Clear Z flag
	RET

;===============================================================================
; VGM_APPLY_DELAY - Apply frame delay timing
;===============================================================================
; Implements tight timing loop to match VGM sample rate (~44.1 KHz)
;
; Uses: AF, BC, HL
; Modifies: vgmdly
;===============================================================================
VGM_APPLY_DELAY:
	LD	HL,(vgmdly)		; Get delay count
VGM_DELAY_LOOP:
	LD	A,(vgmfdly)
	LD	B,A			; CPU cycle calibration
	DJNZ	$			; Tight delay loop
	DEC	HL			; Decrement delay counter
	LD	A,H
	OR	L
	JR	NZ,VGM_DELAY_LOOP	; Loop until delay complete
	RET


;===============================================================================
; VGM_BOOST_VOLUME - Boost OPL volume if writing to volume register
;===============================================================================
; Checks if A (register) is 0x40-0x55 and reduces attenuation in E (data)
; Input: A = register, E = data
; Output: E = boosted data if volume register
; Uses: HL (preserved via stack)
;===============================================================================
VGM_BOOST_VOLUME:
	; Only boost carrier operators: 0x43,0x44,0x45,0x4B,0x4C,0x4D,0x53,0x54,0x55
	CP	43H
	JR	Z,VGM_DO_BOOST
	CP	44H
	JR	Z,VGM_DO_BOOST
	CP	45H
	JR	Z,VGM_DO_BOOST
	CP	4BH
	JR	Z,VGM_DO_BOOST
	CP	4CH
	JR	Z,VGM_DO_BOOST
	CP	4DH
	JR	Z,VGM_DO_BOOST
	CP	53H
	JR	Z,VGM_DO_BOOST
	CP	54H
	JR	Z,VGM_DO_BOOST
	CP	55H
	JR	Z,VGM_DO_BOOST
	RET				; Not a carrier, don't modify
VGM_DO_BOOST:
	; Boost carrier operator output
	PUSH	HL
	LD	A,E			; Get data
	AND	0C0H			; Keep only upper bits (KSL)
	; Attenuation bits (0-5) = 0 = max volume
	LD	E,A			; Store back
	POP	HL
	RET

;===============================================================================
; VGM_MUTE_ALL - Silence all sound hardware
;===============================================================================
; Mutes SN76489, AY-3-8910, and OPL3 chips
;
; Uses: AF, BC
;===============================================================================
VGM_MUTE_ALL:
	; Mute SN76489 channels (both chips)
	LD	A,9FH			; Channel 1 volume = 0
	OUT	(PSGREG),A
	OUT	(PSG2REG),A
	LD	A,0BFH			; Channel 2 volume = 0
	OUT	(PSGREG),A
	OUT	(PSG2REG),A
	LD	A,0DFH			; Channel 3 volume = 0
	OUT	(PSGREG),A
	OUT	(PSG2REG),A
	LD	A,0FFH			; Noise volume = 0
	OUT	(PSGREG),A
	OUT	(PSG2REG),A

	; Mute AY-3-8910 channels (both chips)
	LD	A,8			; Mixer register for channel A
	OUT	(RSEL),A
	OUT	(RSEL2),A
	XOR	A			; Volume = 0
	OUT	(RDAT),A
	OUT	(RDAT2),A
	LD	A,9			; Mixer register for channel B
	OUT	(RSEL),A
	OUT	(RSEL2),A
	XOR	A
	OUT	(RDAT),A
	OUT	(RDAT2),A
	LD	A,10			; Mixer register for channel C
	OUT	(RSEL),A
	OUT	(RSEL2),A
	XOR	A
	OUT	(RDAT),A
	OUT	(RDAT2),A

	; Turn off all OPL keys (0xB0-0xB8 on both banks = 18 channels)
	LD	C,0B0H			; Start at key-on register
VGM_OPL_KEYOFF:
	LD	A,C
	OUT	(OPL3ADDR1),A		; Bank 1
	PUSH	BC
	LD	B,3
	DJNZ	$
	POP	BC
	XOR	A			; Key off (bit 5=0)
	OUT	(OPL3DATA1),A
	LD	A,C
	OUT	(OPL3ADDR2),A		; Bank 2
	PUSH	BC
	LD	B,3
	DJNZ	$
	POP	BC
	XOR	A
	OUT	(OPL3DATA2),A
	INC	C
	LD	A,C
	CP	0B9H			; Done with 9 channels per bank?
	JR	NZ,VGM_OPL_KEYOFF

	; Clear OPL3 registers (0x00-0xFF on both banks)
	LD	BC,0100H		; B=1 (counter), C=0 (start reg)
VGM_OPL3_RESET_LOOP:
	LD	A,C			; Register number
	OUT	(OPL3ADDR1),A		; Select on bank 1
	PUSH	BC
	LD	B,3
	DJNZ	$
	POP	BC
	LD	A,0
	OUT	(OPL3DATA1),A		; Clear on bank 1
	LD	A,C
	OUT	(OPL3ADDR2),A		; Select on bank 2
	PUSH	BC
	LD	B,3
	DJNZ	$
	POP	BC
	LD	A,0
	OUT	(OPL3DATA2),A		; Clear on bank 2
	DEC	BC			; Next register
	LD	A,B
	OR	C
	JP	NZ,VGM_OPL3_RESET_LOOP
	RET
