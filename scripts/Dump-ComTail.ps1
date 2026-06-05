$b = [IO.File]::ReadAllBytes((Join-Path (Split-Path $PSScriptRoot -Parent) "vtune.com"))
$base = 0x100
Write-Output "File size=$($b.Length) lastAddr=0x$('{0:X4}' -f ($base + $b.Length - 1))"
foreach ($a in 0x1E40..0x1E51) {
    $o = $a - $base
    if ($o -ge 0 -and $o -lt $b.Length) {
        Write-Output ('{0:X4}: {1:X2}' -f $a, $b[$o])
    }
}
# Check if addresses beyond file exist in memory map
foreach ($a in @(0x1304, 0x1E43, 0x1E51, 0x1E53, 0x2183, 0x23AE)) {
    $o = $a - $base
    if ($o -ge 0 -and $o -lt $b.Length) {
        $slice = $b[$o..([Math]::Min($o + 7, $b.Length - 1))]
        $hex = ($slice | ForEach-Object { $_.ToString('X2') }) -join ' '
        Write-Output "Addr 0x$('{0:X4}' -f $a) bytes: $hex"
    } else {
        Write-Output "Addr 0x$('{0:X4}' -f $a) offset=$o inFile=False"
    }
}
# legacy one-liners removed
foreach ($a in @()) {
    $o = $a - $base
    $in = ($o -ge 0 -and $o -lt $b.Length)
    Write-Output "Addr 0x$('{0:X4}' -f $a) offset=$o inFile=$in"
}
