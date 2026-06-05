param([string]$Path = "Tunes\rl2wof.pt3")
$b = [IO.File]::ReadAllBytes($Path)
function W([int]$o) { $b[$o] + ($b[$o + 1] -shl 8) }
$patTab = W 103
Write-Output "patTab=$patTab pos0=$($b[192])"
foreach ($o in 228, 230, 232, 324, 326, 328, 362, 364, 431, 433) {
    Write-Output "  word@$o = $(W $o)"
}
$p = $b[192]
$idx = $p * 2
Write-Output "table[P] at $($patTab + $idx) = $(W ($patTab + $idx))"
Write-Output "spec triple: $(W 324) $(W 326) $(W 328)"
$base = W ($patTab + $idx)
Write-Output "pattern base=$base rels: $(W ($base)) $(W ($base+2)) $(W ($base+4))"
