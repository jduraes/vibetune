;===============================================================================
; VibeTune UI Messages
; Extracted from vibetune.asm for modularization
;===============================================================================

; Main application messages - version string defined in build_meta.inc
MSGBAN  .EQU    VERSION_STR         ; Use auto-generated version string
MSGUSE		.DB	"Based on RomWBW Tune v3.13, 28-May-2025",13,10
		.DB	"Copyright (C) 2025, Wayne Warthen, GNU GPL v3",13,10,13,10
		.DB	"PTxPlayer Copyright (C) 2004-2007 S.V.Bulba",13,10
		.DB	"MYMPlay by Marq/Lieves!Tuore",13,10
		.DB	"VGMPlay by J.B. Langston, Marco Maccaferri, Ed Brindley",13,10,13,10
		.DB	"Usage: VIBETUNE <filename>.[PT2|PT3|MYM|VGM] [-msx|-rc] [-delay] [--hbios] [+tn|-tn]",0

; Error messages
MSGBIO		.DB	"Incompatible BIOS or version, "
		.DB	"HBIOS v", '0' + RMJ, ".", '0' + RMN, " required",0
MSGPLT		.DB	"Hardware error, system not supported!",0
MSGHW		.DB	"Hardware error, sound chip not detected!",0
MSGNAM		.DB	"Sound filename invalid (must be .PT2, .PT3, .MYM, or .VGM)",0
MSGFIL		.DB	"Sound file not found!",0
MSGSIZ		.DB	"Sound file too large to load!",0
MSGERR		.DB	"App Error", 0

; Status messages
MSGTIM		.DB	", timer mode",0
MSGDLY		.DB	", delay mode",0
MSGPLY		.DB	"Playing...",0
MSGEND		.DB	" Done",0

; Song information messages
MSGSONGNAME	.DB	"Song name: ", 0
MSGARTIST	.DB	"by:        ", 0

; Special messages
MSGUNSUP	.db	"MYM files not supported with HBIOS yet!\r\n", 0
MSGNOTD00	.DB	"D00 playback not yet implemented - coming soon!",13,10,0

; VGM display messages (adplay style)
MSGVGMPLAY	.DB	"Playing '",0
MSGVGMDOT3	.DB	"'...",0
MSGVGMTYPE	.DB	"Type  : VGM (",0
MSGVGMPSG	.DB	"SN76489",0
MSGVGMAY	.DB	"AY-3-8910",0
MSGVGMAY2	.DB	"2x AY-3-8910",0
MSGVGMOPL2	.DB	"OPL2",0
MSGVGMOPL3	.DB	"OPL3",0
MSGVGMUNK	.DB	"Unknown",0


; Debug messages for VGM timing
MSGCPUTYPE	.DB	" CPU:", 0
MSGZ80		.DB	"Z80", 0
MSGZ180		.DB	"Z180", 0
MSGZ280		.DB	"Z280", 0
MSGVGMFDLY	.DB	" vgmfdly=", 0
