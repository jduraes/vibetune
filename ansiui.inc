;===============================================================================
; ansiui.inc — ANSI/VT100 playlist UI for VibeTune
;
; Ported from the old VibeTune (SHOWPLSTATUS / UI_* / TCFG_* blocks) and adapted to
; the current TRACK_LIST storage (13-byte NUL-terminated "NAME.EXT" entries).
; All routines are no-ops when UI_ACTIVE = 0. UI_ACTIVE is set by UI_SETUP:
; RUN_MODE = RUNMODE_LIST and (DISP_MODE <> plain or ANSI-colour flag set).
;
; Screen layout (1-based rows):
;   1  banner (cyan)
;   2  key help (yellow)
;   4  input file + [Regular]/[TurboSound]
;   5  song name (PT3 $1E, 32 chars)
;   6  artist    (PT3 $42, 32 chars)
;   7  hardware (one line; mode encoded in the text)
;   9+ playlist tiles (16 chars each, UI_PLCOLS = CFG_COLS/16 per row)
;  after list + blank: State: Playing/Paused
;  next row: Looping Status: Off/Track/Playlist
;===============================================================================

#DEFINE UI_ROW_BANNER 1
#DEFINE UI_ROW_MODE   2
#DEFINE UI_ROW_INFO   4
#DEFINE UI_ROW_META1  5
#DEFINE UI_ROW_META2  6
#DEFINE UI_ROW_INFO2  7
#DEFINE UI_ROW_PLLIST 9

#DEFINE UI_TILE_W     16	; marker(1) + name(12) + gap(3)
#DEFINE UI_VALUE_COL  13	; value column after "Label:      " prefixes

;-------------------------------------------------------------------------------
; Mode setup
;-------------------------------------------------------------------------------

UI_SETUP:
	XOR	A
	LD	(UI_ACTIVE), A
	LD	(UI_INIT), A
	LD	A, (RUN_MODE)
	CP	RUNMODE_LIST
	RET	NZ
	LD	A, (DISP_MODE)
	OR	A
	JR	NZ, UI_SETUP1
	LD	A, (CFG_FLAGS)
	AND	$01
	RET	Z
UI_SETUP1:
	LD	A, $FF
	LD	(UI_ACTIVE), A
	RET

;-------------------------------------------------------------------------------
; Frame: clear screen once, banner row 1 (cyan), key help row 2 (yellow)
;-------------------------------------------------------------------------------

UI_ENSURE_INIT:
	LD	A, (UI_ACTIVE)
	OR	A
	RET	Z
	LD	A, (UI_INIT)
	OR	A
	RET	NZ
	CALL	TCFG_CLRHOME
	CALL	TCFG_COL_HDR
	LD	B, UI_ROW_BANNER
	LD	C, 1
	CALL	TCFG_ANSI_AT
	LD	DE, MSG_BANNER
	CALL	PRTSTR
	CALL	TCFG_COL_PRM
	LD	B, UI_ROW_MODE
	LD	C, 1
	CALL	TCFG_ANSI_AT
	LD	DE, MSG_PLKEYS
	CALL	PRTSTR
	CALL	TCFG_COL_RST
	LD	A, $FF
	LD	(UI_INIT), A
	RET

;-------------------------------------------------------------------------------
; UI_PLCOLS = max(1, CFG_COLS / UI_TILE_W)
;-------------------------------------------------------------------------------

UI_CALC_COLS:
	LD	A, (CFG_COLS)
	LD	B, 0
UI_CALC_COLS1:
	CP	UI_TILE_W
	JR	C, UI_CALC_COLS2
	SUB	UI_TILE_W
	INC	B
	JR	UI_CALC_COLS1
UI_CALC_COLS2:
	LD	A, B
	OR	A
	JR	NZ, UI_CALC_COLS3
	INC	A
UI_CALC_COLS3:
	LD	(UI_PLCOLS), A
	RET

;-------------------------------------------------------------------------------
; Full playlist render (tiles from row UI_ROW_PLLIST) + loop status line
;-------------------------------------------------------------------------------

UI_SHOW_PLAYLIST:
	LD	A, (UI_ACTIVE)
	OR	A
	RET	Z
	LD	A, (TRACK_COUNT)
	OR	A
	RET	Z
	CALL	UI_ENSURE_INIT
	CALL	UI_CALC_COLS
	CALL	UI_CLEAR_PLAYLIST
	LD	A, (TRACK_COUNT)
	LD	B, A			; B = remaining entries
	LD	C, 0			; C = entry index
	LD	A, UI_ROW_PLLIST
	LD	(UI_ROWCUR), A
	LD	A, 1
	LD	(UI_COLCUR), A
	XOR	A
	LD	(UI_COLUSED), A
UI_SHOWPL_LOOP:
	PUSH	BC
	LD	A, (TRACK_SELECTED)
	CP	C
	JR	NZ, UI_SHOWPL_MARK
	LD	A, '>'
	JR	UI_SHOWPL_MARKED
UI_SHOWPL_MARK:
	LD	A, ' '
UI_SHOWPL_MARKED:
	LD	(UI_MARK), A
	LD	A, (UI_ROWCUR)
	LD	B, A
	LD	A, (CFG_ROWS)
	CP	B
	JR	C, UI_SHOWPL_DONE	; no more visible rows
	LD	A, (UI_COLCUR)
	LD	C, A			; C (index) restored via POP BC below
	CALL	TCFG_ANSI_AT
	LD	A, C			; index is clobbered; recompute ptr from BC later
	POP	BC
	PUSH	BC
	LD	A, C
	CALL	UI_TRACK_PTR
	CALL	UI_PRT_TILE
	POP	BC
	INC	C
	LD	A, (UI_COLCUR)
	ADD	A, UI_TILE_W
	LD	(UI_COLCUR), A
	LD	A, (UI_COLUSED)
	INC	A
	LD	(UI_COLUSED), A
	LD	E, A
	LD	A, (UI_PLCOLS)
	CP	E
	JR	NZ, UI_SHOWPL_NEXT
	XOR	A
	LD	(UI_COLUSED), A
	LD	A, 1
	LD	(UI_COLCUR), A
	LD	A, (UI_ROWCUR)
	INC	A
	LD	(UI_ROWCUR), A
UI_SHOWPL_NEXT:
	DEC	B
	JP	NZ, UI_SHOWPL_LOOP
	CALL	TCFG_COL_RST		; belt: metadata/footer follow with default SGR
	RET
UI_SHOWPL_DONE:
	POP	BC
	CALL	TCFG_COL_RST
	RET

;-------------------------------------------------------------------------------
; HL = TRACK_LIST + A * TRACK_NAME_LEN
;-------------------------------------------------------------------------------

UI_TRACK_PTR:
	LD	HL, 0
	LD	B, A
UI_TRACK_PTR1:
	LD	A, B
	OR	A
	JR	Z, UI_TRACK_PTR2
	DEC	B
	LD	DE, TRACK_NAME_LEN
	ADD	HL, DE
	JR	UI_TRACK_PTR1
UI_TRACK_PTR2:
	LD	DE, TRACK_LIST
	ADD	HL, DE
	RET

;-------------------------------------------------------------------------------
; Print one 16-char tile: marker ('>' yellow / ' ') + name (12, space-padded)
; + 3-space gap. HL = NUL-terminated name.
;-------------------------------------------------------------------------------

UI_PRT_TILE:
	LD	A, (UI_MARK)
	CP	'>'
	JR	Z, UI_PRT_TILE_YEL
	CALL	TCFG_COL_RST		; clear prior selection highlight
	JR	UI_PRT_TILE1
UI_PRT_TILE_YEL:
	CALL	TCFG_COL_PRM
UI_PRT_TILE1:
	LD	A, (UI_MARK)
	CALL	PRTCHR
	LD	B, 12
UI_PRT_TILE2:
	LD	A, (HL)
	OR	A
	JR	Z, UI_PRT_TILE3
	CALL	PRTCHR
	INC	HL
	DEC	B
	JR	NZ, UI_PRT_TILE2
	JR	UI_PRT_TILE4
UI_PRT_TILE3:
	LD	A, ' '
	CALL	PRTCHR
	DEC	B
	JR	NZ, UI_PRT_TILE3
UI_PRT_TILE4:
	LD	B, 3
UI_PRT_TILE5:
	LD	A, ' '
	CALL	PRTCHR
	DJNZ	UI_PRT_TILE5
	CALL	TCFG_COL_RST		; always drop SGR after each tile
	RET

; Print UI_MARK with optional yellow highlight for '>'.
UI_PRT_MARK:
	LD	A, (UI_MARK)
	CP	'>'
	JR	NZ, UI_PRT_MARK1
	CALL	TCFG_COL_PRM
UI_PRT_MARK1:
	LD	A, (UI_MARK)
	CALL	PRTCHR
	LD	A, (UI_MARK)
	CP	'>'
	JR	NZ, UI_PRT_MARK2
	CALL	TCFG_COL_RST
UI_PRT_MARK2:
	RET

; Index in A → tile origin B=row, C=col (1-based). CY if off-screen.
UI_TILE_AT_INDEX:
	PUSH	DE
	PUSH	HL
	PUSH	AF			; save entry index (UI_CALC_COLS clobbers A)
	CALL	UI_CALC_COLS
	POP	AF
	LD	E, A
	XOR	A
	LD	D, A			; row offset from UI_ROW_PLLIST
	LD	A, (UI_PLCOLS)
	LD	L, A
UI_TILE_AT_INDEX1:
	LD	A, E
	CP	L
	JR	C, UI_TILE_AT_INDEX2
	SUB	L
	LD	E, A
	INC	D
	JR	UI_TILE_AT_INDEX1
UI_TILE_AT_INDEX2:
	LD	A, UI_ROW_PLLIST
	ADD	A, D
	LD	B, A
	LD	A, (CFG_ROWS)
	CP	B
	JR	NC, UI_TILE_AT_INDEX3
	SCF
	JR	UI_TILE_AT_INDEXX
UI_TILE_AT_INDEX3:
	LD	A, E
	ADD	A, A
	ADD	A, A
	ADD	A, A
	ADD	A, A			; * UI_TILE_W (16)
	INC	A
	LD	C, A
	AND	A
UI_TILE_AT_INDEXX:
	POP	HL
	POP	DE
	RET

;-------------------------------------------------------------------------------
; Redraw one tile. IN: A = entry index, UI_MARK = marker char.
;-------------------------------------------------------------------------------

UI_DRAW_PL_ENTRY:
	PUSH	AF
	LD	(UI_IDXTMP), A
	PUSH	BC
	PUSH	DE
	PUSH	HL
	LD	A, (UI_IDXTMP)
	CALL	UI_TILE_AT_INDEX
	JR	C, UI_DRAW_PL_ENTRYX
	CALL	TCFG_ANSI_AT
	LD	A, (UI_IDXTMP)
	CALL	UI_TRACK_PTR
	CALL	UI_PRT_TILE
UI_DRAW_PL_ENTRYX:
	POP	HL
	POP	DE
	POP	BC
	POP	AF
	RET

; Marker char only at tile origin (UI_MARK). IN: A = entry index.
UI_DRAW_MARKER_ONLY:
	PUSH	AF
	PUSH	BC
	PUSH	DE
	PUSH	HL
	CALL	UI_TILE_AT_INDEX
	JR	C, UI_DRAW_MARKER_ONLYX
	CALL	TCFG_ANSI_AT
	CALL	UI_PRT_MARK
UI_DRAW_MARKER_ONLYX:
	POP	HL
	POP	DE
	POP	BC
	POP	AF
	RET

;-------------------------------------------------------------------------------
; Move the '>' marker from TRACK_OLD to TRACK_SELECTED (two-tile delta)
;-------------------------------------------------------------------------------

UI_UPDATE_MARKER_DELTA:
	LD	A, (UI_ACTIVE)
	OR	A
	RET	Z
	LD	A, ' '
	LD	(UI_MARK), A
	LD	A, (TRACK_OLD)
	CALL	UI_DRAW_PL_ENTRY	; full tile — clears yellow highlight on name
	LD	A, '>'
	LD	(UI_MARK), A
	LD	A, (TRACK_SELECTED)
	CALL	UI_DRAW_PL_ENTRY
	JP	UI_CURSOR_ON_SELECTED

;-------------------------------------------------------------------------------
; Park the flashing VT cursor on the '>' of TRACK_SELECTED (tile origin).
; After printing a tile the cursor sits in the 3-space gap / next column;
; call this so the blink overlays the selection marker instead.
;-------------------------------------------------------------------------------

UI_CURSOR_ON_SELECTED:
	LD	A, (UI_ACTIVE)
	OR	A
	RET	Z
	LD	A, (TRACK_COUNT)
	OR	A
	RET	Z
	PUSH	BC
	PUSH	DE
	PUSH	HL
	LD	A, (TRACK_SELECTED)
	CALL	UI_TILE_AT_INDEX
	JR	C, UI_CUR_MARKX
	CALL	TCFG_ANSI_AT
UI_CUR_MARKX:
	POP	HL
	POP	DE
	POP	BC
	RET

;-------------------------------------------------------------------------------
; Footer after playlist: State row, then Looping Status (park cursor on '>')
;-------------------------------------------------------------------------------

UI_SHOW_FOOTER:
	LD	A, (PLAY_STATE)
	CP	PLAY_PAUSED
	JR	Z, UI_SHOW_FOOTER_P
	CALL	UI_DRAW_STATE_PLAYING
	JR	UI_SHOW_LOOP_STATUS
UI_SHOW_FOOTER_P:
	CALL	UI_DRAW_STATE_PAUSED
	; fall through

; "Looping Status: Off/Track/Playlist" on the row below State
UI_SHOW_LOOP_STATUS:
	LD	A, (UI_ACTIVE)
	OR	A
	RET	Z
	CALL	UI_FOOTER_STATE_ROW
	RET	C			; no room for State — skip loop too
	INC	B			; loop row = state + 1
	LD	A, (CFG_ROWS)
	CP	B
	RET	C
	PUSH	BC
	CALL	UI_CLR_ROW
	POP	BC
	LD	C, 1
	CALL	TCFG_ANSI_AT
	LD	DE, MSG_LOOPSTAT
	CALL	PRTSTR
	LD	A, (LOOP_MODE)
	CP	LOOP_TRACK
	JR	Z, UI_LOOPSTAT_TRK
	CP	LOOP_PLAYLIST
	JR	Z, UI_LOOPSTAT_PL
	LD	DE, MSG_LOOPSTAT_OFF
	JR	UI_LOOPSTAT_PRT
UI_LOOPSTAT_TRK:
	LD	DE, MSG_LOOPSTAT_TRK
	JR	UI_LOOPSTAT_PRT
UI_LOOPSTAT_PL:
	LD	DE, MSG_LOOPSTAT_PL
UI_LOOPSTAT_PRT:
	CALL	PRTSTR
	JP	UI_CURSOR_ON_SELECTED

; OUT: B = State row; CY set if off-screen. Playlist rows + blank + 1.
UI_FOOTER_STATE_ROW:
	CALL	UI_CALC_COLS
	LD	A, (UI_PLCOLS)
	LD	D, A
	LD	A, (TRACK_COUNT)
	LD	E, 0
UI_FOOTER_ROWS:
	OR	A
	JR	Z, UI_FOOTER_ROWS1
	INC	E
	SUB	D
	JR	NC, UI_FOOTER_ROWS
UI_FOOTER_ROWS1:
	LD	A, UI_ROW_PLLIST
	ADD	A, E
	INC	A			; blank line after playlist
	LD	B, A
	LD	A, (CFG_ROWS)
	CP	B
	RET				; CY if CFG_ROWS < B

;-------------------------------------------------------------------------------
; State: Playing (green) / Paused (yellow) — dynamic footer row
;-------------------------------------------------------------------------------

UI_STATUS_PLAYING:
	LD	A, (UI_ACTIVE)
	OR	A
	RET	Z
	CALL	UI_ENSURE_INIT
	CALL	UI_DRAW_STATE_PLAYING
	JP	UI_CURSOR_ON_SELECTED

UI_STATUS_PAUSED:
	LD	A, (UI_ACTIVE)
	OR	A
	RET	Z
	CALL	UI_ENSURE_INIT
	CALL	UI_DRAW_STATE_PAUSED
	JP	UI_CURSOR_ON_SELECTED

UI_DRAW_STATE_PLAYING:
	XOR	A
	JR	UI_DRAW_STATE_BODY
UI_DRAW_STATE_PAUSED:
	LD	A, 1
UI_DRAW_STATE_BODY:
	PUSH	AF
	CALL	UI_FOOTER_STATE_ROW
	POP	AF
	RET	C
	PUSH	AF
	PUSH	BC
	CALL	UI_CLR_ROW
	POP	BC
	POP	AF
	LD	C, 1
	CALL	TCFG_ANSI_AT
	OR	A
	CALL	NZ, TCFG_COL_PRM
	CALL	Z, TCFG_COL_OK
	JR	NZ, UI_DRAW_STATE_PAUSED_MSG
	LD	DE, MSG_STATE_PLAYING
	JR	UI_DRAW_STATE_PRT
UI_DRAW_STATE_PAUSED_MSG:
	LD	DE, MSG_STATE_PAUSED
UI_DRAW_STATE_PRT:
	CALL	PRTSTR
	JP	TCFG_COL_RST

; Position at row B / UI_VALUE_COL and erase the value field only.
UI_GOTO_VALUE_ROW:
	LD	C, UI_VALUE_COL
	CALL	TCFG_ANSI_AT
	JP	TCFG_EL_EOL

;-------------------------------------------------------------------------------
; Incremental track-status refresh (list mode track switch).
; Updates only the value columns on rows 4-7; playlist marker is separate
; (UI_UPDATE_MARKER_DELTA). No row clears, no footer redraw.
;-------------------------------------------------------------------------------

UI_TRACK_STATUS_UPDATE:
	LD	A, (UI_ACTIVE)
	OR	A
	RET	Z
	LD	B, UI_ROW_INFO
	CALL	UI_GOTO_VALUE_ROW
	LD	DE, ARG_BUFFER
	CALL	PRTSTR
	LD	A, (TSFLAG)
	OR	A
	JR	NZ, UI_TSU_TS
	LD	DE, MSG_FTYPE_REG
	JR	UI_TSU_FTYPE
UI_TSU_TS:
	LD	DE, MSG_FTYPE_TS
UI_TSU_FTYPE:
	CALL	PRTSTR
	LD	B, UI_ROW_META1
	CALL	UI_GOTO_VALUE_ROW
	LD	HL, METATITLE
	CALL	UI_PRT_META32
	LD	B, UI_ROW_META2
	CALL	UI_GOTO_VALUE_ROW
	LD	HL, METAARTIST
	CALL	UI_PRT_META32
	LD	B, UI_ROW_INFO2
	CALL	UI_GOTO_VALUE_ROW
	CALL	PRINT_PLAYBACK_HW_VALUE
	RET

;-------------------------------------------------------------------------------
; Track status block (rows 4-7): input+type, metadata, hardware
;-------------------------------------------------------------------------------

UI_TRACK_STATUS_BLOCK:
	LD	A, (UI_ACTIVE)
	OR	A
	RET	Z
	CALL	UI_ENSURE_INIT
	LD	B, UI_ROW_INFO
UI_TSB_CLEAR:
	CALL	UI_CLR_ROW
	LD	A, B
	CP	UI_ROW_INFO2
	JR	Z, UI_TSB_INFO
	INC	B
	JR	UI_TSB_CLEAR
UI_TSB_INFO:
	CALL	TCFG_COL_RST		; playlist may have left SGR yellow active
	LD	B, UI_ROW_INFO
	LD	C, 1
	CALL	TCFG_ANSI_AT
	CALL	PRINT_INPUT_FILE_LINE
	CALL	UI_PRINT_META
	LD	B, UI_ROW_INFO2
	LD	C, 1
	CALL	TCFG_ANSI_AT
	CALL	PRINT_PLAYBACK_HW_LINE
	; Footer state for the actual play/pause (avoids a Playing flash
	; when reloading into a paused state).
	LD	A, (PLAY_STATE)
	CP	PLAY_PAUSED
	JP	Z, UI_TSB_PAUSED
	JP	UI_TSB_PLAYING
UI_TSB_PAUSED:
	CALL	UI_STATUS_PAUSED
	JP	UI_SHOW_LOOP_STATUS
UI_TSB_PLAYING:
	CALL	UI_STATUS_PLAYING
	JP	UI_SHOW_LOOP_STATUS

;-------------------------------------------------------------------------------
; Full UI redraw (R key): frame + playlist + status block
;-------------------------------------------------------------------------------

UI_FULL_REDRAW:
	LD	A, (UI_ACTIVE)
	OR	A
	RET	Z
	XOR	A
	LD	(UI_INIT), A
	CALL	UI_SHOW_PLAYLIST
	JP	UI_TRACK_STATUS_BLOCK

;-------------------------------------------------------------------------------
; Metadata: snapshot PT3 title/author from MUSIC_BUF ($1E/$42, 32 chars each).
; Bytes outside 32..126 are sanitised to spaces so renderers can print raw.
; Non-PTx engines (MYM) have no header metadata: both fields are blanked.
;-------------------------------------------------------------------------------

META_SNAPSHOT:
	LD	A, (FILE_ENGINE)
	CP	ENGINE_PTX
	JR	Z, META_SNAP_PTX
	; Non-PTx (MYM): no header metadata — blank both fields.
	LD	DE, METATITLE
	CALL	META_BLANK32
	LD	DE, METAARTIST
META_BLANK32:
	LD	B, $20
	LD	A, ' '
META_BLANK321:
	LD	(DE), A
	INC	DE
	DJNZ	META_BLANK321
	RET
META_SNAP_PTX:
	LD	HL, MUSIC_BUF + $1E
	LD	DE, METATITLE
	CALL	META_COPY32
	LD	HL, MUSIC_BUF + $42
	LD	DE, METAARTIST
META_COPY32:
	LD	B, $20
META_COPY321:
	LD	A, (HL)
	CP	32
	JR	C, META_COPYFIX
	CP	127
	JR	C, META_COPY322
META_COPYFIX:
	LD	A, ' '
META_COPY322:
	LD	(DE), A
	INC	HL
	INC	DE
	DJNZ	META_COPY321
	RET

; Print metadata at rows 5/6 (UI mode).
UI_PRINT_META:
	LD	A, (UI_ACTIVE)
	OR	A
	RET	Z
	LD	B, UI_ROW_META1
	LD	C, 1
	CALL	TCFG_ANSI_AT
	LD	DE, MSG_SONGNAME
	CALL	PRTSTR
	LD	HL, METATITLE
	CALL	UI_PRT_META32
	LD	B, UI_ROW_META2
	LD	C, 1
	CALL	TCFG_ANSI_AT
	LD	DE, MSG_ARTIST
	CALL	PRTSTR
	LD	HL, METAARTIST
UI_PRT_META32:
	LD	B, $20
UI_PRT_META321:
	LD	A, (HL)
	INC	HL
	CALL	PRTCHR
	DJNZ	UI_PRT_META321
	RET

; Print metadata as plain lines (non-UI mode).
PRINT_SONG_META_PLAIN:
	LD	DE, MSG_SONGNAME
	CALL	PRTSTR
	LD	HL, METATITLE
	LD	B, $20
PSM_PLAIN1:
	LD	A, (HL)
	INC	HL
	CALL	PRTCHR
	DJNZ	PSM_PLAIN1
	CALL	CRLF
	LD	DE, MSG_ARTIST
	CALL	PRTSTR
	LD	HL, METAARTIST
	LD	B, $20
PSM_PLAIN2:
	LD	A, (HL)
	INC	HL
	CALL	PRTCHR
	DJNZ	PSM_PLAIN2
	JP	CRLF

;-------------------------------------------------------------------------------
; Park the cursor below the playlist before exiting to CP/M
;-------------------------------------------------------------------------------

UI_EXIT_TO_PROMPT:
	LD	A, (UI_ACTIVE)
	OR	A
	RET	Z
	CALL	UI_CALC_COLS
	LD	A, (TRACK_COUNT)
	OR	A
	JR	NZ, UI_EXIT_PROMPT1
	LD	A, UI_ROW_PLLIST + 1
	JR	UI_EXIT_PROMPT4
UI_EXIT_PROMPT1:
	LD	E, A			; remaining entries
	LD	A, (UI_PLCOLS)
	LD	D, A			; entries per row
	LD	B, 0			; rows used
UI_EXIT_PROMPT2:
	INC	B
	LD	A, E
	SUB	D
	JR	C, UI_EXIT_PROMPT3
	LD	E, A
	JR	UI_EXIT_PROMPT2
UI_EXIT_PROMPT3:
	LD	A, UI_ROW_PLLIST
	ADD	A, B			; last playlist row
	INC	A			; blank line
	INC	A			; State line
	INC	A			; Looping Status line
	INC	A			; blank before CP/M prompt
UI_EXIT_PROMPT4:
	LD	D, A
	LD	A, (CFG_ROWS)
	CP	D
	JR	NC, UI_EXIT_PROMPT5
	LD	D, A			; clamp to visible screen
UI_EXIT_PROMPT5:
	LD	B, D
	LD	C, 1
	CALL	TCFG_ANSI_AT
	CALL	TCFG_COL_RST
	RET

;-------------------------------------------------------------------------------
; Erase rows UI_ROW_PLLIST .. CFG_ROWS
;-------------------------------------------------------------------------------

; Erase playlist rows plus State/Loop footer rows (not the whole screen).
UI_CLEAR_PLAYLIST:
	PUSH	AF
	PUSH	BC
	PUSH	DE
	CALL	UI_PL_ROWS		; clobbers C — set the start row only after this
	LD	E, A
	LD	A, UI_ROW_PLLIST
	ADD	A, E
	INC	A			; blank line after tiles
	INC	A			; State row
	INC	A			; Loop row (+1 row: also cover a shrunk prior list)
	LD	E, A			; last row to clear
	LD	A, (CFG_ROWS)
	CP	E
	JR	NC, UI_CLEAR_PLAYLIST0
	LD	E, A
UI_CLEAR_PLAYLIST0:
	LD	C, UI_ROW_PLLIST	; first row to clear
UI_CLEAR_PLAYLIST1:
	LD	A, E
	CP	C
	JR	C, UI_CLEAR_PLAYLIST2
	PUSH	BC
	LD	B, C
	CALL	UI_CLR_ROW
	POP	BC
	INC	C
	JR	UI_CLEAR_PLAYLIST1
UI_CLEAR_PLAYLIST2:
	POP	DE
	POP	BC
	POP	AF
	RET

; Erase row B and leave the cursor at its column 1.
UI_CLR_ROW:
	PUSH	AF
	PUSH	BC
	PUSH	DE
	PUSH	HL
	LD	C, 1
	CALL	TCFG_ANSI_AT
	CALL	TCFG_EL2
	POP	HL
	POP	DE
	POP	BC
	POP	AF
	RET

;-------------------------------------------------------------------------------
; Low-level ANSI emitters (only called on UI-active paths)
;-------------------------------------------------------------------------------

; Z if plain mode without ANSI colour — skip CSI/SGR output.
TCFG_ANSI_ON:
	LD	A, (DISP_MODE)
	OR	A
	RET	NZ
	LD	A, (CFG_FLAGS)
	AND	$01
	RET

; Cursor position: B = row, C = col (1-based).
TCFG_ANSI_AT:
	PUSH	AF
	PUSH	BC
	PUSH	DE
	PUSH	HL
	LD	A, 27
	CALL	PRTCHR
	LD	A, '['
	CALL	PRTCHR
	PUSH	BC
	LD	A, B
	CALL	UI_PRTDECB
	LD	A, $3B
	CALL	PRTCHR
	POP	BC
	LD	A, C
	CALL	UI_PRTDECB
	LD	A, 'H'
	CALL	PRTCHR
	POP	HL
	POP	DE
	POP	BC
	POP	AF
	RET

; Emit DE string when ANSI colour mode is active (DISP_MODE or CFG bit 0).
TCFG_ANSI_STR:
	PUSH	AF
	CALL	TCFG_ANSI_ON
	JR	Z, TCFG_ANSI_STR0
	CALL	PRTSTR
TCFG_ANSI_STR0:
	POP	AF
	RET

; Clear screen + home. Emits only when DISP_MODE <> plain or colour flag set.
TCFG_CLRHOME:
	LD	DE, UI_SEQ_CLR
	JP	TCFG_ANSI_STR

; Erase current line (ESC[2K), same gating as TCFG_CLRHOME.
TCFG_EL2:
	LD	DE, UI_SEQ_EL2
	JP	TCFG_ANSI_STR

; Erase from cursor to end of line (ESC [ K).
TCFG_EL_EOL:
	LD	DE, UI_SEQ_EL
	JP	TCFG_ANSI_STR

; Cursor position: B = row, C = col (1-based).
TCFG_COL_HDR:
	LD	DE, UI_SEQ_CYAN
	JP	TCFG_EMIT

TCFG_COL_PRM:
	LD	DE, UI_SEQ_YEL
	JP	TCFG_EMIT

TCFG_COL_OK:
	LD	DE, UI_SEQ_GRN
	JP	TCFG_EMIT

TCFG_COL_RST:
	LD	DE, UI_SEQ_RST
	JP	TCFG_EMIT

TCFG_EMIT:
	PUSH	AF
	LD	A, (CFG_FLAGS)
	AND	$01
	JR	Z, TCFG_EMIT0
	CALL	PRTSTR
TCFG_EMIT0:
	POP	AF
	RET

; Print A (0-255) as decimal, leading zeros suppressed (for cursor addressing).
UI_PRTDECB:
	PUSH	BC
	PUSH	DE
	LD	B, 0
	LD	C, 100
	CALL	UI_PRTDECB_DIG
	LD	C, 10
	CALL	UI_PRTDECB_DIG
	ADD	A, '0'
	CALL	PRTCHR
	POP	DE
	POP	BC
	RET

UI_PRTDECB_DIG:
	LD	D, 0
UI_PRTDECB_DIG0:
	CP	C
	JR	C, UI_PRTDECB_DIG1
	SUB	C
	INC	D
	JR	UI_PRTDECB_DIG0
UI_PRTDECB_DIG1:
	LD	E, A
	LD	A, D
	OR	A
	JR	NZ, UI_PRTDECB_DIG2
	LD	A, B
	OR	A
	JR	Z, UI_PRTDECB_DIG3
UI_PRTDECB_DIG2:
	LD	B, $FF
	LD	A, D
	ADD	A, '0'
	CALL	PRTCHR
UI_PRTDECB_DIG3:
	LD	A, E
	RET

;-------------------------------------------------------------------------------
; Matrix navigation (WASD + arrow keys). Ported from VibeTune-old
; PLAYLIST_MOVE_WASD: there is no separate cursor — the '>' marker IS the
; current track, and an accepted move switches tracks (like N/P).
; Each UI_NAV_* returns A = new TRACK_SELECTED index.
;
; Plain-text -list (UI_ACTIVE=0) prints one track per line, so arrows/WASD
; move linearly (±1 with wrap). ANSI mode keeps the tile-grid matrix.
;-------------------------------------------------------------------------------

; Linear prev/next for the vertical plain list. Returns A = new index.
UI_NAV_LINEAR_PREV:
	LD	A, (TRACK_SELECTED)
	OR	A
	JR	NZ, UI_NAV_LIN_PREV_DEC
	LD	A, (TRACK_COUNT)
	DEC	A
	RET
UI_NAV_LIN_PREV_DEC:
	DEC	A
	RET

UI_NAV_LINEAR_NEXT:
	LD	A, (TRACK_SELECTED)
	INC	A
	LD	HL, TRACK_COUNT
	CP	(HL)
	RET	C
	XOR	A
	RET

; Total playlist rows = ceil(TRACK_COUNT / UI_PLCOLS). Returns A.
UI_PL_ROWS:
	CALL	UI_CALC_COLS
	LD	A, (TRACK_COUNT)
	LD	C, A			; remaining
	LD	B, 0			; rows
	LD	A, (UI_PLCOLS)
	LD	D, A
UI_PL_ROWS1:
	LD	A, C
	OR	A
	JR	Z, UI_PL_ROWS2
	INC	B
	SUB	D
	JR	C, UI_PL_ROWS2
	LD	C, A
	JR	UI_PL_ROWS1
UI_PL_ROWS2:
	LD	A, B
	RET

; Column of index A within its row: returns A = index mod UI_PLCOLS.
UI_PL_COL_OF:
	PUSH	AF			; UI_CALC_COLS clobbers A
	CALL	UI_CALC_COLS
	POP	AF
	LD	B, A			; idx
	LD	A, (UI_PLCOLS)
	LD	C, A
	LD	A, B
UI_PL_COL_OF1:
	CP	C
	JR	C, UI_PL_COL_OF2
	SUB	C
	JR	UI_PL_COL_OF1
UI_PL_COL_OF2:
	RET

; Up one row; wraps top -> bottom (column clamped on a short last row).
UI_NAV_UP:
	LD	A, (UI_ACTIVE)
	OR	A
	JP	Z, UI_NAV_LINEAR_PREV
	CALL	UI_CALC_COLS
	LD	A, (TRACK_SELECTED)
	LD	B, A
	LD	A, (UI_PLCOLS)
	LD	C, A			; C = cols
	LD	A, B
	SUB	C
	JR	NC, UI_NAV_UP_RET	; row above exists
	; wrap: idx + rows*cols, then clamp to last entry
	CALL	UI_PL_ROWS		; A = rows
	LD	B, A
	LD	A, (UI_PLCOLS)
	LD	C, A
	LD	A, (TRACK_SELECTED)
UI_NAV_UP_WRAP:
	ADD	A, C
	DJNZ	UI_NAV_UP_WRAP
	LD	HL, TRACK_COUNT
	CP	(HL)
	JR	C, UI_NAV_UP_RET
	LD	A, (TRACK_COUNT)
	DEC	A
UI_NAV_UP_RET:
	RET

; Down one row; wraps bottom -> same column in the top row.
UI_NAV_DOWN:
	LD	A, (UI_ACTIVE)
	OR	A
	JP	Z, UI_NAV_LINEAR_NEXT
	CALL	UI_CALC_COLS
	LD	A, (TRACK_SELECTED)
	LD	B, A
	LD	A, (UI_PLCOLS)
	ADD	A, B			; idx + cols
	LD	HL, TRACK_COUNT
	CP	(HL)
	JR	C, UI_NAV_DOWN_RET
	LD	A, B
	CALL	UI_PL_COL_OF		; wrap to top row, same column
UI_NAV_DOWN_RET:
	RET

; Left one column; wraps to the last entry of the current row.
UI_NAV_LEFT:
	LD	A, (UI_ACTIVE)
	OR	A
	JP	Z, UI_NAV_LINEAR_PREV
	LD	A, (TRACK_SELECTED)
	CALL	UI_PL_COL_OF
	OR	A
	JR	NZ, UI_NAV_LEFT_DEC
	; at column 0: wrap to min(rowstart + cols, count) - 1
	LD	A, (TRACK_SELECTED)	; row start
	LD	B, A
	LD	A, (UI_PLCOLS)
	ADD	A, B
	LD	HL, TRACK_COUNT
	CP	(HL)
	JR	C, UI_NAV_LEFT1
	LD	A, (TRACK_COUNT)
UI_NAV_LEFT1:
	DEC	A
	RET
UI_NAV_LEFT_DEC:
	LD	A, (TRACK_SELECTED)
	DEC	A
	RET

; Right one column; wraps to the first entry of the current row.
UI_NAV_RIGHT:
	LD	A, (UI_ACTIVE)
	OR	A
	JP	Z, UI_NAV_LINEAR_NEXT
	LD	A, (TRACK_SELECTED)
	CALL	UI_PL_COL_OF
	LD	B, A			; B = col
	LD	A, (UI_PLCOLS)
	DEC	A
	CP	B			; last column?
	JR	Z, UI_NAV_RIGHT_WRAP
	LD	A, (TRACK_SELECTED)
	INC	A
	LD	HL, TRACK_COUNT
	CP	(HL)
	JR	NC, UI_NAV_RIGHT_WRAP	; past last entry -> wrap
	RET
UI_NAV_RIGHT_WRAP:
	LD	A, (TRACK_SELECTED)
	SUB	B			; row start
	RET

; Wait up to B polls for a console byte. Returns char in A, 0 if timeout.
; Uses BDOS CONST (11) so we never block inside UI_ESC_SEQ.
UI_ESC_GETCH:
	PUSH	BC
UI_ESC_GETCH1:
	PUSH	BC
	LD	C, $0B
	CALL	BDOS
	OR	A
	JR	Z, UI_ESC_GETCH2
	LD	C, $06
	LD	E, $FF
	CALL	BDOS
	POP	BC
	POP	BC
	AND	$7F
	RET
UI_ESC_GETCH2:
	POP	BC
	DJNZ	UI_ESC_GETCH1
	POP	BC
	XOR	A
	RET

; Called after Esc in UI mode: give an escape sequence a short window to
; arrive (arrow keys = ESC [ A/B/C/D or ESC O A/B/C/D; keypad Enter = ESC O M).
; Returns A=0 bare Esc (quit), A=1 sequence consumed, A=2 delete, A=3 Enter.
; On an arrow key, UI_IDXTMP holds the new index (<> $FF).
UI_ESC_SEQ:
	LD	A, $FF
	LD	(UI_IDXTMP), A
	LD	B, $40			; spin count waiting for the introducer
	CALL	UI_ESC_GETCH
	OR	A
	JR	Z, UI_ESC_SEQ0		; bare Esc
	CP	'['
	JR	Z, UI_ESC_SEQ_CSI
	CP	'O'
	JR	Z, UI_ESC_SEQ_SS3
	LD	A, 1			; unknown sequence — swallow
	RET
UI_ESC_SEQ0:
	XOR	A
	RET

UI_ESC_SEQ_CSI:
	LD	B, $40
	CALL	UI_ESC_GETCH
	OR	A
	JR	Z, UI_ESC_SEQ1		; incomplete — swallow
	CP	'3'
	JR	Z, UI_ESC_DEL_TILDE
	CP	'A'
	JR	Z, UI_ESC_UP
	CP	'B'
	JR	Z, UI_ESC_DOWN
	CP	'C'
	JR	Z, UI_ESC_RIGHT
	CP	'D'
	JR	Z, UI_ESC_LEFT
	LD	A, 1
	RET

UI_ESC_SEQ_SS3:
	LD	B, $40
	CALL	UI_ESC_GETCH
	OR	A
	JR	Z, UI_ESC_SEQ1
	CP	'M'
	JR	Z, UI_ESC_ENTER		; keypad Enter
	CP	'A'
	JR	Z, UI_ESC_UP
	CP	'B'
	JR	Z, UI_ESC_DOWN
	CP	'C'
	JR	Z, UI_ESC_RIGHT
	CP	'D'
	JR	Z, UI_ESC_LEFT
	LD	A, 1
	RET

UI_ESC_SEQ1:
	LD	A, 1
	RET

UI_ESC_ENTER:
	LD	A, 3
	RET
UI_ESC_DEL_TILDE:
	; ESC [ 3 ~ = DEL key: consume the trailing '~', report A=2
	LD	B, $40
	CALL	UI_ESC_GETCH
	OR	A
	JR	Z, UI_ESC_SEQ1
	LD	A, 2
	RET
UI_ESC_UP:
	CALL	UI_NAV_UP
	JR	UI_ESC_NAV_DONE
UI_ESC_DOWN:
	CALL	UI_NAV_DOWN
	JR	UI_ESC_NAV_DONE
UI_ESC_RIGHT:
	CALL	UI_NAV_RIGHT
	JR	UI_ESC_NAV_DONE
UI_ESC_LEFT:
	CALL	UI_NAV_LEFT
UI_ESC_NAV_DONE:
	LD	(UI_IDXTMP), A
	LD	A, 1
	RET

;-------------------------------------------------------------------------------
; Strings
;-------------------------------------------------------------------------------

MSG_PLKEYS:
	.DB	"Keys: Esc=quit, Space=pause, Enter=play, N/P=track, WASD=nav, (R)edraw, (l/L)oop trk/plist", 0
MSG_LOOPSTAT:
	.DB	"Looping Status: ", 0
MSG_LOOPSTAT_OFF:
	.DB	"Off", 0
MSG_LOOPSTAT_TRK:
	.DB	"Track", 0
MSG_LOOPSTAT_PL:
	.DB	"Playlist", 0
MSG_SONGNAME:
	.DB	"Song name:  ", 0
MSG_ARTIST:
	.DB	"by:         ", 0

UI_SEQ_CLR:
	.DB	27, '[', '2', 'J', 27, '[', 'H', 0
UI_SEQ_EL2:
	.DB	27, '[', '2', 'K', 0
UI_SEQ_EL:
	.DB	27, '[', 'K', 0
UI_SEQ_CYAN:
	.DB	27, '[', '3', '6', 'm', 0
UI_SEQ_YEL:
	.DB	27, '[', '3', '3', 'm', 0
UI_SEQ_GRN:
	.DB	27, '[', '3', '2', 'm', 0
UI_SEQ_RST:
	.DB	27, '[', '0', 'm', 0
