:: отключение вывода команд на экран
@echo off
chcp 1251 > nul

:: форматирование 
clang-format -i t2_passport_cpp\main.cpp common\exit_codes.hpp
clang-format -i common\string_utils.cpp common\string_utils.hpp

:: папка build и подпапки для артефактов каждой задачи и библиотеки
if not exist build mkdir build
if not exist build\common mkdir build\common
if not exist build\t2_passport_cpp mkdir build\t2_passport_cpp
if not exist build\t2_passport_c mkdir build\t2_passport_c

:: сборка
:: compile main
cl -c /DCOMMON_STATIC /Fo:build\t2_passport_cpp\main.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address t2_passport_cpp\main.cpp
if errorlevel 1 echo FAIL: main_cpp_compilation_error & exit /b 1
:: compile string_utils
cl -c /DCOMMON_STATIC /Fo:build\common\string_utils_cpp.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address common\string_utils.cpp
if errorlevel 1 echo FAIL: string_utils_cpp_compilation_error & exit /b 1

:: Link
link /DEBUG /OUT:build\t2_passport_cpp\main.exe build\t2_passport_cpp\main.obj build\common\string_utils_cpp.obj
if errorlevel 1 echo FAIL: main_cpp_link_error & exit /b 1


:: смена кодировок для main_out чтобы findstr могла нормально искать строки
set "DEFtoOEMmain=powershell -Command "Set-Content -Path build\t2_passport_cpp\main_out.txt -Encoding OEM -Value (Get-Content build\t2_passport_cpp\main_out.txt -Encoding Default -Raw)""
set "OEMtoDEFmain=powershell -Command "Set-Content -Path build\t2_passport_cpp\main_out.txt -Encoding Default -Value (Get-Content build\t2_passport_cpp\main_out.txt -Encoding OEM -Raw)""
                                                              
:: тесты 
echo CPP TESTS

(echo звезда & echo 6) | build\t2_passport_cpp\main.exe > build\t2_passport_cpp\main_out.txt
if errorlevel 1 echo FAIL: norm-case main exit code & exit /b 1
%DEFtoOEMmain%
findstr /C:"Фигура «звезда»: 6 вершин." build\t2_passport_cpp\main_out.txt > nul
if errorlevel 1 echo FAIL: norm-case & %OEMtoDEFmain%  & exit /b 1
%OEMtoDEFmain%

type nul | build\t2_passport_cpp\main.exe 2> nul
if not errorlevel 66 echo FAIL: eof_name & exit /b 1

echo; | build\t2_passport_cpp\main.exe 2> nul
if not errorlevel 65 echo FAIL: empty_name & exit /b 1

(echo многоугольник & type nul) | build\t2_passport_cpp\main.exe 2> nul
if not errorlevel 66 echo FAIL: eof_vertexes & exit /b 1

(echo многоугольник & echo;) | build\t2_passport_cpp\main.exe 2> nul
if not errorlevel 65 echo FAIL: empty_vertexes & exit /b 1

(echo многоугольник & echo -5) | build\t2_passport_cpp\main.exe 2> nul
if not errorlevel 64 echo FAIL: negative_vertexes & exit /b 1 

(echo многоугольник & echo 3.3) |  build\t2_passport_cpp\main.exe 2> nul
if not errorlevel 65 echo FAIL: fractional_vertexes & exit /b 1

(echo многоугольник & echo abc) | build\t2_passport_cpp\main.exe 2> nul
if not errorlevel 65 echo FAIL: nan_vertexes & exit /b 1

echo ALL CPP TESTS PASSED

:: форматирование
clang-format -i t2_passport_c\main.c common\exit_codes.h
clang-format -i common\string_utils.c common\string_utils.h
:: clang-format -i t2_passport_c\ref.c

:: сборка
:: compil. main
cl -c /DCOMMON_STATIC /Fo:build\t2_passport_c\main.obj /std:c17 /W4 /permissive- /Od /Zi /MDd /fsanitize=address t2_passport_c\main.c
if errorlevel 1 echo FAIL: main_c_compilation_error & exit /b 1
:: compil. string_utils
cl -c /DCOMMON_STATIC /Fo:build\common\string_utils_c.obj /std:c17 /W4 /permissive- /Od /Zi /MDd /fsanitize=address common\string_utils.c
if errorlevel 1 echo FAIL: string_utils_c_compilation_error & exit /b 1

:: Link
link /DEBUG /OUT:build\t2_passport_c\main.exe build\t2_passport_c\main.obj build\common\string_utils_c.obj
if errorlevel 1 echo FAIL: main_c_link_error & exit /b 1

:: теперь main_out будет свой в каждой задаче, поэтому переопределение переменной для кодировки
set "DEFtoOEMmain=powershell -Command "Set-Content -Path build\t2_passport_c\main_out.txt -Encoding OEM -Value (Get-Content build\t2_passport_c\main_out.txt -Encoding Default -Raw)""
set "OEMtoDEFmain=powershell -Command "Set-Content -Path build\t2_passport_c\main_out.txt -Encoding Default -Value (Get-Content build\t2_passport_c\main_out.txt -Encoding OEM -Raw)""

:: тесты
echo C TESTS

(echo звезда & echo 6) | build\t2_passport_c\main.exe > build\t2_passport_c\main_out.txt
if errorlevel 1 echo FAIL: norm-case main exit code & exit /b 1
%DEFtoOEMmain%
findstr /C:"Фигура «звезда»: 6 вершин." build\t2_passport_c\main_out.txt > nul
if errorlevel 1 echo FAIL: norm-case & %OEMtoDEFmain%  & exit /b 1
%OEMtoDEFmain%

type nul | build\t2_passport_c\main.exe 2> nul
if not errorlevel 66 echo FAIL: eof_name & exit /b 1

echo; | build\t2_passport_c\main.exe 2> nul
if not errorlevel 65 echo FAIL: empty_name & exit /b 1

(echo многоугольник & type nul) | build\t2_passport_c\main.exe 2> nul
if not errorlevel 66 echo FAIL: eof_vertexes & exit /b 1

(echo многоугольник & echo;) | build\t2_passport_c\main.exe 2> nul
if not errorlevel 65 echo FAIL: empty_vertexes & exit /b 1

(echo многоугольник & echo -5) | build\t2_passport_c\main.exe 2> nul
if not errorlevel 64 echo FAIL: negative_vertexes & exit /b 1 

(echo многоугольник & echo 3.3) |  build\t2_passport_c\main.exe 2> nul
if not errorlevel 65 echo FAIL: fractional_vertexes & exit /b 1

(echo многоугольник & echo abc) | build\t2_passport_c\main.exe 2> nul
if not errorlevel 65 echo FAIL: nan_vertexes & exit /b 1

echo ALL C TESTS PASSED
exit /b 0