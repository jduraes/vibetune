;
; Quark timing — same model as tune.com / timing.inc (20 ms target per quark).
; WAITQ uses HBIOS timer when available, else CPU-calibrated delay loop (40 T-states/iter).
;

PROBETIMER:
	LD	A, (DELAYMD)
	OR	A
	JR	Z, PROBETIMER_PROBE
	XOR	A
	LD	(WMOD), A
	RET
PROBETIMER_PROBE:
	XOR	A
	LD	(WMOD), A
	LD	B, BF_SYSGET
	LD	C, $D0
	RST	08
	LD	A, L
	OR	H
	OR	E
	OR	D
	RET	Z			; tick count zero => no live timer, stay in delay mode
	LD	A, $FF			; tick count nonzero => timer running, use timer mode
	LD	(WMOD), A
	RET

INIT_QDLY_BASE:
	LD	B, BF_SYSGET
	LD	C, $F0
	RST	08
	SRL	D
	RR	E
	EX	DE, HL
	LD	(QDLY), HL
	RET

APPLY_ENGINE_QDLY_ADJ:
	LD	HL, (QDLY)
	LD	DE, 185
	LD	A, (FILE_ENGINE)
	CP	ENGINE_MYM
	JR	NZ, APPLY_QDLY_SUB
	LD	DE, 125
APPLY_QDLY_SUB:
	OR	A
	SBC	HL, DE
	LD	(QDLY), HL
	RET

; Print ", timer mode" or ", delay mode" (tune.com TSTTIMER style).
PRINT_TIMING_MODE:
	LD	A, (WMOD)
	OR	A
	LD	DE, MSG_TIMING_DLY
	JR	Z, PRINT_TIMING_MODE_OUT
	LD	DE, MSG_TIMING_TIM
PRINT_TIMING_MODE_OUT:
	JP	PRTSTR

WAITQ:
	LD	A, (WMOD)
	OR	A
	JR	Z, WAITQ_DLY
WAITQ_TIM:
	CALL	WAITQ_TIM2
	LD	C, A
WAITQ_TIM1:
	PUSH	BC
	CALL	WAITQ_TIM2
	POP	BC
	CP	C
	RET	NZ
	JR	WAITQ_TIM1

WAITQ_TIM2:
	LD	B, BF_SYSGET
	LD	C, $D0
	RST	08
	LD	A, L
	RET

WAITQ_DLY:
	LD	BC, (QDLY)
WAITQ_DLY1:
	DEC	BC
	NOP
	NOP
	NOP
	NOP
	LD	A, B
	OR	C
	JP	NZ, WAITQ_DLY1
	RET
