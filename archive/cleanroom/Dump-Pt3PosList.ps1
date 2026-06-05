param([string]$Path = "Tunes\rl2wof.pt3")
$b = [IO.File]::ReadAllBytes($Path)
$pos = 192
$bytes = @()
while ($b[$pos] -ne 0xFF -and $pos -lt $b.Length) {
    $bytes += $b[$pos]
    $pos++
}
Write-Output "pos list ($($bytes.Count) entries): $($bytes -join ',')"
Write-Output "next byte @${pos}: 0x$($b[$pos].ToString('X2'))"
