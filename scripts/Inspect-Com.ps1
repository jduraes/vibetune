param([string]$Path = "vtune.com")
$b = [IO.File]::ReadAllBytes((Join-Path (Split-Path $PSScriptRoot -Parent) $Path))
Write-Output "Size=$($b.Length) (0x$($b.Length.ToString('X')))"
Write-Output "Byte0 (load@100h)=0x$($b[0].ToString('X2')) expect AF for XOR A"
Write-Output "Bytes0-7: $(($b[0..7] | ForEach-Object { $_.ToString('X2') }) -join ' ')"
Write-Output "End-8: $(($b[($b.Length-8)..($b.Length-1)] | ForEach-Object { $_.ToString('X2') }) -join ' ')"
