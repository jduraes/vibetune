param(
    [string]$TuneAsm = "..\RomWBW\Source\Apps\Tune\tune.asm",
    [string]$OutInc = "..\pt3bulba.inc"
)
$root = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$tunePath = Join-Path $root "RomWBW\Source\Apps\Tune\tune.asm"
$outPath = Join-Path $root "VibeTune\pt3bulba.inc"
$lines = Get-Content $tunePath
$hdr = @(
    "; Bulba PT2/PT3 player extracted from tune.asm (RomWBW Tune v3.16)",
    "; Module at MUSIC_BUF. Call START (init), START+5 (quark), START+8 (mute).",
    "#DEFINE MDLADDR MUSIC_BUF",
    "#DEFINE ISHBIOS LD A,(HBIOSMD) : OR A",
    "#IFDEF WBW",
    "_ZX	.SET	0",
    "_MSX	.SET	0",
    "_WBW	.SET	1",
    "#ENDIF",
    "CurPosCounter	.EQU	0",
    "ACBBAC		.EQU	0",
    "LoopChecker	.EQU	1",
    "Id		.EQU	0",
    "CPUFAMZ180	.EQU	1",
    "SBCV2004	.EQU	0",
    ""
)
$body = $lines[839..2372]
Set-Content -Path $outPath -Value ($hdr + $body) -Encoding ASCII
Write-Output "Wrote $($body.Count) lines -> $outPath ($((Get-Item $outPath).Length) bytes)"
