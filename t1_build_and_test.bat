::не показывать команды, прописанные ниже
@echo off
:: кодировка. и перенаправить вывод сообщение о кодировке "вникуда"
chcp 1251 > nul

:: ===== C++ (t1_dist_matrix) =====
:: ‘ормат
clang-format -i t1_dist_matrix_cpp\main.cpp common\exit_codes.hpp
clang-format -i common\geometry.hpp common\geometry.cpp

:: папка build и подпапки дл€ артефактов, если этих папок ещЄ нет
if not exist build mkdir build
if not exist build\common mkdir build\common
if not exist build\t1_dist_matrix_cpp mkdir build\t1_dist_matrix_cpp
if not exist build\t1_dist_matrix_c mkdir build\t1_dist_matrix_c 

:: 2. —борка
:: compil. main
cl -c /DCOMMON_STATIC /Fo:build\t1_dist_matrix_cpp\main.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address t1_dist_matrix_cpp\main.cpp
if errorlevel 1 echo FAIL: main_cpp_compilation_error & exit /b 1
:: compil. geometry 
cl -c /DCOMMON_STATIC /Fo:build\common\geometry_cpp.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address common\geometry.cpp
if errorlevel 1 echo FAIL: geometry_cpp_compilation_error & exit /b 1

:: Link
link /DEBUG /OUT:build\t1_dist_matrix_cpp\main.exe build\t1_dist_matrix_cpp\main.obj build\common\geometry_cpp.obj
if errorlevel 1 echo FAIL: main_cpp_link_error & exit /b 1


echo C++ TESTS
:: 3. Acceptance-тесты                                             	
echo 3 | build\t1_dist_matrix_cpp\main.exe > build\t1_dist_matrix_cpp\main_out.txt
if errorlevel 1 echo FAIL: norm-case main exit code & exit /b 1
findstr /C:"   0.000   1.732   1.732" build\t1_dist_matrix_cpp\main_out.txt > nul
if errorlevel 1 echo FAIL: norm-case & exit /b 1

:: перенаправить сообщение об ошибке "вникуда". 2 - это номер потока stderr
echo abc | build\t1_dist_matrix_cpp\main.exe 2> nul
if not errorlevel 65 echo FAIL: non-number & exit /b 1

type nul | build\t1_dist_matrix_cpp\main.exe 2> nul
if not errorlevel 66 echo FAIL: empty & exit /b 1

echo 2.3 | build\t1_dist_matrix_cpp\main.exe 2> nul
if not errorlevel 65 echo FAIL: not-integer & exit /b 1

echo 2 | build\t1_dist_matrix_cpp\main.exe 2> nul
if not errorlevel 64 echo FAIL: range-low & exit /b 1

echo 21 | build\t1_dist_matrix_cpp\main.exe 2> nul
if not errorlevel 64 echo FAIL: range-high & exit /b 1

echo ALL TESTS PASSED

:: ===== C (t1_dist_matrix_c) =====
:: формат - main, codes, geometry, string_utils
clang-format -i t1_dist_matrix_c\main.c common\exit_codes.h
clang-format -i common\geometry.c common\geometry.h
:: clang-format -i t1_dist_matrix_c\ref.c

:: сборка
:: compil. main
cl -c /DCOMMON_STATIC /Fo:build\t1_dist_matrix_c\main.obj /std:c17 /W4 /permissive- /Od /Zi /MDd /fsanitize=address t1_dist_matrix_c\main.c
if errorlevel 1 echo FAIL: main_c_compilation_error & exit /b 1
:: compil. geom
cl -c /DCOMMON_STATIC /Fo:build\common\geometry_c.obj /std:c17 /W4 /permissive- /Od /Zi /MDd /fsanitize=address common\geometry.c
if errorlevel 1 echo FAIL: geometry_c_compilation_error & exit /b 1
:: Link
link /DEBUG /OUT:build\t1_dist_matrix_c\main.exe build\t1_dist_matrix_c\main.obj build\common\geometry_c.obj
if errorlevel 1 echo FAIL: main_c_link_error & exit /b 1
:: cl -c /Fo:t1_dist_matrix_c\ref.obj /std:c17 /W4 /permissive- /Od /Zi /MDd /fsanitize=address t1_dist_matrix_c\ref.c
:: if errorlevel 1 echo FAIL: ref_c_compilation_error & exit /b 1
:: link /DEBUG /OUT:t1_dist_matrix_c\ref.exe t1_dist_matrix_c\ref.obj
:: if errorlevel 1 echo FAIL: ref_c_link_error & exit /b 1

echo C TESTS
::acceptance тесты
echo 3 | build\t1_dist_matrix_c\main.exe > build\t1_dist_matrix_c\main_out.txt
if errorlevel 1 echo FAIL: norm-case main exit code & exit /b 1
findstr /C:"   0.000   1.732   1.732" build\t1_dist_matrix_c\main_out.txt > nul
if errorlevel 1 echo FAIL: norm-case & exit /b 1

type nul | build\t1_dist_matrix_c\main.exe 2> nul
if not errorlevel 66 echo FAIL: empty & exit /b 1

echo abc | build\t1_dist_matrix_c\main.exe 2> nul
if not errorlevel 65 echo FAIL: non-number & exit /b 1

echo 2.3 | build\t1_dist_matrix_c\main.exe 2> nul
if not errorlevel 65 echo FAIL: not-integer & exit /b 1

echo 1 | build\t1_dist_matrix_c\main.exe 2> nul
if not errorlevel 64 echo FAIL: range-low & exit /b 1

echo 100 | build\t1_dist_matrix_c\main.exe 2> nul
if not errorlevel 64 echo FAIL: range-high & exit /b 1  

echo ALL TESTS PASSED
exit /b 0