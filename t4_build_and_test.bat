@echo off
chcp 1251 > nul

:: формат 
clang-format -i t4_filter_cpp\main.cpp gen_cloud\main.cpp

:: сборка + линковка main
cl -c /Fo:t4_filter_cpp\main.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address t4_filter_cpp\main.cpp
if errorlevel 1 echo FAIL: main_cpp_compilation_error & exit /b 1
link /DEBUG /OUT:t4_filter_cpp\main.exe t4_filter_cpp\main.obj
if errorlevel 1 echo FAIL: main_cpp_link_error & exit /b 1
:: сборка + линковка генератора облака чисел
:: cl -c /Fo:gen_cloud\main.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address gen_cloud\main.cpp
:: if errorlevel 1 echo FAIL: gen_cpp_compilation_error & exit /b 1
:: link /DEBUG /OUT:gen_cloud\gen_cloud.exe gen_cloud\main.obj  
:: if errorlevel 1 echo FAIL: gen_cpp_link_error & exit /b 1
 
:: папка build если её ещё не было
if not exist build mkdir build

:: переменные для смены кодировки, чтобы findstr находила то, что ей передают
:: а консоль передаёт ей в OEM 
set "DEFtoOEM=powershell -Command "Set-Content -Path build\err.txt -Encoding OEM -Value (Get-Content build\err.txt -Encoding Default -Raw)""
:: в конце применятся эта команда, чтобы в err.txt был читаемый текст
set "OEMtoDEF=powershell -Command "Set-Content -Path build\err.txt -Encoding Default -Value (Get-Content build\err.txt -Encoding OEM -Raw)""

:: тесты с ошибочными значениями радиуса
echo CPP TESTS
:: пустой радиус
t4_filter_cpp\main.exe > nul 2> build\err.txt
if not errorlevel 64 echo FAIL: radius empty: wrong code & exit /b 1
if errorlevel 65 echo FAIL: radius empty: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Ожидался радиус; его значение не было введено" build\err.txt > nul
if errorlevel 1 echo FAIL: radius empty: wrong info in cerr & %OEMtoDEF% & exit /b 1

:: не одно число
t4_filter_cpp\main.exe 2 3 > nul 2> build\err.txt
if not errorlevel 64 echo FAIL: radius too much: wrong code & exit /b 1
if errorlevel 65 echo FAIL: radius too much: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Ожидался радиус; были введены лишние аргументы" build\err.txt > nul
if errorlevel 1 echo FAIL: radius too much: wrong info in cerr & %OEMtoDEF% & exit /b 1

:: не число в качестве радиуса
t4_filter_cpp\main.exe 4ch > nul 2> build\err.txt
if not errorlevel 64 echo FAIL: radius symbol: wrong code & exit /b 1
if errorlevel 65 echo FAIL: radius symbol: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Радиус должен быть числом. Получено: 4ch" build\err.txt > nul
if errorlevel 1 echo FAIL: radius symbol: wrong info in cerr & %OEMtoDEF% & exit /b 1

:: не конечное число в качестве аргумента
t4_filter_cpp\main.exe inf > nul 2> build\err.txt
if not errorlevel 64 echo FAIL: inf case: wrong code & exit /b 1
if errorlevel 65 echo FAIL: inf case: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Радиус должен быть конечным числом. Получено: inf" build\err.txt > nul
if errorlevel 1 echo FAIL: inf case: wrong info in cerr & %OEMtoDEF% & exit /b 1

:: отрицательный радиус
t4_filter_cpp\main.exe -10 > nul 2> build\err.txt
if not errorlevel 64 echo FAIL: radius negative: wrong code & exit /b 1
if errorlevel 65 echo FAIL: radius negative: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Радиус должен быть положительным. Получено: -10" build\err.txt > nul
if errorlevel 1 echo FAIL: radius negative: wrong info in cerr & %OEMtoDEF% & exit /b 1

:: тесты с ошибочными значениями точек

:: нечисловое значение
echo 2 wow 3 | t4_filter_cpp\main.exe 1 > nul 2> build\err.txt
if not errorlevel 65 echo FAIL: nan point: wrong code & exit /b 1
if errorlevel 66 echo FAIL: nan point: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Строка 1. Нечисловое значение: 2 wow 3" build\err.txt > nul
if errorlevel 1 echo FAIL: nan point: wrong info in cerr & %OEMtoDEF% & exit /b 1

:: неверное кол-во аргументов
echo 2 3 | t4_filter_cpp\main.exe 1 > nul 2> build\err.txt
if not errorlevel 65 echo FAIL: too few coords: wrong code & exit /b 1
if errorlevel 66 echo FAIL: too few coords: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Строка 1. Ожидалось X Y Z, получено: 2 3" build\err.txt > nul
if errorlevel 1 echo FAIL:  too few coords: wrong info in cerr & %OEMtoDEF% & exit /b 1

:: отсутствие точек
type nul | t4_filter_cpp\main.exe 1 > nul 2> build\err.txt
if not errorlevel 66 echo FAIL: empty coords: wrong code & exit /b 1
if errorlevel 67 echo FAIL: empty coords: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Точки отсутствуют" build\err.txt > nul
if errorlevel 1 echo FAIL: empty coords: wrong info in cerr & %OEMtoDEF% & exit /b 1


:: err.txt в читаемую кодировку
%OEMtoDEF%

:: тесты на визуально понятных данных
echo 3 4 5 | t4_filter_cpp\main.exe 8 > build\main_out.txt 
if errorlevel 1 echo FAIL: test1_main_exit_code & exit /b 1
findstr /C:"3.000 4.000 5.000" build\main_out.txt > nul
if errorlevel 1 echo FAIL: test1_wrong_out & exit /b 1

:: здесь проверяется что все строки в файле 4.000 5.000 5.000 
:: т.к. на входе только одна такая точка, то это будет проверкой, 
:: что никаких строк кроме 4.000 5.000 5.000 в файле нет
(echo 4 5 5 & echo 5 11 0 & echo 0 0 11) | t4_filter_cpp\main.exe 10 > build\main_out.txt 
if errorlevel 1 echo FAIL: test2_main_exit_code & exit /b 1
findstr /C:"4.000 5.000 5.000" build\main_out.txt > nul
if errorlevel 1 echo FAIL: test2_wrong_out & exit /b 1
findstr /V /C:"4.000 5.000 5.000" build\main_out.txt > nul
if not errorlevel 1 echo FAIL: test2_extra_info_in_out & exit /b 1

echo 3 4 0 | t4_filter_cpp\main.exe 5 > build\main_out.txt 
if errorlevel 1 echo FAIL: test3_main_exit_code & exit /b 1
findstr /R "." build\main_out.txt > nul
if not errorlevel 1 echo FAIL: test3_not_empty_output & exit /b 1

ECHO ALL CPP TESTS PASSED

:: тесты программы на Си

::формат
clang-format -i t4_filter_c\main.c 

:: сборка и линковка main.c
cl -c /Fo:t4_filter_c\main.obj /std:c17 /W4 /permissive- /Od /Zi /MDd /fsanitize=address t4_filter_c\main.c
if errorlevel 1 echo FAIL: main_c_compilation_error & exit /b 1
link /DEBUG /OUT:t4_filter_c\main.exe t4_filter_c\main.obj
if errorlevel 1 echo FAIL: main_c_link_error & exit /b 1

:: тесты с ошибочными значениями радиуса
echo C TESTS
:: отсутствие аргумента радиуса
t4_filter_c\main.exe > nul 2> build\err.txt
if not errorlevel 64 echo FAIL: radius empty: wrong code & exit /b 1 
if errorlevel 65 echo FAIL: radius empty: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Ожидался радиус; его значение не было введено" build\err.txt > nul
if errorlevel 1 echo FAIL: radius empty: wrong info in stderr & %OEMtoDEF% & exit /b 1

:: более 1 аргумента
t4_filter_c\main.exe 2 3 > nul 2> build\err.txt
if not errorlevel 64 echo FAIL: radius too much: wrong code & exit /b 1
if errorlevel 65 echo FAIL: radius too much: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Ожидался радиус; были введены лишние аргументы" build\err.txt > nul
if errorlevel 1 echo FAIL: radius too much: wrong info in stderr & %OEMtoDEF% & exit /b 1

:: нечисловое значение в качестве радиуса
t4_filter_c\main.exe 2to3 > nul 2> build\err.txt
if not errorlevel 64 echo FAIL: radius symbol: wrong code & exit /b 1
if errorlevel 65 echo FAIL: radius symbol: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Радиус должен быть числом. Получено: 2to3" build\err.txt > nul
if errorlevel 1 echo FAIL: radius symbol: wrong info in stderr & %OEMtoDEF% & exit /b 1

:: не конечное число в качестве радиуса
t4_filter_c\main.exe inf > nul 2> build\err.txt
if not errorlevel 64 echo FAIL: inf case: wrong code & exit /b 1
if errorlevel 65 echo FAIL: inf case: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Радиус должен быть конечным числом. Получено: inf" build\err.txt > nul
if errorlevel 1 echo FAIL: inf case: wrong info in cerr & %OEMtoDEF% & exit /b 1

:: отрицательное число в качестве радиуса
t4_filter_c\main.exe -23 > nul 2> build\err.txt
if not errorlevel 64 echo FAIL: radius negative: wrong code & exit /b 1
if errorlevel 65 echo FAIL: radius negative: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Радиус должен быть положительным. Получено: -23" build\err.txt > nul
if errorlevel 1 echo FAIL: radius negative: wrong info in stderr & %OEMtoDEF% & exit /b 1

:: тесты с ошибками при вводе точек

:: нечисловое значение
echo 2 3 err | t4_filter_c\main.exe 23 > nul 2> build\err.txt
if not errorlevel 65 echo FAIL: nan point: wrong code & exit /b 1
if errorlevel 66 echo FAIL: nan point: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Строка 1. Нечисловое значение: 2 3 err" build\err.txt > nul
if errorlevel 1 echo FAIL: nan point: wrong info in stderr & %OEMtoDEF% & exit /b 1

:: неверное кол-во точек
echo 2 3 | t4_filter_c\main.exe 23 > nul 2> build\err.txt
if not errorlevel 65 echo FAIL: too few coords: wrong code & exit /b 1
if errorlevel 66 echo FAIL: too few coords: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Строка 1. Ожидалось X Y Z, получено: 2 3 " build\err.txt > nul
if errorlevel 1 echo FAIL: too few coords: wrong info in stderr & %OEMtoDEF% & exit /b 1

:: отсутствие аргументов
type nul | t4_filter_c\main.exe 23 > nul 2> build\err.txt
if not errorlevel 66 echo FAIL: empty coords: wrong code & exit /b 1
if errorlevel 67 echo FAIL: empty coords: wrong code & exit /b 1
%DEFtoOEM%
findstr /C:"Точки отсутствуют" build\err.txt > nul
if errorlevel 1 echo FAIL: empty coords: wrong info in stderr & %OEMtoDEF% & exit /b 1
 
:: последний err.txt в читаемую кодировку
%OEMtoDEF%

:: тесты на визуально понятных данных

echo 3 4 5 | t4_filter_c\main.exe 8 > build\main_out.txt 
if errorlevel 1 echo FAIL: test1_main_exit_code & exit /b 1
findstr /C:"3.000 4.000 5.000" build\main_out.txt > nul
if errorlevel 1 echo FAIL: test1_wrong_out & exit /b 1

(echo 4 5 5 & echo 5 11 0 & echo 0 0 11) | t4_filter_c\main.exe 10 > build\main_out.txt 
if errorlevel 1 echo FAIL: test2_main_exit_code & exit /b 1
findstr /C:"4.000 5.000 5.000" build\main_out.txt > nul
if errorlevel 1 echo FAIL: test2_wrong_out & exit /b 1
findstr /V /C:"4.000 5.000 5.000" build\main_out.txt > nul
if not errorlevel 1 echo FAIL: test2_extra_info_in_out & exit /b 1

echo 3 4 0 | t4_filter_c\main.exe 5 > build\main_out.txt 
if errorlevel 1 echo FAIL: test3_main_exit_code & exit /b 1
findstr /R "." build\main_out.txt > nul
if not errorlevel 1 echo FAIL: test3_not_empty_output & exit /b 1


:: тесты с генератором облака точек и powershell reference
::gen_cloud\gen_cloud --size 10 --seed 42 | t4_filter_c\main.exe 100 > build\main_out.txt
::if errorlevel 1 echo FAIL: norm_case_10_42 main exit code & exit /b 1
::gen_cloud\gen_cloud --size 10 --seed 42 | powershell -ExecutionPolicy Bypass -File t4_filter_ref.ps1 100 > build\ref_out.txt
::fc build\main_out.txt build\ref_out.txt > nul
::if errorlevel 1 echo FAIL: norm_case_10_42 & exit /b 1


ECHO ALL C TESTS PASSED	