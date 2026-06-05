# Compare COM bytes against TASM lst emitted data lines (addr + hex bytes)
$root = Split-Path $PSScriptRoot -Parent
$com = [IO.File]::ReadAllBytes((Join-Path $root "vtune.com"))
$lst = Get-Content (Join-Path $root "vtune.lst")
$base = 0x100
$emitted = @{}
foreach ($line in $lst) {
    if ($line -match '^\d+\s+([0-9A-F]{4})\s+((?:[0-9A-F]{2}\s+)+)') {
        $addr = [Convert]::ToInt32($matches[1], 16)
        $bytes = $matches[2].Trim().Split(' ', [StringSplitOptions]::RemoveEmptyEntries)
        for ($i = 0; $i -lt $bytes.Length; $i++) {
            $emitted[$addr + $i] = [Convert]::ToByte($bytes[$i], 16)
        }
    }
}
$mism = 0
foreach ($addr in ($emitted.Keys | Sort-Object)) {
    $off = $addr - $base
    if ($off -lt 0 -or $off -ge $com.Length) { continue }
    $c = $com[$off]
    $e = $emitted[$addr]
    if ($c -ne $e) {
        Write-Output ("MISMATCH @{0:X4}: com={1:X2} lst={2:X2}" -f $addr, $c, $e)
        $mism++
        if ($mism -ge 30) { break }
    }
}
Write-Output "Mismatches in loaded image: $mism (checked $($emitted.Count) lst bytes)"
Write-Output "COM last addr 0x$('{0:X4}' -f ($base + $com.Length - 1)) lst max 0x$('{0:X4}' -f ($emitted.Keys | Measure-Object -Maximum).Maximum)"
