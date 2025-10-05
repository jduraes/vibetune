;===============================================================================
; VibeTune File Type Configuration Module
; File-type specific configurations for load addresses and timing parameters
;===============================================================================

;-------------------------------------------------------------------------------
; CONFIGURE_FILE_LOAD_ADDRESS  
; Sets the appropriate DMA load address based on file type
; Input:  A = file type (TYPPT2, TYPPT3, or TYPMYM)
; Output: DMA variable set to appropriate load address
; Uses:   A, HL
;-------------------------------------------------------------------------------
CONFIGURE_FILE_LOAD_ADDRESS:
	LD	HL,MDLADDR		; Assume load address
	LD	(DMA),HL		; ... for PTx files
	CP	TYPMYM			; MYM file?
	RET	NZ			; If not, all set
	LD	HL,rows			; Otherwise, load address
	LD	(DMA),HL		; ... for MYM files
	RET

;-------------------------------------------------------------------------------
; CONFIGURE_FILE_PLAYBACK
; Configures file-specific playback settings and starts appropriate player
; Input:  A = file type (TYPPT2, TYPPT3, or TYPMYM)
; Uses:   A, DE, HL
;-------------------------------------------------------------------------------
CONFIGURE_FILE_PLAYBACK:
	CP	TYPPT2			; PT2?
	JR	Z,CONFIG_PT2		; If so, do it
	CP	TYPPT3			; PT3?
	JR	Z,CONFIG_PT3		; If so, do it
	CP	TYPMYM			; MYM?
	JR	Z,CONFIG_MYM		; If so, do it
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
	JR	NZ,EXIT			; Bail out when done playing
	CALL	GETKEY			; Check for keypress
	JR	NZ,EXIT			; Abort on keypress
	;LD	A,13			; Back to
	;CALL	PRTCHR			; ... start of line
	;LD	A,(CurPos)		; Get current position
	;CALL	PRTHEX			; ... and display it
	CALL	WAITQ			; Wait one quark period
	JR	PTXLP			; Loop for next quark

CONFIG_MYM:
	CALL	CRLF2			; Formatting
	LD	DE,MSGPLY		; Playing message
	CALL	PRTSTR			; Print message
	LD	HL,(QDLY)		; Get basic quark delay
	OR	A			; Clear carry
	LD	DE,125			; Avg TS / quark = ~5000, so 125 delay loops
	SBC	HL,DE			; Adjust for file type
	LD	(QDLY),HL		; Save updated quark delay factor
	;LD	BC,(rows)
	;CALL	PRTHEXWORD
	CALL	mymini			; Initialize player
        CALL    extract         	; Unpack the first fragment
MYMLP	CALL	extract
	JR	NC,EXIT			; CF clear at end of tune
WAITVB	CALL	WAITQ
	CALL	upsg			; Update PSG registers
	CALL	GETKEY			; Check for keypess
	JR	NZ,EXIT			; Bail out if so
	LD      A,(played)      	; Wait until VBI has played a fragment
        OR      A
        JR      NZ,WAITVB
        LD      (psource),IY
        LD      A,FRAG
        LD      (played),A
	;CALL	PRTDOT
	JR	MYMLP
	RET				; Should never reach here