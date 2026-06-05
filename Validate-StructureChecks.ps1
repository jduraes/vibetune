param(
    [string]$TunesPath = "Tunes",
    [string]$InvalidPath = "TestVectors/Invalid"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Test-PrintableAscii([byte]$b) {
    return ($b -ge 32 -and $b -lt 127)
}

function Test-VibeTuneStructure {
    param(
        [byte[]]$Bytes,
        [string]$Engine
    )

    if ($Engine -eq "MYM") {
        if ($Bytes.Length -lt 2) { return $false }
        if ($Bytes[0] -ne 0x92) { return $false }
        if ($Bytes[1] -eq 0x00) { return $false }
        if ($Bytes[1] -ge 0x40) { return $false }
        return $true
    }

    if ($Engine -eq "PTX") {
        if ($Bytes.Length -lt 32) { return $false }
        for ($i = 0; $i -lt 16; $i++) {
            if (-not (Test-PrintableAscii $Bytes[$i])) { return $false }
        }

        $window = [System.Text.Encoding]::ASCII.GetString($Bytes, 0, 32)
        if ($window.StartsWith("Vortex Tracker")) { return $true }
        if ($window.StartsWith("ProTracker ")) { return $true }
        return $false
    }

    return $false
}

function Get-EngineFromExtension([string]$Name) {
    $ext = [System.IO.Path]::GetExtension($Name).ToLowerInvariant()
    switch ($ext) {
        ".pt2" { return "PTX" }
        ".pt3" { return "PTX" }
        ".mym" { return "MYM" }
        default { return "UNSUPPORTED" }
    }
}

$results = @()
$hadFailure = $false

Get-ChildItem -Path $TunesPath -File | Sort-Object Name | ForEach-Object {
    $engine = Get-EngineFromExtension $_.Name
    if ($engine -eq "UNSUPPORTED") { return }

    $bytes = [System.IO.File]::ReadAllBytes($_.FullName)
    $pass = Test-VibeTuneStructure -Bytes $bytes -Engine $engine
    if (-not $pass) { $hadFailure = $true }

    $results += [PSCustomObject]@{
        Set = "Corpus"
        File = $_.Name
        Engine = $engine
        Expected = "Pass"
        Actual = if ($pass) { "Pass" } else { "Fail" }
    }
}

Get-ChildItem -Path $InvalidPath -File | Sort-Object Name | ForEach-Object {
    $engine = Get-EngineFromExtension $_.Name
    if ($engine -eq "UNSUPPORTED") { return }

    $bytes = [System.IO.File]::ReadAllBytes($_.FullName)
    $pass = Test-VibeTuneStructure -Bytes $bytes -Engine $engine
    if ($pass) { $hadFailure = $true }

    $results += [PSCustomObject]@{
        Set = "Invalid"
        File = $_.Name
        Engine = $engine
        Expected = "Fail"
        Actual = if ($pass) { "Pass" } else { "Fail" }
    }
}

$results | Format-Table -AutoSize | Out-String | Write-Output

if ($hadFailure) {
    Write-Error "Structure validation regression detected."
}

Write-Output "Structure validation checks passed for corpus and invalid vectors."
