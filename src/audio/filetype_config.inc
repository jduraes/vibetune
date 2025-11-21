;===============================================================================
; VibeTune File Type Configuration Module
; File-type specific configurations for load addresses and timing parameters
;===============================================================================

;-------------------------------------------------------------------------------
; CONFIGURE_FILE_LOAD_ADDRESS  
; Sets the appropriate DMA load address based on file type
; Input:  A = file type (TYPPT2, TYPPT3, TYPMYM, TYPVGM, or TYPD00)
; Output: DMA variable set to appropriate load address
; Uses:   A, HL
;-------------------------------------------------------------------------------
CONFIGURE_FILE_LOAD_ADDRESS:
	LD	HL,MDLADDR		; Assume load address
	LD	(DMA),HL		; ... for PTx files
	CP	TYPMYM			; MYM file?
	JR	Z,LOAD_MYM		; If so, set MYM address
	CP	TYPVGM			; VGM file?
	JR	Z,LOAD_VGM		; If so, set VGM address
	CP	TYPD00			; D00 file?
	JR	Z,LOAD_D00		; If so, set D00 address
	RET				; Otherwise, use default
LOAD_MYM:
	LD	HL,rows			; MYM load address
	LD	(DMA),HL		; ... for MYM files
	RET
LOAD_VGM:
	LD	HL,vgmdata		; VGM load address
	LD	(DMA),HL		; ... for VGM files
	RET
LOAD_D00:
	LD	HL,d00data		; D00 load address
	LD	(DMA),HL		; ... for D00 files
	RET

;-------------------------------------------------------------------------------
; CONFIGURE_FILE_PLAYBACK
; Configures file-specific playback settings and starts appropriate player
; Input:  A = file type (TYPPT2, TYPPT3, TYPMYM, TYPVGM, or TYPD00)
; Uses:   A, DE, HL
;-------------------------------------------------------------------------------
CONFIGURE_FILE_PLAYBACK:
	CP	TYPPT2			; PT2?
	JR	Z,CONFIG_PT2		; If so, do it
	CP	TYPPT3			; PT3?
	JR	Z,CONFIG_PT3		; If so, do it
	CP	TYPMYM			; MYM?
	JR	Z,CONFIG_MYM		; If so, do it
	CP	TYPVGM			; VGM?
	JP	Z,CONFIG_VGM		; If so, do it (JP not JR - distance!)
	CP	TYPD00			; D00?
	JP	Z,CONFIG_D00		; If so, do it (JP not JR - distance!)
	JP	ERRNAM			; This should never happen

CONFIG_PT2:
	LD	A,2			; SETUP value to PT2 sound files
	LD	(START+10),A		; Save it
	; Avg TS / quark for PT2 files has *not* been measured!!!
	LD	DE,185			; Avg TS / quark = 7400, so 185 delay loops
	JR	CONFIG_PTX		; Play PTx file

CONFIG_PT3:
	LD	A,0			; SETUP value to PT3 sound files
	LD	(START+10),A		; Save it
	LD	DE,185			; Avg TS / quark = 7400, so 185 delay loops
	JR	CONFIG_PTX		; Play PTx file

CONFIG_PTX:
	LD	HL,(QDLY)		; Get basic quark delay
	OR	A			; Clear carry
	SBC	HL,DE			; Adjust for file type
	LD	(QDLY),HL		; Save updated quark delay factor

	CALL	CRLF2
	LD	DE, MSGSONGNAME         ; Print song name message
	CALL	PRTSTR
	LD	DE, MDLADDR + $1E       ; Print 32 character long song name from module
	LD	B, $20
CONFIG_PTX_SONG:
	LD	A,(DE)
	CALL	PRTCHR
	INC	DE
	DJNZ	CONFIG_PTX_SONG
	CALL	CRLF
	LD	DE, MSGARTIST           ; Print "by" message
	CALL	PRTSTR
	LD	DE, MDLADDR + $42       ; Print 32 character long composer/artist from module
	LD	B,  $20
CONFIG_PTX_ARTIST:
	LD	A,(DE)
	CALL	PRTCHR
	INC	DE
	DJNZ	CONFIG_PTX_ARTIST
	CALL	CRLF2			; Formatting
	LD	DE,MSGPLY		; Playing message
	CALL	PRTSTR			; Print message
	CALL	START			; Do initialization
PTXLP	CALL	START+5			; Play one quark
	LD	A,(START+10)		; Get setup byte
	BIT	7,A			; Check bit 7 (loop point passed)
	JR	NZ,CONFIG_PTX_EXIT	; Bail out when done playing
	CALL	GETKEY			; Check for keypress
	JR	NZ,CONFIG_PTX_EXIT	; Abort on keypress
	CALL	WAITQ			; Wait one quark period
	JR	PTXLP			; Loop for next quark
CONFIG_PTX_EXIT:
	RET				; Return to main program for proper exit

CONFIG_MYM:
	CALL	CRLF2			; Formatting
	LD	DE,MSGPLY		; Playing message
	CALL	PRTSTR			; Print message
	LD	HL,(QDLY)		; Get basic quark delay
	OR	A			; Clear carry
	LD	DE,125			; Avg TS / quark = ~5000, so 125 delay loops
	SBC	HL,DE			; Adjust for file type
	LD	(QDLY),HL		; Save updated quark delay factor
	CALL	mymini			; Initialize player
        CALL    extract         	; Unpack the first fragment
MYMLP	CALL	extract
	JR	NC,CONFIG_MYM_EXIT	; CF clear at end of tune
WAITVB	CALL	WAITQ
	CALL	upsg			; Update PSG registers
	CALL	GETKEY			; Check for keypess
	JR	NZ,CONFIG_MYM_EXIT	; Bail out if so
	LD      A,(played)      	; Wait until VBI has played a fragment
        OR      A
        JR      NZ,WAITVB
        LD      (psource),IY
        LD      A,FRAG
        LD      (played),A
	JR	MYMLP
CONFIG_MYM_EXIT:
	RET				; Return to main program for proper exit

; VGM playback configuration - placeholder implementation
CONFIG_VGM:
	CALL	CRLF2			; Formatting

	; Display VGM information - adplay style
	LD	DE,MSGVGMPLAY		; "Playing '"
	CALL	PRTSTR
	LD	DE,FCB+1		; Print filename
	LD	B,8
VGM_FNAME:
	LD	A,(DE)
	CP	' '
	JR	Z,VGM_FEXT
	CALL	PRTCHR
	INC	DE
	DJNZ	VGM_FNAME
VGM_FEXT:
	LD	A,'.'
	CALL	PRTCHR
	LD	DE,FCB+9
	LD	B,3
VGM_FEXT2:
	LD	A,(DE)
	CP	' '
	JR	Z,VGM_FNAME_DONE
	CALL	PRTCHR
	INC	DE
	DJNZ	VGM_FEXT2
VGM_FNAME_DONE:
	LD	A,0x27			; Single quote
	CALL	PRTCHR
	LD	DE,MSGVGMDOT3		; "..."
	CALL	PRTSTR
	CALL	CRLF
	
	; Print "Type : VGM (chip)"
	LD	DE,MSGVGMTYPE		; "Type  : VGM ("
	CALL	PRTSTR
	
	; Detect chip - check most common first
	LD	HL,(vgmdata + 50H)	; YM3812 (OPL2)
	LD	A,H
	OR	L
	JR	Z,VGM_CHECK_OPL3
	LD	DE,MSGVGMOPL2
	JR	VGM_CHIP_FOUND
VGM_CHECK_OPL3:
	LD	HL,(vgmdata + 5CH)	; YMF262 (OPL3)
	LD	A,H
	OR	L
	JR	Z,VGM_CHECK_AY
	LD	DE,MSGVGMOPL3
	JR	VGM_CHIP_FOUND
VGM_CHECK_AY:
	LD	HL,(vgmdata + 74H)	; AY-3-8910 (v1.61+)
	LD	A,H
	OR	L
	JR	Z,VGM_CHECK_PSG
	; Quick scan for dual chip (check first 64 bytes)
	LD	HL,(vgmdata + 34H)	; Get VGM data start offset
	LD	DE,vgmdata + 34H
	ADD	HL,DE			; Calculate absolute start
	LD	B,64			; Scan only 64 bytes
VGM_AY_SCAN:
	LD	A,(HL)			; Get command
	CP	66H			; End of stream?
	JR	Z,VGM_AY_SINGLE		; Stop scan
	CP	0A0H			; AY write?
	JR	NZ,VGM_AY_NEXT
	INC	HL			; Point to register
	LD	A,(HL)			; Get register number
	BIT	7,A			; Chip 2 = bit 7 set
	LD	DE,MSGVGMAY2		; Dual chip message
	JR	NZ,VGM_CHIP_FOUND	; Found chip 2!
	DEC	HL			; Back up
VGM_AY_NEXT:
	INC	HL
	DJNZ	VGM_AY_SCAN
VGM_AY_SINGLE:
	LD	DE,MSGVGMAY		; Single chip
	JR	VGM_CHIP_FOUND
VGM_CHECK_PSG:
	LD	HL,(vgmdata + 0CH)	; SN76489
	LD	A,H
	OR	L
	JR	Z,VGM_UNKNOWN
	LD	DE,MSGVGMPSG
	JR	VGM_CHIP_FOUND
VGM_UNKNOWN:
	LD	DE,MSGVGMUNK
VGM_CHIP_FOUND:
	CALL	PRTSTR
	LD	A,')'			; Close paren
	CALL	PRTCHR
	CALL	CRLF2
	
	LD	DE,MSGPLY		; Playing message
	CALL	PRTSTR			; Print message
	
	; Calculate dynamic frame delay based on CPU speed
	; Formula: vgmfdly = ((CPU_KHz * 2) / 1024) * 11 / 16
	; This scales correctly for all CPU speeds (tuned for 128 BPM)
	LD	HL,(QDLY)		; CPU_KHz/2
	ADD	HL,HL			; CPU_KHz
	ADD	HL,HL			; CPU_KHz * 2
	; Divide by 1024 = shift right 10 bits
	LD	A,H			; Get high byte (divides by 256)
	SRL	A			; Divide by 2 (now /512)
	SRL	A			; Divide by 2 (now /1024)
	; Now multiply by 11/16
	; A = A * 11 / 16
	LD	B,A			; Save original
	ADD	A,A			; A = A * 2
	ADD	A,B			; A = A * 3
	LD	C,A			; C = A * 3
	ADD	A,A			; A = A * 6
	ADD	A,A			; A = A * 12
	SUB	B			; A = A * 12 - A = A * 11
	; Divide by 16 (shift right 4)
	SRL	A			; /2
	SRL	A			; /4
	SRL	A			; /8
	SRL	A			; /16
	; Set minimum value
	CP	5			; Minimum for very slow CPUs
	JR	NC,VGM_FDLY_OK
	LD	A,5
VGM_FDLY_OK:
	LD	(vgmfdly),A		; Save frame delay
	LD	(vgmfdly),A		; Save frame delay
	LD	(vgmfdly),A		; Save frame delay
	CALL	VGM_MUTE_ALL		; Reset hardware
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
	CALL	VGM_PLAY_FRAME
	JR	Z,VGM_EXIT		; Exit if end of file (Z set by VGM_PLAY_FRAME)
	
	; Check for keypress
	CALL	GETKEY			; Check for key
	JR	NZ,VGM_EXIT		; Exit if key pressed
	
	; Apply frame delay
	CALL	VGM_APPLY_DELAY		; Wait for frame timing
	
	JR	VGM_MAINLOOP		; Loop

VGM_EXIT:
	CALL	VGM_MUTE_ALL		; Silence all hardware
	RET				; Return to main program for proper exit

CONFIG_D00:
	CALL	CRLF2			; Formatting
	LD	DE,MSGNOTD00		; D00 not implemented message
	CALL	PRTSTR			; Print message
	CALL	CRLF			; Formatting
	RET				; Return to main program for proper exit
