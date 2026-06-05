param([string]$Path = "Tunes\rl2wof.pt3")
$bytes = [IO.File]::ReadAllBytes($Path)
$patTab = $bytes[103] + ($bytes[104] -shl 8)
$pat0 = $bytes[$patTab] + ($bytes[$patTab + 1] -shl 8)
$base = $pat0
Write-Output "File: $Path"
Write-Output "patTabOff=$patTab pat0Base=$pat0 speed=$($bytes[100]) pos0=$($bytes[192])"
foreach ($off in 0, 2, 4) {
    $rel = $bytes[$base + $off] + ($bytes[$base + $off + 1] -shl 8)
    $abs = $base + $rel
    $b = $bytes[$abs]
    Write-Output "  ch offset+$off rel=$rel abs=$abs firstByte=0x$($b.ToString('X2'))"
}
$idx = $bytes[192] * 2
foreach ($off in 0, 2, 4) {
    $abs = $bytes[$patTab + $idx + $off] + ($bytes[$patTab + $idx + $off + 1] -shl 8)
    Write-Output "tune bind ch+$off stream@$abs first=0x$($bytes[$abs].ToString('X2'))"
}
Write-Output "pos@192=$($bytes[192]) pos@200=$($bytes[200])"
Write-Output "tune-style patsPtr=$($bytes[103] + ($bytes[104] -shl 8) + 100)"
$abs = 167
$hex = ($bytes[$abs..($abs + 15)] | ForEach-Object { $_.ToString('X2') }) -join ' '
Write-Output "stream@167: $hex"
