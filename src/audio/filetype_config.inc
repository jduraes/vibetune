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
	LD	DE,MSGPLY		; Playing message
	CALL	PRTSTR			; Print message
	
	; Calculate dynamic frame delay based on CPU speed
	LD	HL,(QDLY)		; CPU_KHz/2
	ADD	HL,HL			; Restore full CPU speed in KHz
	SRL	H			; Divide by 2
	RR	L
	SRL	H			; Divide by 4
	RR	L
	SRL	H			; Divide by 8 (~CPU_MHz * 2)
	RR	L
	LD	A,L			; Get result
	LD	B,A			; Save original
	SRL	A			; Divide by 2
	SRL	A			; Divide by 4
	SRL	A			; Divide by 8 (12.5%)
	ADD	A,B			; Add back = 112.5%
	CP	10			; Check minimum
	JR	NC,VGM_FDLY_OK
	LD	A,10			; Use minimum
VGM_FDLY_OK:
	LD	(vgmfdly),A		; Save frame delay
	; Initialize VGM playback
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
