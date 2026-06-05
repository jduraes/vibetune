$root = Split-Path $PSScriptRoot -Parent
$com = [IO.File]::ReadAllBytes((Join-Path $root "vtune.com"))
$lst = Get-Content (Join-Path $root "vtune.lst")
$base = 0x100
Write-Output "=== COM vs LST from 1E40 ==="
for ($addr = 0x1E40; $addr -le 0x1E60; $addr++) {
    $o = $addr - $base
    $cb = if ($o -ge 0 -and $o -lt $com.Length) { $com[$o].ToString('X2') } else { "--" }
    $lb = "--"
    foreach ($line in $lst) {
        if ($line -match "^\d+\s+$($addr.ToString('X4'))\s+([0-9A-F]{2})") {
            $lb = $matches[1]
            break
        }
        if ($line -match "^\d+\s+$($addr.ToString('X4'))\s+([0-9A-F]{2}\s+[0-9A-F]{2})") {
            $parts = $matches[1] -split '\s+'
            $idx = $addr - [Convert]::ToInt32($line.Split()[1], 16)
            if ($idx -ge 0 -and $idx -lt $parts.Length) { $lb = $parts[$idx] }
            break
        }
    }
    # simpler: grep lst for exact address with byte
    $grep = $lst | Where-Object { $_ -match "^\d+\s+$($addr.ToString('X4'))\s+([0-9A-F]{2}\s+)+[^;]*$" -or $_ -match "^\d+\s+$($addr.ToString('X4'))\s+([0-9A-F]{2})\s" }
    if ($grep) {
        if ($grep -match "^\d+\s+[0-9A-F]{4}\s+((?:[0-9A-F]{2}\s*)+)") {
            $bytes = $matches[1].Trim().Split(' ', [StringSplitOptions]::RemoveEmptyEntries)
            $lineAddr = [Convert]::ToInt32(($grep -split '\s+')[1], 16)
            $idx = $addr - $lineAddr
            if ($idx -ge 0 -and $idx -lt $bytes.Length) { $lb = $bytes[$idx] }
        }
    }
    $mark = if ($cb -eq $lb) { "OK" } else { "BAD" }
    Write-Output ("{0:X4} com={1} lst~={2} {3}" -f $addr, $cb, $lb, $mark)
}
