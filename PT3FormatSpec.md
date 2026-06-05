31-May-2026

# PT3 Format — Action Module (clean-room reference)

Documented from module behavior and corpus inspection (`rl2wof.pt3`, `rl2wofts.pt3`). Use for VibeTune decode implementation. Do not treat RomWBW player source as code to include.

## 1. Module identity

1.1 ProTracker family header begins with printable `ProTracker ` prefix (validated in `VALIDATE_PTX_STRUCTURE`).
1.2 Version digit(s) follow in header (e.g. `3.7` in rl2wof).
1.3 Module is loaded contiguously into `MUSIC_BUF`; all offsets below are from module base.

## 2. Header fields used at init

2.1 Offset **100** — speed / delay byte (ticks per pattern row; rl2wof = 4).
2.2 Offset **102** — loop position index (future loop semantics).
2.3 Offset **99** — tone table selector (used when building note period table; rl2wof = 2).
2.4 Offset **192** (0xC0) — **position list**: sequence of pattern indices; `$FF` terminates.
2.5 Offset **103**–**104** — little-endian offset from module base to the **pattern address table** (rl2wof = 228, not 256).
2.6 Pattern table: one **16-bit** LE word per pattern slot (offset from module base). rl2wof table at **228**: 324, 362, 431, 447, …
2.7 **Channel bind (tune.com / Bulba):** position byte **P** → index **P×2** into pattern table; **three consecutive LE words** are stream pointers for A, B, C (module-relative). rl2wof position 0: **68**, **106**, **175** — not offsets inside one pattern header.

## 3. Pattern layout

3.1 Each pattern contains **64 rows**.
3.2 Each row is **6 bytes** encoding channels A, B, C with variable-length event bytes (not three fixed words).
3.3 Row events use a compact command space (note, volume, ornament, sample, envelope, special); full decode is Phase C in `pt3engine.inc`.

## 4. Timing model

4.1 Host main loop calls `PT3_ENGINE_STEP` once per frame delay (~50 Hz target).
4.2 Each step decrements quark counter; at zero, `PT3_ENGINE_QUARK` runs one decode/output cycle.
4.3 Quark counter reloads from speed byte at offset 100 (minimum sensible default 6 if zero).

## 5. Output model

5.1 Decode maintains per-channel state and a 14-byte AY shadow register block.
5.2 `PT3_FLUSH_AY` writes shadow registers through `PSG_WRITE_ROUTED` (respects TurboSound routing from M8).
5.3 Output is AY-3-8910 family via runtime-configured ports (HBIOS query or probe). Register semantics: reg 7 bit=1 disables tone/noise; reg 8–10 level nibble 15 ($0F) is minimum output. Reference: RomWBW `Source/HBIOS/ay38910.asm`. Do not write period registers to 0 for mute (highest pitch).

## 6. Implementation phases (VibeTune)

6.1 **Phase A (done)** — quark counter, module bind, speed byte.
6.2 **Phase B (done)** — position/pattern pointers, channel BSS, note period table, AY flush from channel state.
6.3 **Phase C (v0.0.42)** — clean-room pattern-line decoder (note/volume/position advance); ornament/slide/env stubs.
6.4 **Phase D (next)** — hardware audible acceptance: `rl2wof.pt3` (AY) and `rl2wofts.pt3` (TurboSound).

## 7. Acceptance tests

7.1 Startup already validated (M8/M9).
7.2 Decode smoke pending until Phase C–D complete.
