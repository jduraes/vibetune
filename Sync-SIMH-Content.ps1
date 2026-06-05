param(
    [switch]$Force
)

$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$romwbwRoot = Resolve-Path (Join-Path $repoRoot '..\RomWBW')
$imagesRoot = Join-Path $romwbwRoot 'Source\Images'
$stampPath = Join-Path $repoRoot '.simh-sync.stamp'
$appsRoot = Join-Path $romwbwRoot 'Binary\Apps'
$appsTunesRoot = Join-Path $appsRoot 'Tunes'

$vtuneCom = Join-Path $repoRoot 'vtune.com'
$vtunecfgCom = Join-Path $repoRoot 'vtunecfg.com'
$tunesDir = Join-Path $repoRoot 'Tunes'

if (!(Test-Path $vtuneCom)) {
    throw "Missing required file: $vtuneCom"
}

if (!(Test-Path $tunesDir)) {
    throw "Missing Tunes directory: $tunesDir"
}

$tuneFiles = Get-ChildItem $tunesDir -File | Where-Object { $_.Extension -match '^\.(pt2|pt3|mym)$' }
if ($tuneFiles.Count -eq 0) {
    throw "No tune files found in $tunesDir"
}

$sourceFiles = @($vtuneCom) + ($tuneFiles | ForEach-Object { $_.FullName })
if (Test-Path $vtunecfgCom) {
    $sourceFiles += $vtunecfgCom
}
$latestSourceTicks = ($sourceFiles | ForEach-Object { (Get-Item $_).LastWriteTimeUtc.Ticks } | Measure-Object -Maximum).Maximum
$latestStampTicks = 0

if (Test-Path $stampPath) {
    $latestStampTicks = (Get-Item $stampPath).LastWriteTimeUtc.Ticks
}

if (-not $Force -and $latestStampTicks -ge $latestSourceTicks) {
    Write-Host "SIMH content sync is up to date."
    exit 0
}

if (!(Test-Path $appsRoot)) {
    throw "Missing RomWBW apps root: $appsRoot"
}

if (!(Test-Path $appsTunesRoot)) {
    New-Item -ItemType Directory -Path $appsTunesRoot -Force | Out-Null
}

Copy-Item -Path $vtuneCom -Destination (Join-Path $appsRoot 'vtune.com') -Force
if (Test-Path $vtunecfgCom) {
    Copy-Item -Path $vtunecfgCom -Destination (Join-Path $appsRoot 'vtunecfg.com') -Force
}

foreach ($tune in $tuneFiles) {
    Copy-Item -Path $tune.FullName -Destination (Join-Path $appsTunesRoot $tune.Name) -Force
}

Push-Location $imagesRoot
try {
    & powershell -NoProfile -ExecutionPolicy Unrestricted .\BuildImg.ps1 hd1k_cpm22
    if ($LASTEXITCODE -ne 0) { throw "BuildImg hd1k_cpm22 failed with exit code $LASTEXITCODE" }

    & powershell -NoProfile -ExecutionPolicy Unrestricted .\BuildImg.ps1 hd1k_zpm3
    if ($LASTEXITCODE -ne 0) { throw "BuildImg hd1k_zpm3 failed with exit code $LASTEXITCODE" }

    & powershell -NoProfile -ExecutionPolicy Unrestricted .\BuildDsk.ps1 hd1k_combo
    if ($LASTEXITCODE -ne 0) { throw "BuildDsk hd1k_combo failed with exit code $LASTEXITCODE" }
}
finally {
    Pop-Location
}

Set-Content -Path $stampPath -Value (Get-Date -Format 'yyyy-MM-dd HH:mm:ss') -Encoding ASCII
Write-Host "SIMH content synchronized into CP/M disk images."
