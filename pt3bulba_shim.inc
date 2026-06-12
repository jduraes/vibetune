; Bulba player glue for standalone VibeTune (maps tune.asm expectations).

_ZX		.EQU	0
_MSX		.EQU	0
_WBW		.EQU	1
CPUFAMZ180	.EQU	1
SBCV2004	.EQU	0
CurPosCounter	.EQU	0
ACBBAC		.EQU	0
LoopChecker	.EQU	0	; CHECKLP POP corrupts stack when loop disabled (bit0=0)
Id		.EQU	0

#DEFINE MDLADDR MUSIC_BUF
#DEFINE PORTS BULBA_PORTS
#DEFINE Z180 Z180_IO_BASE
#DEFINE DCSAV PSG_IO_DCSAV

ISHBIOS:
	LD	A, (HBIOSMD)
	OR	A
	RET

SLOWIO:
	JP	PSG_IO_SLOW_ENTER

NORMIO:
	JP	PSG_IO_SLOW_EXIT

; Copy RomWBW-detected / CLI-selected ports into Bulba ROUT layout (RSEL, RDAT).
; Does not choose ports — DETECT_HARDWARE_CONFIG sets PSG_REG_PORT/PSG_DATA_PORT.
BULBA_SYNC_PORTS:
	LD	A, (PSG_REG_PORT)
	LD	(BULBA_PORTS + 0), A
	LD	A, (PSG_DATA_PORT)
	LD	(BULBA_PORTS + 1), A
	RET

; tune.com keeps these as emitted .DB 0 (NOT .DS): a .DS here leaves a gap that
; TASM omits from the .com image, shifting every following routine and breaking
; all absolute CALLs (silent startup). Emitting real bytes keeps the image flat.
OCTAVEADJ:
	.DB	0
TMP:
	.DB	0
