$b = [IO.File]::ReadAllBytes((Join-Path (Split-Path $PSScriptRoot -Parent) "vtune.com"))
$base = 0x100
foreach ($addr in @(0x0100, 0x0109, 0x0124, 0x0127, 0x192B, 0x1938)) {
    $o = $addr - $base
    $n = [Math]::Min(6, $b.Length - $o)
    $hex = ($b[$o..($o + $n - 1)] | ForEach-Object { $_.ToString('X2') }) -join ' '
    Write-Output ('{0:X4}: {1}' -f $addr, $hex)
}
