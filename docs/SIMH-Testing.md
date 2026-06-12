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

## Host gate

```text
pwsh -NoProfile -File .\Validate-StructureChecks.ps1
```
