31-May-2026

# VibeTune SIMH Testing

## Quick start

```text
cmd /c Build.cmd
cmd /c Run-SIMH-VibeTune.cmd
```

`Sim.vibetune.cfg` uses `hd1k_combo.img` on HDSK0.

At `Boot [H=Help]:` type **`2.4`** then Enter (unit 2 = HDSK0, slice 4 = ZPM3).

At `A0:SYSTEM>`:

```text
vtune
vtune rl2wof
vtune -list
vtune rl2wof -delay
```

`vtune rl2wof -delay` must show **`Timing: delay mode`** (v0.0.97+).

Quit SIMH: Esc then `q`.

## Sync

Patches **user 0** on `hd1k_zpm3.img` via `cpmcp`, then re-concats `hd1k_combo.img` (append existing slices; does not rebuild RomWBW manifests).

```text
powershell -NoProfile -ExecutionPolicy Bypass -File .\Sync-SIMH-Content.ps1 -Force
```

Base image missing:

```text
powershell -NoProfile -ExecutionPolicy Bypass -File .\Sync-SIMH-Content.ps1 -Rebuild
```

Close SIMH before sync.

## Scripted console (telnet)

Piped stdin into SIMH on Windows does not reach the guest console. Reliable
alternative: run SIMH with a telnet console and drive it over TCP.

- Config: `..\RomWBW\Tools\simh\Sim.vtunecfg-test.cfg` (copy of
  `Sim.vibetune.cfg` + `set console telnet=2323`).
- Driver: `scripts/Drive-SIMH.py` — waits for `Boot [H=Help]:`, sends `2.4`,
  waits for `A0:SYSTEM`, then sends each argument as a command. Suffix a
  command with `::N` to extend its output drain to N seconds
  (e.g. `"vtune rl2wof::12"`). Send a lone escape as `$'\x1b'`.

```bash
cd ../RomWBW/Tools/simh
./altairz80.exe Sim.vtunecfg-test.cfg ../../Binary/SBC_simh_std.rom &
sleep 3
python scripts/Drive-SIMH.py "vtunecfg show" "vtunecfg ansi" "vtunecfg verify"
taskkill //IM altairz80.exe //F
```

### Attach variant (preferred for iterative testing)

`scripts/Drive-SIMH-Attach.py` connects to an **already-running** ZPM3 session
(boots 2.4 only if it sees the boot prompt) and leaves SIMH running, so many
test batches can share one emulator instance. Same argument syntax, plus
`RAW:keys` for raw keystrokes without trailing CR (backslash escapes decoded,
e.g. `"RAW:\x1b[C::4"` for Right-arrow):

```bash
python scripts/Drive-SIMH-Attach.py "vtune demo3::75" "RAW:\x1b::3"
```

Gotchas:

- Kill SIMH (`taskkill //IM altairz80.exe //F`) before any image sync — it
  locks `hd1k_combo.img`. Wait ~1s before `cpmcp`.
- A crashed driver run can kill the SIMH telnet console; taskkill + relaunch.
- ZPM3 occasionally swallows a command after heavy output ("G ?" artifact);
  just re-run it.
- `VTUNE.CFG` written by `vtunecfg` inside SIMH does **not** survive the next
  `BuildDsk.ps1 hd1k_combo` (the combo image is regenerated from
  `hd1k_zpm3.img`). Recreate it (`vtunecfg` → A → Y → Esc) after each sync.

Note: `BuildImg.ps1 hd1k_zpm3` rebuilds from staging dirs and does **not**
pick up freshly built `vtune.com` / `vtunecfg.com`. Patch the image directly
from `..\RomWBW\Tools\cpmtools` (finds `diskdefs` in cwd), then re-concat:

```bash
cd ../RomWBW/Tools/cpmtools
./cpmrm -f wbw_hd1k ../../Binary/hd1k_zpm3.img 0:vtunecfg.com
./cpmcp -f wbw_hd1k ../../Binary/hd1k_zpm3.img ../../../VibeTune/vtunecfg.com 0:
cd ../Source/Images && powershell -NoProfile -ExecutionPolicy Unrestricted ./BuildDsk.ps1 hd1k_combo
```

## Host gate

```text
pwsh -NoProfile -File .\Validate-StructureChecks.ps1
```
