::не показывать команды, прописанные ниже
@echo off
:: кодировка. и перенаправить вывод сообщение о кодировке "вникуда"
chcp 1251 > nul

:: ===== C++ (t1_dist_matrix) =====
:: 1. Формат
clang-format -i t1_dist_matrix\main.cpp t1_dist_matrix\ref.cpp common\exit_codes.hpp

:: 2. Сборка
cl /c /Fo:t1_dist_matrix\main.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address t1_dist_matrix\main.cpp
link /DEBUG /OUT:t1_dist_matrix\main.exe t1_dist_matrix\main.obj
cl /c /Fo:t1_dist_matrix\ref.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address t1_dist_matrix\ref.cpp
link /DEBUG /OUT:t1_dist_matrix\ref.exe t1_dist_matrix\ref.obj

echo C++ TESTS
:: 3. Acceptance-тесты                                             	
echo 5 | t1_dist_matrix\main.exe > t1_dist_matrix\main_out.txt
echo 5 | t1_dist_matrix\ref.exe > t1_dist_matrix\ref_out.txt
fc t1_dist_matrix\main_out.txt t1_dist_matrix\ref_out.txt > nul
if errorlevel 1 echo FAIL: norm-case & exit /b 1

:: перенаправить сообщение об ошибке "вникуда". 2 - это номер потока stderr
echo abc | t1_dist_matrix\main.exe 2> nul
if not errorlevel 65 echo FAIL: non-number & exit /b 1

type nul | t1_dist_matrix\main.exe 2> nul
if not errorlevel 66 echo FAIL: empty & exit /b 1

echo 2 | t1_dist_matrix\main.exe 2> nul
if not errorlevel 64 echo FAIL: range-low & exit /b 1

echo 21 | t1_dist_matrix\main.exe 2> nul
if not errorlevel 64 echo FAIL: range-high & exit /b 1

echo ALL TESTS PASSED

:: ===== C (t1_dist_matrix_c) =====
:: формат
clang-format -i t1_dist_matrix_c\main.c t1_dist_matrix_c\ref.c common\exit_codes.h

:: сборка
cl -c /Fo:t1_dist_matrix_c\main.obj /std:c17 /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address t1_dist_matrix_c\main.c
link /DEBUG /OUT:t1_dist_matrix_c\main.exe t1_dist_matrix_c\main.obj
cl -c /Fo:t1_dist_matrix_c\ref.obj /std:c17 /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address t1_dist_matrix_c\ref.c
link /DEBUG /OUT:t1_dist_matrix_c\ref.exe t1_dist_matrix_c\ref.obj

echo C TESTS
::acceptance тесты
echo 5 | t1_dist_matrix_c\main.exe > t1_dist_matrix_c\main_out_c.txt
echo 5 | t1_dist_matrix_c\ref.exe > t1_dist_matrix_c\ref_out_c.txt
fc t1_dist_matrix_c\main_out_c.txt t1_dist_matrix_c\ref_out_c.txt > nul
if errorlevel 1 echo FAIL: norm-case & exit /b 1

type nul | t1_dist_matrix_c\main.exe 2> nul
if not errorlevel 66 echo FAIL: empty & exit /b 1

echo abc | t1_dist_matrix_c\main.exe 2> nul
if not errorlevel 65 echo FAIL: non-number & exit /b 1

echo 2.3 | t1_dist_matrix_c\main.exe 2> nul
if not errorlevel 65 echo FAIL: not-integer & exit /b 1

echo 1 | t1_dist_matrix_c\main.exe 2> nul
if not errorlevel 64 echo FAIL: range-low & exit /b 1

echo 100 | t1_dist_matrix_c\main.exe 2> nul
if not errorlevel 64 echo FAIL: range-high & exit /b 1

echo ALL TESTS PASSED   