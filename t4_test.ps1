# чтобы в дробных числах была точка, не запятая 1 2 abc
[System.Threading.Thread]::CurrentThread.CurrentCulture = [System.Globalization.CultureInfo]::InvariantCulture
$r = [double]$args[0]

# массив точек
$points = @()

# считывание точек
foreach($line in $input){
$line = $line.Trim()
if ($line -eq ""){ continue }

$parts = $line.Split(' ', [StringSplitOptions]::RemoveEmptyEntries)

$points += [PSCustomObject]@{x = [double]$parts[0]; y = [double]$parts[1]; z = [double]$parts[2]}
}

foreach($p in $points){
$distance = [Math]::Sqrt($p.x*$p.x + $p.y*$p.y + $p.z*$p.z)
if ($distance -lt $r){
"{0:F3} {1:F3} {2:F3}" -f $p.x, $p.y, $p.z
}
}






































