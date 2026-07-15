:: отключение вывода команд на экран
@echo off
chcp 1251 > nul

:: форматирование 
clang-format -i t2_passport_cpp\main.cpp common\string_utils.hpp
:: clang-format -i t2_passport_cpp\ref.cpp

:: сборка
cl -c /Fo:t2_passport_cpp\main.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address t2_passport_cpp\main.cpp
if errorlevel 1 echo FAIL: main_cpp_compilation_error & exit /b 1
link /DEBUG /OUT:t2_passport_cpp\main.exe t2_passport_cpp\main.obj
if errorlevel 1 echo FAIL: main_cpp_link_error & exit /b 1

:: cl -c /Fo:t2_passport_cpp\ref.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address t2_passport_cpp\ref.cpp
:: if errorlevel 1 echo FAIL: ref_cpp_compilation_error & exit /b 1
:: link /DEBUG /OUT:t2_passport_cpp\ref.exe t2_passport_cpp\ref.obj
:: if errorlevel 1 echo FAIL: ref_cpp_link_error & exit /b 1

:: смена кодировок для main_out чтобы findstr могла нормально искать строки
set "DEFtoOEMmain=powershell -Command "Set-Content -Path build\main_out.txt -Encoding OEM -Value (Get-Content build\main_out.txt -Encoding Default -Raw)""
set "OEMtoDEFmain=powershell -Command "Set-Content -Path build\main_out.txt -Encoding Default -Value (Get-Content build\main_out.txt -Encoding OEM -Raw)""

                                                              
:: тесты 
echo CPP TESTS

(echo звезда & echo 6) | t2_passport_cpp\main.exe > build\main_out.txt
if errorlevel 1 echo FAIL: norm-case main exit code & exit /b 1
%DEFtoOEMmain%
findstr /C:"Фигура «звезда»: 6 вершин." build\main_out.txt > nul
if errorlevel 1 echo FAIL: norm-case & %OEMtoDEFmain%  & exit /b 1
%OEMtoDEFmain%

type nul | t2_passport_cpp\main.exe 2> nul
if not errorlevel 66 echo FAIL: eof_name & exit /b 1

echo; | t2_passport_cpp\main.exe 2> nul
if not errorlevel 65 echo FAIL: empty_name & exit /b 1

(echo многоугольник & type nul) | t2_passport_cpp\main.exe 2> nul
if not errorlevel 66 echo FAIL: eof_vertexes & exit /b 1

(echo многоугольник & echo;) | t2_passport_cpp\main.exe 2> nul
if not errorlevel 65 echo FAIL: empty_vertexes & exit /b 1

(echo многоугольник & echo -5) | t2_passport_cpp\main.exe 2> nul
if not errorlevel 64 echo FAIL: negative_vertexes & exit /b 1 

(echo многоугольник & echo 3.3) |  t2_passport_cpp\main.exe 2> nul
if not errorlevel 65 echo FAIL: fractional_vertexes & exit /b 1

(echo многоугольник & echo abc) | t2_passport_cpp\main.exe 2> nul
if not errorlevel 65 echo FAIL: nan_vertexes & exit /b 1

echo ALL CPP TESTS PASSED

:: форматирование
clang-format -i t2_passport_c\main.c common\string_utils.h
:: clang-format -i t2_passport_c\ref.c

:: сборка
cl -c /Fo:t2_passport_c\main.obj /std:c17 /W4 /permissive- /Od /Zi /MDd /fsanitize=address t2_passport_c\main.c
if errorlevel 1 echo FAIL: main_c_compilation_error & exit /b 1
link /DEBUG /OUT:t2_passport_c\main.exe t2_passport_c\main.obj
if errorlevel 1 echo FAIL: main_c_link_error & exit /b 1

:: cl -c /Fo:t2_passport_c\ref.obj /std:c17 /W4 /permissive- /Od /Zi /MDd /fsanitize=address t2_passport_c\ref.c
:: if errorlevel 1 echo FAIL: ref_c_compilation_error & exit /b 1
:: link /DEBUG /OUT:t2_passport_c\ref.exe t2_passport_c\ref.obj
:: if errorlevel 1 echo FAIL: ref_c_link_error & exit /b 1


:: тесты
echo C TESTS

(echo звезда & echo 6) | t2_passport_c\main.exe > build\main_out.txt
if errorlevel 1 echo FAIL: norm-case main exit code & exit /b 1
%DEFtoOEMmain%
findstr /C:"Фигура «звезда»: 6 вершин." build\main_out.txt > nul
if errorlevel 1 echo FAIL: norm-case & %OEMtoDEFmain%  & exit /b 1
%OEMtoDEFmain%

type nul | t2_passport_c\main.exe 2> nul
if not errorlevel 66 echo FAIL: eof_name & exit /b 1

echo; | t2_passport_c\main.exe 2> nul
if not errorlevel 65 echo FAIL: empty_name & exit /b 1

(echo многоугольник & type nul) | t2_passport_c\main.exe 2> nul
if not errorlevel 66 echo FAIL: eof_vertexes & exit /b 1

(echo многоугольник & echo;) | t2_passport_c\main.exe 2> nul
if not errorlevel 65 echo FAIL: empty_vertexes & exit /b 1

(echo многоугольник & echo -5) | t2_passport_c\main.exe 2> nul
if not errorlevel 64 echo FAIL: negative_vertexes & exit /b 1 

(echo многоугольник & echo 3.3) |  t2_passport_c\main.exe 2> nul
if not errorlevel 65 echo FAIL: fractional_vertexes & exit /b 1

(echo многоугольник & echo abc) | t2_passport_c\main.exe 2> nul
if not errorlevel 65 echo FAIL: nan_vertexes & exit /b 1

echo ALL C TESTS PASSED