# SC126 forced `-delay` fix (v0.0.152–154)

Hardware-verified on RCZ180 EB (SC126 class, platform id 10): `vtune rl2wof -delay` plays at **94 BPM** with clean channels. RC2014/RCZ80 native delay unchanged.

## Problem

On Z180 boards with a live HBIOS timer, forcing `-delay` caused:

| Symptom | Observed BPM / behaviour |
|---------|---------------------------|
| Tempo too fast | 114 BPM (target 94) |
| White-noise / fuzz | v0.0.151 baseline |
| Out-of-phase blips | After v0.0.152 partial fix |

Timer mode (no `-delay`) and RC2014 auto-fallback delay were already correct.

## Root causes

1. **Tempo:** `QDLY0 = CPUKHZ/2` scales with CPU speed, but the engine bias (`-185`) is an absolute constant. On ~18.432 MHz Z180 the frame was ~21% too short.
2. **IRQ jitter:** SC126 has a live HBIOS timer interrupt. The delay-mode busy-wait and Bulba quark computation ran with interrupts enabled, unlike RC2014 (no timer IRQ in delay mode).

## Solution (gated on `FORCED_DLY_HWTIMER`)

Set in `PROBETIMER` when `DELAYMD≠0` **and** HBIOS `$D0` tick is nonzero. Always **0** on RC2014.

| Build | Change |
|-------|--------|
| v0.0.152 | `SC126_DLY_SCALE` — QDLY × 155/128 for single-chip PTx delay |
| v0.0.153 | `FORCED_DLY_FRAME_BEGIN`/`END` — DI through play + WAITQ; ROUT defers EI |
| v0.0.154 | Exit message `Done...`; full hardware sign-off |

### Key symbols (`timing.inc`, `vibetune.asm`, `pt3bulba.inc`)

- `FORCED_DLY_HWTIMER` — discriminator flag
- `SC126_DLY_SCALE` — tempo correction
- `FORCED_DLY_FRAME_BEGIN` / `FORCED_DLY_FRAME_END` — full-quark IRQ shield
- ROUT skips `EI` when flag set (frame end restores interrupts)

## Calibration (SC126, `rl2wof -delay`)

| Field | Value (hex) |
|-------|-------------|
| CPU kHz | 4800 |
| QDLY0 | 2400 |
| After bias | 2347 |
| Final QDLY | 2AA9 |

## Files

- `timing.inc` — timing pipeline, scale, frame shield
- `vibetune.asm` — `MAIN_LOOP` integration, `FORCED_DLY_HWTIMER` data
- `pt3bulba.inc` — ROUT EI deferral
