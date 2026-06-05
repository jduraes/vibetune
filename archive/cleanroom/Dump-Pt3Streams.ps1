param([string]$Path = "Tunes\rl2wof.pt3")
$b = [IO.File]::ReadAllBytes($Path)
function W([int]$o) { $b[$o] + ($b[$o + 1] -shl 8) }
function Hex([int]$o, [int]$n) {
    ($b[$o..($o + $n - 1)] | ForEach-Object { $_.ToString("X2") }) -join " "
}
$pat = W 228
Write-Output "=== bind model A: table[P], table[P+1], table[P+2] ==="
foreach ($o in 0, 2, 4) { $a = W (228 + $o); Write-Output "  stream@$a : $(Hex $a 12)" }
Write-Output "=== bind model B: patBase + header rels ==="
foreach ($o in 0, 2, 4) { $a = $pat + (W ($pat + $o)); Write-Output "  stream@$a : $(Hex $a 12)" }
Write-Output "=== tune three-POP (106, 175) ==="
foreach ($a in 106, 175, 174, 162, 165, 167) { Write-Output "  @$a : $(Hex $a 12)" }
