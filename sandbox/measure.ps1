$n=40; $fail=0
for($i=1; $i -le $n; $i++){
  $t0=Get-Date
  cmake --build build/release --clean-first -j *> build\release\flake_$i.log
  $code=$LASTEXITCODE
  if($code -ne 0){ $fail++ }
  "run $i code=$code {0:N1}s" -f ((Get-Date)-$t0).TotalSeconds
}
"failures: $fail / $n"