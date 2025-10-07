;===============================================================================
; VibeTune File Type Detection Module
; Extracted from vibetune.asm for modularization
;===============================================================================

;-------------------------------------------------------------------------------
; DETECT_FILE_TYPE
; Analyzes FCB filename and extension to determine file type
; Input:  FCB contains filename and extension
; Output: A = file type constant (TYPPT2, TYPPT3, or TYPMYM)
;         FILTYP variable updated with detected type
; Uses:   A, C
;-------------------------------------------------------------------------------
DETECT_FILE_TYPE:
	; Check sound filename (must be *.PT2, *.PT3, or *.MYM)
	LD	A,(FCB+1)		; Get first char of filename
	CP	' '			; Compare to blank
	JP	Z,ERRCMD		; If so, missing filename
	LD	A,(FCB+9)		; If the filetype
	CP	' '			; is blanks
	JR	NZ,HASEXT		; then assume
	LD	A,'P'			; type PT3.
	LD	(FCB+9),A
	LD	A,'T'			; Fill in
	LD	(FCB+10),A		; the file
	LD	A,'3'			; extension
	LD	(FCB+11),A		; and the
	LD	C,TYPPT3		; file type
	JR	_SETTYPE
HASEXT	LD	A,(FCB+9)		; Extension char 1
	CP	'P'			; Check for 'P'
	JP	NZ,CHKMYM		; If not, check for MYM extension
	LD	A,(FCB+10)		; Extension char 2
	CP	'T'			; Check for 'T'
	JP	NZ,ERRNAM		; If not, bad file extension
	LD	A,(FCB+11)		; Extension char 3
	LD	C,TYPPT2		; Assume PT2 file type
	CP	'2'			; Check for '2'
	JR	Z,_SETTYPE		; If so, commit file type value
	LD	C,TYPPT3		; Assume PT3 file type
	CP	'3'			; Check for '3'
	JR	Z,_SETTYPE		; If so, commit file type value
	JP	ERRNAM			; Anything else is a bad file extension
CHKMYM	LD	A,(FCB+9)		; Extension char 1
	CP	'M'			; Check for 'M'
	JP	NZ,CHKVGM		; If not, check for VGM extension
	LD	A,(FCB+10)		; Extension char 2
	CP	'Y'			; Check for 'Y'
	JP	NZ,ERRNAM		; If not, bad file extension
	LD	A,(FCB+11)		; Extension char 3
	LD	C,TYPMYM		; Assume MYM file type
	CP	'M'			; Check for 'M'
	JR	Z,_SETTYPE		; If so, commit file type value
	JP	ERRNAM			; Anything else is a bad file extension
CHKVGM	LD	A,(FCB+9)		; Extension char 1
	CP	'V'			; Check for 'V'
	JP	NZ,ERRNAM		; If not, bad file extension
	LD	A,(FCB+10)		; Extension char 2
	CP	'G'			; Check for 'G'
	JP	NZ,ERRNAM		; If not, bad file extension
	LD	A,(FCB+11)		; Extension char 3
	LD	C,TYPVGM		; Assume VGM file type
	CP	'M'			; Check for 'M'
	JR	Z,_SETTYPE		; If so, commit file type value
	JP	ERRNAM			; Anything else is a bad file extension
_SETTYPE LD	A,C			; Get file type value
	LD	(FILTYP),A		; Save file type value
	RET				; Return with A = file type