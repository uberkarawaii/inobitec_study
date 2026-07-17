::не показывать команды, прописанные ниже
@echo off
:: кодировка. и перенаправить вывод сообщение о кодировке "вникуда"
chcp 1251 > nul

:: ===== C++ (t1_dist_matrix) =====
:: 1. ‘ормат
clang-format -i t1_dist_matrix\main.cpp common\exit_codes.hpp
:: clang-format -i t1_dist_matrix\ref.cpp

:: 2. —борка
cl /c /Fo:t1_dist_matrix\main.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address t1_dist_matrix\main.cpp
if errorlevel 1 echo FAIL: main_cpp_compilation_error & exit /b 1
link /DEBUG /OUT:t1_dist_matrix\main.exe t1_dist_matrix\main.obj
if errorlevel 1 echo FAIL: main_cpp_link_error & exit /b 1
:: cl /c /Fo:t1_dist_matrix\ref.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address t1_dist_matrix\ref.cpp
:: if errorlevel 1 echo FAIL: ref_cpp_compilation_error & exit /b 1
:: link /DEBUG /OUT:t1_dist_matrix\ref.exe t1_dist_matrix\ref.obj
:: if errorlevel 1 echo FAIL: ref_cpp_link_error & exit /b 1

:: папка build если еЄ ещЄ не было
if not exist build mkdir build

echo C++ TESTS
:: 3. Acceptance-тесты                                             	
echo 3 | t1_dist_matrix\main.exe > build\main_out.txt
if errorlevel 1 echo FAIL: norm-case main exit code & exit /b 1
findstr /C:"   0.000   1.732   1.732" build\main_out.txt > nul
if errorlevel 1 echo FAIL: norm-case & exit /b 1

:: перенаправить сообщение об ошибке "вникуда". 2 - это номер потока stderr
echo abc | t1_dist_matrix\main.exe 2> nul
if not errorlevel 65 echo FAIL: non-number & exit /b 1

type nul | t1_dist_matrix\main.exe 2> nul
if not errorlevel 66 echo FAIL: empty & exit /b 1

echo 2.3 | t1_dist_matrix\main.exe 2> nul
if not errorlevel 65 echo FAIL: not-integer & exit /b 1

echo 2 | t1_dist_matrix\main.exe 2> nul
if not errorlevel 64 echo FAIL: range-low & exit /b 1

echo 21 | t1_dist_matrix\main.exe 2> nul
if not errorlevel 64 echo FAIL: range-high & exit /b 1

echo ALL TESTS PASSED

:: ===== C (t1_dist_matrix_c) =====
:: формат
clang-format -i t1_dist_matrix_c\main.c common\exit_codes.h
:: clang-format -i t1_dist_matrix_c\ref.c

:: сборка
cl -c /Fo:t1_dist_matrix_c\main.obj /std:c17 /W4 /permissive- /Od /Zi /MDd /fsanitize=address t1_dist_matrix_c\main.c
if errorlevel 1 echo FAIL: main_c_compilation_error & exit /b 1
link /DEBUG /OUT:t1_dist_matrix_c\main.exe t1_dist_matrix_c\main.obj
if errorlevel 1 echo FAIL: main_c_link_error & exit /b 1
:: cl -c /Fo:t1_dist_matrix_c\ref.obj /std:c17 /W4 /permissive- /Od /Zi /MDd /fsanitize=address t1_dist_matrix_c\ref.c
:: if errorlevel 1 echo FAIL: ref_c_compilation_error & exit /b 1
:: link /DEBUG /OUT:t1_dist_matrix_c\ref.exe t1_dist_matrix_c\ref.obj
:: if errorlevel 1 echo FAIL: ref_c_link_error & exit /b 1

echo C TESTS
::acceptance тесты
echo 3 | t1_dist_matrix_c\main.exe > build\main_out.txt
if errorlevel 1 echo FAIL: norm-case main exit code & exit /b 1
findstr /C:"   0.000   1.732   1.732" build\main_out.txt > nul
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