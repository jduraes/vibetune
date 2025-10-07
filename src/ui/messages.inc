;===============================================================================
; VibeTune UI Messages
; Extracted from vibetune.asm for modularization
;===============================================================================

; Main application messages
MSGBAN  .DB "VibeTune v0.2.6.8 for RomWBW, 07-Oct-2025",0
MSGUSE		.DB	"Based on RomWBW Tune v3.13, 28-May-2025",13,10
		.DB	"Copyright (C) 2025, Wayne Warthen, GNU GPL v3",13,10
		.DB	"PTxPlayer Copyright (C) 2004-2007 S.V.Bulba",13,10
		.DB	"MYMPlay by Marq/Lieves!Tuore",13,10,13,10
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
MSGNOTVGM	.DB	"VGM playback not yet implemented - coming soon!",13,10,0
