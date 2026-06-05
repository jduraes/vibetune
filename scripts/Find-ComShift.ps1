$root = Split-Path $PSScriptRoot -Parent
$com = [IO.File]::ReadAllBytes((Join-Path $root "vtune.com"))
$lst = Get-Content (Join-Path $root "vtune.lst")
$base = 0x100
$emitted = [ordered]@{}
foreach ($line in $lst) {
    if ($line -match '^\d+\s+([0-9A-F]{4})\s+((?:[0-9A-F]{2}\s+)+)') {
        if ($line -match '^\d+\+\s') { continue }
        $addr = [Convert]::ToInt32($matches[1], 16)
        $bytes = $matches[2].Trim().Split(' ', [StringSplitOptions]::RemoveEmptyEntries)
        for ($i = 0; $i -lt $bytes.Length; $i++) {
            $emitted[$addr + $i] = [Convert]::ToByte($bytes[$i], 16)
        }
    }
}
$addrs = @($emitted.Keys)
$maxAddr = ($addrs | Measure-Object -Maximum).Maximum
Write-Output "Lst emitted bytes: $($addrs.Count) maxAddr=0x$('{0:X4}' -f $maxAddr)"
for ($shift = -4..4) {
    $m = 0
    $checked = 0
    foreach ($addr in $addrs) {
        $off = $addr - $base + $shift
        if ($off -lt 0 -or $off -ge $com.Length) { continue }
        $checked++
        if ($com[$off] -ne $emitted[$addr]) { $m++ }
    }
    Write-Output "shift=$shift mismatches=$m checked=$checked"
}
