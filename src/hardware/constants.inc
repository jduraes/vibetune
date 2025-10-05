;===============================================================================
; VibeTune Hardware Constants  
; Extracted from vibetune.asm for modularization
;===============================================================================

; HIGH SPEED CPU CONTROL
SBCV2004	.EQU	0		; ENABLE SBC-V2-004 HALF CLOCK DIVIDER
CPUFAMZ180	.EQU	1		; ENABLE Z180 WAIT STATE MANAGEMENT

; Conditional assembly constants
_ZX		.EQU    0		; Version of ROUT (ZX or MSX standards)
_MSX		.EQU    0
_WBW		.EQU    0
HBIOS		.EQU    0

; Player configuration constants  
CurPosCounter	.EQU	0		; Current position counter at (START+11)
ACBBAC		.EQU	0		; Allow channels allocation bits at (START+10)
LoopChecker	.EQU	1		; Allow loop checking and disabling
Id		.EQU	1		; Insert official identificator
#DEFINE Release "1"			; Release number

; Hardware description strings
HWSTR_SCG	.DB	"SCG ECB Board",0
HWSTR_N8	.DB	"N8 Onboard Sound",0
HWSTR_RCEB	.DB	"RCBus Sound Module (EB)",0
HWSTR_RCMSX	.DB	"RCBus Sound Module (MSX)",0
HWSTR_RCMF	.DB	"RCBus Sound Module (MF)",0
HWSTR_LINC	.DB	"Z50 LiNC Sound Module",0
HWSTR_MBC	.DB	"NHYODYNE Sound Module",0
HWSTR_DUO	.DB	"DUODYNE Sound Module",0
HWSTR_NABU	.DB	"NABU Onboard Sound",0
HWSTR_HEATH	.DB	"HEATH H8 MSX Module",0
HWSTR_MSX	.DB	"MSX Standard Ports (A0H/A1H)",0
HWSTR_RC	.DB	"RCBus Standard Ports (D8H/D0H)",0