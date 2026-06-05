$b = [IO.File]::ReadAllBytes((Join-Path (Split-Path $PSScriptRoot -Parent) "vtune.com"))
$base = 0x100
$o = 0x1938 - $base
$ascii = -join ($b[($o - 2)..($o + 40)] | ForEach-Object { if ($_ -ge 32 -and $_ -le 126) { [char]$_ } else { '.' } })
Write-Output "Around MSG_BANNER: $ascii"
Write-Output "Hex: $(($b[($o-2)..($o+10)] | ForEach-Object { $_.ToString('X2') }) -join ' ')"
