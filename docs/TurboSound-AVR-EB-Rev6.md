# TurboSound AVR-AY on the Ed Brindley YM2149 Card (Rev6)

This document explains how the **TurboSound dual-AY emulator (two ATmega8 chips)** behaves when installed on an **Ed Brindley YM2149 Sound Card Rev6** for RC2014, which performs its own address decoding using **A0/A1**.

The key point:  
**The card's A0/A1 decoding does not conflict with TurboSound's dual-chip selection mechanism.**

---

## 1. What the Ed Brindley YM2149 Card Does

The rev6 card contains glue logic that:

- Reads Z80 address lines (including **A0/A1**)  
- Decodes the I/O port being accessed  
- Generates the standard AY/YM control signals:
  - **BDIR** (Bus Direction)
  - **BC1** (Control)
- Presents the Z80 **data bus (D0–D7)** to the sound chip

From the perspective of the sound chip, the card provides a **normal AY/YM interface**:

- When BC1/BDIR indicate "Latch Register", the chip receives a register number.
- When BC1/BDIR indicate "Write Data", the chip receives data for that register.

The YM2149 itself does **not** see A0/A1 directly; it only sees the decoded control signals.

---

## 2. What the TurboSound AVR-AY Emulator Expects

The TurboSound emulator (two ATmega8 chips) behaves like **two AY-3-8910/YM2149 chips wired in parallel**.  
It expects:

- **BDIR**
- **BC1**
- **D0–D7**
- Clock
- Reset
- Audio output pins

TurboSound selects which "virtual AY chip" to use based on **register numbers written over the data bus**:

- Writing register **0xFF** → Chip 1  
- Writing register **0xFE** → Chip 2  

This selection happens **inside the firmware**, not via address lines.

---

## 3. Why A0/A1 Addressing Does Not Interfere

The Ed Brindley card's A0/A1 logic only determines **which I/O port** the Z80 is accessing.  
Once the port is decoded, the card asserts BC1/BDIR appropriately.

TurboSound's dual-chip selection is based on **data written to the register-select port**, not on A0/A1.

Therefore:

- The card continues to generate BC1/BDIR exactly as it does for a real YM2149.
- The TurboSound emulator sees a **standard AY bus**.
- Software can select AY1 or AY2 by writing **0xFF or 0xFE** as the register index.
- The card's A0/A1 decoding is completely orthogonal to TurboSound's internal logic.

In other words:

> **If the emulator is wired pin-compatible with the YM2149 socket, it will work.  
> The card's addressing scheme does not block TurboSound functionality.**

---

## 4. Practical Considerations

### Physical Compatibility
The TurboSound design is meant as a **drop-in AY replacement**.  
As long as the board matches the YM2149 pinout, the rev6 card will treat it as a normal PSG.

### Audio Mixing
TurboSound mixes the outputs of both emulated AY chips **on the emulator board** using resistors and an RC filter.  
The Ed Brindley card simply receives the final mixed audio.

### ZX-Spectrum-specific Notes
Some TurboSound write-ups mention A14/A15 wiring for ZX-Spectrum machines.  
These do **not** apply to RC2014, because the rev6 card already performs its own local address decoding.

---

## 5. Summary

- The Ed Brindley rev6 card handles **address decoding** and produces **BDIR/BC1**.  
- TurboSound only needs **BDIR/BC1 + data bus**, just like a real AY/YM.  
- TurboSound's dual-chip selection uses **register numbers**, not address lines.  
- Therefore, **TurboSound works correctly on the rev6 card**, provided the emulator is wired as a YM2149 replacement.
