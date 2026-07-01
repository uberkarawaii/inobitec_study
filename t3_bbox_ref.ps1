# чтобы в дробных числах была точка, не запятая
[System.Threading.Thread]::CurrentThread.CurrentCulture = [System.Globalization.CultureInfo]::InvariantCulture
# массив точек
$points = @()
# центроид
$center_x = 0
$center_y = 0
$center_z = 0
	
foreach($line in $input){
# обрезка пробелов по краям. пустые строки пропускаются
$line = $line.Trim()
if ($line -eq ""){ continue }
# разбить по пробелам, сложить в переменные
$parts = $line.Split(' ', [StringSplitOptions]::RemoveEmptyEntries)
$x = [double]$parts[0]
$y = [double]$parts[1]
$z = [double]$parts[2]
# в итоговый массив точек
$points += [PSCustomObject]@{x = $x; y = $y; z = $z}
# сумма в центроиде
$center_x += $x
$center_y += $y
$center_z += $z
}

# деление на кол-во для центроида
$center_x /= $points.Count
$center_y /= $points.Count
$center_z /= $points.Count

# максимум и минимум, границы паралеллепипеда
$min_x = $points[0].x; $min_y = $points[0].y; $min_z = $points[0].z
$max_x = $points[0].x; $max_y = $points[0].y; $max_z = $points[0].z
# расстояние до центроида
$dist = 0
# проход для поиска границ и вычисления расст. до центроида
foreach($p in $points){
if ($p.x -lt $min_x){ $min_x = $p.x } 
if ($p.y -lt $min_y){ $min_y = $p.y }
if ($p.z -lt $min_z){ $min_z = $p.z }

if ($p.x -gt $max_x){ $max_x = $p.x }
if ($p.y -gt $max_y){ $max_y = $p.y }
if ($p.z -gt $max_z){ $max_z = $p.z }

$dist += [Math]::Sqrt([Math]::Pow(($p.x - $center_x), 2) + [Math]::Pow(($p.y - $center_y), 2) + [Math]::Pow(($p.z - $center_z), 2))
}

# деление на кол-во точек для среднего расстояния
$dist /= $points.Count

# вывод к-ва точек
"Количество точек: {0}" -f $points.Count
# вывод границ паралеллепипеда
"Координаты огранич. паралеллепипеда x: [ {0:F3}; {1:F3} ]  y: [ {2:F3}; {3:F3} ]  z: [ {4:F3}; {5:F3} ]" -f $min_x, $max_x, $min_y, $max_y, $min_z, $max_z 

# вывод центроида
"Координаты центроида x: {0:F3}, y: {1:F3}, z: {2:F3}" -f $center_x, $center_y, $center_z

# вывод среднего расстояния до центроида
"Среднее расстояние от точек до центроида: {0:F3}" -f $dist




