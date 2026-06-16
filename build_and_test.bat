@echo off
chcp 1251 > nul

:: 1. ‘ормат
clang-format -i t1_dist_matrix\main.cpp t1_dist_matrix\ref.cpp common\exit_codes.hpp

:: 2. —борка
cl /c /Fo:t1_dist_matrix\main.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address t1_dist_matrix\main.cpp
link /DEBUG /OUT:t1_dist_matrix\main.exe t1_dist_matrix\main.obj
cl /c /Fo:t1_dist_matrix\ref.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address t1_dist_matrix\ref.cpp
link /DEBUG /OUT:t1_dist_matrix\ref.exe t1_dist_matrix\ref.obj

:: 3. Acceptance-тесты
echo 5 | t1_dist_matrix\main.exe > main_out.txt
echo 5 | t1_dist_matrix\ref.exe > ref_out.txt
fc main_out.txt ref_out.txt > nul
if errorlevel 1 echo FAIL: norm-case & exit /b 1

echo abc | t1_dist_matrix\main.exe 2> nul
if not errorlevel 65 echo FAIL: non-number & exit /b 1

echo; | t1_dist_matrix\main.exe 2> nul
if not errorlevel 66 echo FAIL: empty & exit /b 1

echo 2 | t1_dist_matrix\main.exe 2> nul
if not errorlevel 64 echo FAIL: range-low & exit /b 1

echo 21 | t1_dist_matrix\main.exe 2> nul
if not errorlevel 64 echo FAIL: range-high & exit /b 1

echo ALL TESTS PASSED