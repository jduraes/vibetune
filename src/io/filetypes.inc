;===============================================================================
; VibeTune File Type Constants
; Extracted from vibetune.asm for modularization
;===============================================================================

; File type identification values
TYPPT2		.EQU	1		; FILTYP value for PT2 sound file
TYPPT3		.EQU	2		; FILTYP value for PT3 sound file
TYPMYM		.EQU	3		; FILTYP value for MYM sound file
TYPVGM		.EQU	4		; FILTYP value for VGM sound file
TYPD00		.EQU	5		; FILTYP value for D00 sound file

; Audio port selection modes
PORTS_AUTO	.EQU	0		; AUTO select audio chip ports
PORTS_MSX	.EQU	1		; force MSX audio chip ports
PORTS_RC	.EQU	2		; force RCBUS audio chip ports

; Memory and system constants
HEAPEND		.EQU	$E000		; End of heap storage (increased for VGM files)
#DEFINE Release "1"		; Release number
