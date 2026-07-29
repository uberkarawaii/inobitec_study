@echo off
chcp 1251 > nul

:: формат 
clang-format -i t4_filter_cpp\main.cpp gen_cloud\main.cpp
clang-format -i common\string_utils.hpp common\string_utils.cpp
clang-format -i common\geometry.hpp common\geometry.cpp

:: папка build и подпапки для артефактов
if not exist build mkdir build
if not exist build\common mkdir build\common
if not exist build\t4_filter_cpp mkdir build\t4_filter_cpp
if not exist build\t4_filter_c mkdir build\t4_filter_c
 
:: установка пути, по которому main.exe будет искать .dll
set "PATH=%~dp0/build/common;%PATH%"

:: сборка и линковка динамической библиотеки define COMMON_EXPORTS 
:: выходные файлы - в build - по PATH найдётся
cl -c /DCOMMON_EXPORTS /Fo:build\common\string_utils_cpp.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address common\string_utils.cpp
if errorlevel 1 echo FAIL: string_utils_cpp_compilation_error & exit /b 1

cl -c /DCOMMON_EXPORTS /Fo:build\common\geometry_cpp.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address common\geometry.cpp
if errorlevel 1 echo FAIL: geometry_cpp_compilation_error & exit /b 1

link /DEBUG /DLL /OUT:build\common\common_cpp.dll build\common\string_utils_cpp.obj build\common\geometry_cpp.obj
if errorlevel 1 echo FAIL: dll_cpp_link_error & exit /b 1

:: сборка и линковка main с common.lib
cl -c /Fo:build\t4_filter_cpp\main.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address t4_filter_cpp\main.cpp
if errorlevel 1 echo FAIL: main_cpp_compilation_error & exit /b 1

link /DEBUG /OUT:build\t4_filter_cpp\main.exe build\t4_filter_cpp\main.obj build\common\common_cpp.lib
if errorlevel 1 echo FAIL: main_cpp_link_error & exit /b 1

:: переменные для смены кодировки, чтобы findstr находила то, что ей передают
:: а консоль передаёт ей в OEM 
set "DEFtoOEM=powershell -Command "Set-Content -Path build\t4_filter_cpp\err.txt -Encoding OEM -Value (Get-Content build\t4_filter_cpp\err.txt -Encoding Default -Raw)""
:: в конце применятся эта команда, чтобы в err.txt был читаемый текст
set "OEMtoDEF=powershell -Command "Set-Content -Path build\t4_filter_cpp\err.txt -Encoding Default -Value (Get-Content build\t4_filter_cpp\err.txt -Encoding OEM -Raw)""

:: тесты с ошибочными значениями радиуса
echo CPP TESTS
:: пустой радиус
build\t4_filter_cpp\main.exe > nul 2> build\t4_filter_cpp\err.txt
if not errorlevel 64 echo FAIL: radius empty: wrong code & exit /b 1
if errorlevel 65 echo FAIL: radius empty: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Ожидался радиус; его значение не было введено" build\t4_filter_cpp\err.txt > nul
if errorlevel 1 echo FAIL: radius empty: wrong info in cerr & %OEMtoDEF% & exit /b 1

:: не одно число
build\t4_filter_cpp\main.exe 2 3 > nul 2> build\t4_filter_cpp\err.txt
if not errorlevel 64 echo FAIL: radius too much: wrong code & exit /b 1
if errorlevel 65 echo FAIL: radius too much: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Ожидался радиус; были введены лишние аргументы" build\t4_filter_cpp\err.txt > nul
if errorlevel 1 echo FAIL: radius too much: wrong info in cerr & %OEMtoDEF% & exit /b 1

:: не число в качестве радиуса
build\t4_filter_cpp\main.exe 4ch > nul 2> build\t4_filter_cpp\err.txt
if not errorlevel 64 echo FAIL: radius symbol: wrong code & exit /b 1
if errorlevel 65 echo FAIL: radius symbol: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Радиус должен быть числом. Получено: 4ch" build\t4_filter_cpp\err.txt > nul
if errorlevel 1 echo FAIL: radius symbol: wrong info in cerr & %OEMtoDEF% & exit /b 1

:: не конечное число в качестве аргумента
build\t4_filter_cpp\main.exe inf > nul 2> build\t4_filter_cpp\err.txt
if not errorlevel 64 echo FAIL: inf case: wrong code & exit /b 1
if errorlevel 65 echo FAIL: inf case: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Радиус должен быть конечным числом. Получено: inf" build\t4_filter_cpp\err.txt > nul
if errorlevel 1 echo FAIL: inf case: wrong info in cerr & %OEMtoDEF% & exit /b 1

:: отрицательный радиус
build\t4_filter_cpp\main.exe -10 > nul 2> build\t4_filter_cpp\err.txt
if not errorlevel 64 echo FAIL: radius negative: wrong code & exit /b 1
if errorlevel 65 echo FAIL: radius negative: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Радиус должен быть положительным. Получено: -10" build\t4_filter_cpp\err.txt > nul
if errorlevel 1 echo FAIL: radius negative: wrong info in cerr & %OEMtoDEF% & exit /b 1

:: тесты с ошибочными значениями точек

:: нечисловое значение
echo 2 wow 3 | build\t4_filter_cpp\main.exe 1 > nul 2> build\t4_filter_cpp\err.txt
if not errorlevel 65 echo FAIL: nan point: wrong code & exit /b 1
if errorlevel 66 echo FAIL: nan point: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Строка 1. Нечисловое значение: 2 wow 3" build\t4_filter_cpp\err.txt > nul
if errorlevel 1 echo FAIL: nan point: wrong info in cerr & %OEMtoDEF% & exit /b 1

:: неверное кол-во аргументов
echo 2 3 | build\t4_filter_cpp\main.exe 1 > nul 2> build\t4_filter_cpp\err.txt
if not errorlevel 65 echo FAIL: too few coords: wrong code & exit /b 1
if errorlevel 66 echo FAIL: too few coords: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Строка 1. Ожидалось X Y Z, получено: 2 3" build\t4_filter_cpp\err.txt > nul
if errorlevel 1 echo FAIL:  too few coords: wrong info in cerr & %OEMtoDEF% & exit /b 1

:: отсутствие точек
type nul | build\t4_filter_cpp\main.exe 1 > nul 2> build\t4_filter_cpp\err.txt
if not errorlevel 66 echo FAIL: empty coords: wrong code & exit /b 1
if errorlevel 67 echo FAIL: empty coords: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Точки отсутствуют" build\t4_filter_cpp\err.txt > nul
if errorlevel 1 echo FAIL: empty coords: wrong info in cerr & %OEMtoDEF% & exit /b 1

:: err.txt в читаемую кодировку
%OEMtoDEF%

:: тесты на визуально понятных данных
echo 3 4 5 | build\t4_filter_cpp\main.exe 8 > build\t4_filter_cpp\main_out.txt 
if errorlevel 1 echo FAIL: test1_main_exit_code & exit /b 1
findstr /C:"3.000 4.000 5.000" build\t4_filter_cpp\main_out.txt > nul
if errorlevel 1 echo FAIL: test1_wrong_out & exit /b 1

:: здесь проверяется что все строки в файле 4.000 5.000 5.000 
:: т.к. на входе только одна такая точка, то это будет проверкой, 
:: что никаких строк кроме 4.000 5.000 5.000 в файле нет; /V - отдай то, что не соотв. шаблону после /C:
(echo 4 5 5 & echo 5 11 0 & echo 0 0 11) | build\t4_filter_cpp\main.exe 10 > build\t4_filter_cpp\main_out.txt 
if errorlevel 1 echo FAIL: test2_main_exit_code & exit /b 1
findstr /C:"4.000 5.000 5.000" build\t4_filter_cpp\main_out.txt > nul
if errorlevel 1 echo FAIL: test2_wrong_out & exit /b 1
findstr /V /C:"4.000 5.000 5.000" build\t4_filter_cpp\main_out.txt > nul
if not errorlevel 1 echo FAIL: test2_extra_info_in_out & exit /b 1

:: единств. точка, не проходит. /R "." - regex, любой символ. найди строки с любым символом
echo 3 4 0 | build\t4_filter_cpp\main.exe 5 > build\t4_filter_cpp\main_out.txt 
if errorlevel 1 echo FAIL: test3_main_exit_code & exit /b 1
findstr /R "." build\t4_filter_cpp\main_out.txt > nul
if not errorlevel 1 echo FAIL: test3_not_empty_output & exit /b 1

ECHO ALL CPP TESTS PASSED

:: тесты программы на Си

::формат
clang-format -i t4_filter_c\main.c common\exit_codes.h
clang-format -i common\string_utils.c common\string_utils.h
clang-format -i common\geometry.c common\geometry.h

:: изменение переменных перекодировки, т.к. артефакты теперь лежат в других папках
set "DEFtoOEM=powershell -Command "Set-Content -Path build\t4_filter_c\err.txt -Encoding OEM -Value (Get-Content build\t4_filter_c\err.txt -Encoding Default -Raw)""
set "OEMtoDEF=powershell -Command "Set-Content -Path build\t4_filter_c\err.txt -Encoding Default -Value (Get-Content build\t4_filter_c\err.txt -Encoding OEM -Raw)""

:: сборка (с define COMMON_EXPORTS) и линковка динамиеской библиотеки
:: выходные файлы - в build, и он уже находится в PATH
cl -c /DCOMMON_EXPORTS /Fo:build\common\string_utils_c.obj /std:c17 /W4 /permissive- /Od /Zi /MDd /fsanitize=address common\string_utils.c
if errorlevel 1 echo FAIL: string_utils_c_compilation_error & exit /b 1
cl -c /DCOMMON_EXPORTS /Fo:build\common\geometry_c.obj /std:c17 /W4 /permissive- /Od /Zi /MDd /fsanitize=address common\geometry.c
if errorlevel 1 echo FAIL: geometry_c_compilation_error & exit /b 1

link /DEBUG /DLL /OUT:build\common\common_c.dll build\common\string_utils_c.obj build\common\geometry_c.obj
if errorlevel 1 echo FAIL: dll_c_link_error & exit /b 1

:: сборка и линковка main с common.lib
cl -c /Fo:build\t4_filter_c\main.obj /std:c17 /W4 /permissive- /Od /Zi /MDd /fsanitize=address t4_filter_c\main.c
if errorlevel 1 echo FAIL: main_c_compilation_error & exit /b 1

link /DEBUG /OUT:build\t4_filter_c\main.exe build\t4_filter_c\main.obj build\common\common_c.lib
if errorlevel 1 echo FAIL: main_c_link_error & exit /b 1

:: тесты с ошибочными значениями радиуса
echo C TESTS
:: отсутствие аргумента радиуса
build\t4_filter_c\main.exe > nul 2> build\t4_filter_c\err.txt
if not errorlevel 64 echo FAIL: radius empty: wrong code & exit /b 1 
if errorlevel 65 echo FAIL: radius empty: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Ожидался радиус; его значение не было введено" build\t4_filter_c\err.txt > nul
if errorlevel 1 echo FAIL: radius empty: wrong info in stderr & %OEMtoDEF% & exit /b 1

:: более 1 аргумента
build\t4_filter_c\main.exe 2 3 > nul 2> build\t4_filter_c\err.txt 
if not errorlevel 64 echo FAIL: radius too much: wrong code & exit /b 1
if errorlevel 65 echo FAIL: radius too much: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Ожидался радиус; были введены лишние аргументы" build\t4_filter_c\err.txt > nul
if errorlevel 1 echo FAIL: radius too much: wrong info in stderr & %OEMtoDEF% & exit /b 1

:: нечисловое значение в качестве радиуса
build\t4_filter_c\main.exe 2to3 > nul 2> build\t4_filter_c\err.txt
if not errorlevel 64 echo FAIL: radius symbol: wrong code & exit /b 1
if errorlevel 65 echo FAIL: radius symbol: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Радиус должен быть числом. Получено: 2to3" build\t4_filter_c\err.txt > nul
if errorlevel 1 echo FAIL: radius symbol: wrong info in stderr & %OEMtoDEF% & exit /b 1

:: не конечное число в качестве радиуса
build\t4_filter_c\main.exe inf > nul 2> build\t4_filter_c\err.txt
if not errorlevel 64 echo FAIL: inf case: wrong code & exit /b 1
if errorlevel 65 echo FAIL: inf case: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Радиус должен быть конечным числом. Получено: inf" build\t4_filter_c\err.txt > nul
if errorlevel 1 echo FAIL: inf case: wrong info in cerr & %OEMtoDEF% & exit /b 1

:: отрицательное число в качестве радиуса
build\t4_filter_c\main.exe -23 > nul 2> build\t4_filter_c\err.txt
if not errorlevel 64 echo FAIL: radius negative: wrong code & exit /b 1
if errorlevel 65 echo FAIL: radius negative: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Радиус должен быть положительным. Получено: -23" build\t4_filter_c\err.txt > nul
if errorlevel 1 echo FAIL: radius negative: wrong info in stderr & %OEMtoDEF% & exit /b 1

:: тесты с ошибками при вводе точек
:: нечисловое значение
echo 2 3 err | build\t4_filter_c\main.exe 23 > nul 2> build\t4_filter_c\err.txt
if not errorlevel 65 echo FAIL: nan point: wrong code & exit /b 1
if errorlevel 66 echo FAIL: nan point: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Строка 1. Нечисловое значение: 2 3 err" build\t4_filter_c\err.txt > nul
if errorlevel 1 echo FAIL: nan point: wrong info in stderr & %OEMtoDEF% & exit /b 1

:: неверное кол-во точек
echo 2 3 | build\t4_filter_c\main.exe 23 > nul 2> build\t4_filter_c\err.txt
if not errorlevel 65 echo FAIL: too few coords: wrong code & exit /b 1
if errorlevel 66 echo FAIL: too few coords: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Строка 1. Ожидалось X Y Z, получено: 2 3 " build\t4_filter_c\err.txt > nul
if errorlevel 1 echo FAIL: too few coords: wrong info in stderr & %OEMtoDEF% & exit /b 1

:: отсутствие аргументов
type nul | build\t4_filter_c\main.exe 23 > nul 2> build\t4_filter_c\err.txt
if not errorlevel 66 echo FAIL: empty coords: wrong code & exit /b 1
if errorlevel 67 echo FAIL: empty coords: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Точки отсутствуют" build\t4_filter_c\err.txt > nul
if errorlevel 1 echo FAIL: empty coords: wrong info in stderr & %OEMtoDEF% & exit /b 1
 
:: последний err.txt в читаемую кодировку
%OEMtoDEF%

:: тесты на визуально понятных данных
:: единств. точка и она проходит
echo 3 4 5 | build\t4_filter_c\main.exe 8 > build\t4_filter_c\main_out.txt 
if errorlevel 1 echo FAIL: test1_main_exit_code & exit /b 1
findstr /C:"3.000 4.000 5.000" build\t4_filter_c\main_out.txt > nul
if errorlevel 1 echo FAIL: test1_wrong_out & exit /b 1

:: много точек, одна проходит
(echo 4 5 5 & echo 5 11 0 & echo 0 0 11) | build\t4_filter_c\main.exe 10 > build\t4_filter_c\main_out.txt 
if errorlevel 1 echo FAIL: test2_main_exit_code & exit /b 1
findstr /C:"4.000 5.000 5.000" build\t4_filter_c\main_out.txt > nul
if errorlevel 1 echo FAIL: test2_wrong_out & exit /b 1
:: /V - строки, не подходящие под шаблон после /С: - и если такие найдутся, findstr вернёт 0. если таких строк нет - 1
findstr /V /C:"4.000 5.000 5.000" build\t4_filter_c\main_out.txt > nul
if not errorlevel 1 echo FAIL: test2_extra_info_in_out & exit /b 1

:: единств. точка не проходит
echo 3 4 0 | build\t4_filter_c\main.exe 5 > build\t4_filter_c\main_out.txt 
if errorlevel 1 echo FAIL: test3_main_exit_code & exit /b 1
:: /R = regex, "." - один любой символ. т.е. если найдётся строка с любым символом, - вернётся 0. если файл пустой - 1
findstr /R "." build\t4_filter_c\main_out.txt > nul
if not errorlevel 1 echo FAIL: test3_not_empty_output & exit /b 1

ECHO ALL C TESTS PASSED	
exit /b 0