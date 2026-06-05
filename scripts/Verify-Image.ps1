$root = Split-Path $PSScriptRoot -Parent
$b = [IO.File]::ReadAllBytes((Join-Path $root "vtune.com"))
$base = 0x100
# (lstAddr, expectedFirstByteHex, label)
$checks = @(
    @(0x032F, '3A', 'HEAPENDB_INIT: LD A,(7)'),
    @(0x17E8, 'AF', 'LOAD_MUSIC_FILE: XOR A'),
    @(0x1916, 'F5', 'PRTSTR: PUSH AF'),
    @(0x1951, 'F5', 'CRLF: PUSH AF')
)
foreach ($c in $checks) {
    $addr = [int]$c[0]
    $off = $addr - $base
    if ($off -ge 0 -and $off -lt $b.Length) {
        $actual = $b[$off].ToString('X2')
        $ok = if ($actual -eq $c[1]) { 'OK' } else { 'SHIFTED' }
        Write-Output ("{0:X4} off={1} expect={2} actual={3} {4}  ({5})" -f $addr, $off, $c[1], $actual, $ok, $c[2])
    } else {
        Write-Output ("{0:X4} off={1} BEYOND FILE END ({2})" -f $addr, $off, $c[2])
    }
}
