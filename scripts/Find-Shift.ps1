$root = Split-Path $PSScriptRoot -Parent
$b = [IO.File]::ReadAllBytes((Join-Path $root "vtune.com"))
$base = 0x100
# PRTSTR assembled at 0x1916, first opcode F5. Search small shift window.
$lstAddr = 0x1916
foreach ($shift in 0..8) {
    $off = $lstAddr - $base - $shift
    if ($off -ge 0 -and $off -lt $b.Length) {
        $v = $b[$off].ToString('X2')
        $hit = if ($v -eq 'F5') { '<== F5 here' } else { '' }
        Write-Output ("shift=-{0} off={1} byte={2} {3}" -f $shift, $off, $v, $hit)
    }
}
