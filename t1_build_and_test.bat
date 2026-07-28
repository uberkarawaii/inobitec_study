::не показывать команды, прописанные ниже
@echo off
:: кодировка. и перенаправить вывод сообщение о кодировке "вникуда"
chcp 1251 > nul

:: ===== C++ (t1_dist_matrix) =====
:: 1. ‘ормат
clang-format -i t1_dist_matrix_cpp\main.cpp common\exit_codes.hpp
clang-format -i common\geometry.hpp common\geometry.cpp
:: clang-format -i t1_dist_matrix\ref.cpp

:: 2. —борка
:: compil. main
cl -c /DCOMMON_STATIC /Fo:t1_dist_matrix_cpp\main.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address t1_dist_matrix_cpp\main.cpp
if errorlevel 1 echo FAIL: main_cpp_compilation_error & exit /b 1
:: compil. geometry 
cl -c /DCOMMON_STATIC /Fo:common\geometry_cpp.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address common\geometry.cpp
if errorlevel 1 echo FAIL: geometry_cpp_compilation_error & exit /b 1

:: Link
link /DEBUG /OUT:t1_dist_matrix_cpp\main.exe t1_dist_matrix_cpp\main.obj common\geometry_cpp.obj
if errorlevel 1 echo FAIL: main_cpp_link_error & exit /b 1
:: cl /c /Fo:t1_dist_matrix_cpp\ref.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address t1_dist_matrix_cpp\ref.cpp
:: if errorlevel 1 echo FAIL: ref_cpp_compilation_error & exit /b 1
:: link /DEBUG /OUT:t1_dist_matrix_cpp\ref.exe t1_dist_matrix_cpp\ref.obj
:: if errorlevel 1 echo FAIL: ref_cpp_link_error & exit /b 1

:: папка build если еЄ ещЄ не было
if not exist build mkdir build

echo C++ TESTS
:: 3. Acceptance-тесты                                             	
echo 3 | t1_dist_matrix_cpp\main.exe > build\main_out.txt
if errorlevel 1 echo FAIL: norm-case main exit code & exit /b 1
findstr /C:"   0.000   1.732   1.732" build\main_out.txt > nul
if errorlevel 1 echo FAIL: norm-case & exit /b 1

:: перенаправить сообщение об ошибке "вникуда". 2 - это номер потока stderr
echo abc | t1_dist_matrix_cpp\main.exe 2> nul
if not errorlevel 65 echo FAIL: non-number & exit /b 1

type nul | t1_dist_matrix_cpp\main.exe 2> nul
if not errorlevel 66 echo FAIL: empty & exit /b 1

echo 2.3 | t1_dist_matrix_cpp\main.exe 2> nul
if not errorlevel 65 echo FAIL: not-integer & exit /b 1

echo 2 | t1_dist_matrix_cpp\main.exe 2> nul
if not errorlevel 64 echo FAIL: range-low & exit /b 1

echo 21 | t1_dist_matrix_cpp\main.exe 2> nul
if not errorlevel 64 echo FAIL: range-high & exit /b 1

echo ALL TESTS PASSED

:: ===== C (t1_dist_matrix_c) =====
:: формат - main, codes, geometry, string_utils
clang-format -i t1_dist_matrix_c\main.c common\exit_codes.h
clang-format -i common\geometry.c common\geometry.h
:: clang-format -i t1_dist_matrix_c\ref.c

:: сборка
:: compil. main
cl -c /DCOMMON_STATIC /Fo:t1_dist_matrix_c\main.obj /std:c17 /W4 /permissive- /Od /Zi /MDd /fsanitize=address t1_dist_matrix_c\main.c
if errorlevel 1 echo FAIL: main_c_compilation_error & exit /b 1
:: compil. geom
cl -c /DCOMMON_STATIC /Fo:common\geometry_c.obj /std:c17 /W4 /permissive- /Od /Zi /MDd /fsanitize=address common\geometry.c
if errorlevel 1 echo FAIL: geometry_c_compilation_error & exit /b 1
:: Link
link /DEBUG /OUT:t1_dist_matrix_c\main.exe t1_dist_matrix_c\main.obj common\geometry_c.obj
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
exit /b 0