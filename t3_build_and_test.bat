:: выключить вывод bat команд + русская кодировка
@echo off
chcp 1251 > nul

:: формат 
clang-format -i t3_bbox_cpp\main.cpp gen_cloud\main.cpp

:: сборка + линковка main
cl -c /Fo:t3_bbox_cpp\main.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address t3_bbox_cpp\main.cpp
if errorlevel 1 echo FAIL: main_cpp_compilation_error & exit /b 1
link /DEBUG /OUT:t3_bbox_cpp\main.exe t3_bbox_cpp\main.obj
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

:: и для main
set "DEFtoOEMmain=powershell -Command "Set-Content -Path build\main_out.txt -Encoding OEM -Value (Get-Content build\main_out.txt -Encoding Default -Raw)""
set "OEMtoDEFmain=powershell -Command "Set-Content -Path build\main_out.txt -Encoding Default -Value (Get-Content build\main_out.txt -Encoding OEM -Raw)""


:: тесты с main и кодами ошибок
echo CPP TESTS
:: пустой ввод
type nul | t3_bbox_cpp\main.exe > nul 2> build\err.txt
if not errorlevel 66 echo FAIL: empty_case & exit /b 1
%DEFtoOEM%
findstr /C:"Входные данные отсутствуют" build\err.txt > nul
if errorlevel 1 echo FAIL: no info in cerr about empty input & %OEMtoDEF% & exit /b 1          

:: не число во входном потоке
echo 3 2 bam | t3_bbox_cpp\main.exe > nul 2> build\err.txt 
:: провекрка что выходной код именно 65 - не более и не менее
if errorlevel 66 echo FAIL: not_digit_case & exit /b 1 
if not errorlevel 65 echo FAIL: not_digit_case & exit /b 1 
:: поиск вывода ошибочной строки с номером
%DEFtoOEM%
findstr /C:"Строка 1. Нечисловые данные: 3 2 bam" build\err.txt > nul
if errorlevel 1 echo FAIL: not_digit_case: not found info in cerr & %OEMtoDEF% & exit /b 1 

:: неполный набор аргументов X Y Z
echo 2 3 | t3_bbox_cpp\main.exe > nul 2> build\err.txt
:: проверка что код == 65
if not errorlevel 65 echo FAIL: too few args case & exit /b 1
if errorlevel 66 echo FAIL: too few args case & exit /b 1
:: поиск вывода ошибочной строки с номером
%DEFtoOEM%
findstr /C:"Строка 1. Ожидались координаты X Y Z. Получено: 2 3" build\err.txt > nul
if errorlevel 1 echo FAIL: too few args case: not found info in cerr & %OEMtoDEF% & exit /b 1

:: если всё удачно, пересохранить err.txt в 1251
%OEMtoDEF%

:: тесты на визуально понятных данных
:: одна точка на вход - она и будет центроидом
echo 1.5 2.5 3.5 | t3_bbox_cpp\main.exe > build\main_out.txt
if errorlevel 1 echo FAIL: test1 main exit code & exit /b 1
%DEFtoOEMmain%
findstr /C:"Количество точек: 1" build\main_out.txt > nul 
if errorlevel 1 echo FAIL: test1_wrong_quantity & %OEMtoDEFmain% & exit /b 1
findstr /C:"Координаты огранич. паралеллепипеда x: [ 1.500; 1.500 ] y: [ 2.500; 2.500 ] z: [ 3.500; 3.500 ]" build\main_out.txt > nul
if errorlevel 1 echo FAIL: test1_wrong_boundaries & %OEMtoDEFmain% & exit /b 1
findstr /C:"Координаты центроида x: 1.500, y: 2.500, z: 3.500" build\main_out.txt > nul
if errorlevel 1 echo FAIL: test1_wrong_center_coords & %OEMtoDEFmain% & exit /b 1
findstr /C:"Среднее расстояние от точек до центроида: 0.000" build\main_out.txt > nul
if errorlevel 1 echo FAIL: test1_wrong_avg_dist & %OEMtoDEFmain% & exit /b 1


(echo 0 0 0 & echo 1 0 0 & echo 0 1 0) | t3_bbox_cpp\main.exe > build\main_out.txt
if errorlevel 1 echo FAIL: test2 main exit code & exit /b 1
%DEFtoOEMmain%
findstr /C:"Количество точек: 3" build\main_out.txt > nul 
if errorlevel 1 echo FAIL: test2_wrong_quantity & %OEMtoDEFmain% & exit /b 1
findstr /C:"Координаты огранич. паралеллепипеда x: [ 0.000; 1.000 ] y: [ 0.000; 1.000 ] z: [ 0.000; 0.000 ]" build\main_out.txt > nul
if errorlevel 1 echo FAIL: test2_wrong_boundaries & %OEMtoDEFmain% & exit /b 1
findstr /C:"Координаты центроида x: 0.333, y: 0.333, z: 0.000" build\main_out.txt > nul
if errorlevel 1 echo FAIL: test2_wrong_center_coords & %OEMtoDEFmain% & exit /b 1
findstr /C:"Среднее расстояние от точек до центроида: 0.654" build\main_out.txt > nul
if errorlevel 1 echo FAIL: test2_wrong_avg_dist & %OEMtoDEFmain% & exit /b 1

:: при сложениях точек по итогу сумма будет = 0
(echo 0 -16 0 & echo 0 16 0 & echo 16 0 0 & echo -16 0 0) | t3_bbox_cpp\main.exe > build\main_out.txt
if errorlevel 1 echo FAIL: test3 main exit code & exit /b 1
%DEFtoOEMmain%
findstr /C:"Количество точек: 4" build\main_out.txt > nul 
if errorlevel 1 echo FAIL: test3_wrong_quantity & %OEMtoDEFmain% & exit /b 1
findstr /C:"Координаты огранич. паралеллепипеда x: [ -16.000; 16.000 ] y: [ -16.000; 16.000 ] z: [ 0.000; 0.000 ]" build\main_out.txt > nul
if errorlevel 1 echo FAIL: test3_wrong_boundaries & %OEMtoDEFmain% & exit /b 1
findstr /C:"Координаты центроида x: 0.000, y: 0.000, z: 0.000" build\main_out.txt > nul
if errorlevel 1 echo FAIL: test3_wrong_center_coords & %OEMtoDEFmain% & exit /b 1
findstr /C:"Среднее расстояние от точек до центроида: 16.000" build\main_out.txt > nul
if errorlevel 1 echo FAIL: test3_wrong_avg_dist & %OEMtoDEFmain% & exit /b 1
%OEMtoDEFmain%


:: тест на 10^6 - выполняется долго по сравнению с остальными, поэтому закомментирован
:: gen_cloud\gen_cloud --size 1000000 --seed 42 | t3_bbox_cpp\main.exe > build\main_out.txt
:: if errorlevel 1 echo FAIL: main exit code & exit /b 1
:: gen_cloud\gen_cloud --size 1000000 --seed 42 | powershell -ExecutionPolicy Bypass -File t3_bbox_ref.ps1 > build\ref_out.txt
:: fc build\main_out.txt build\ref_out.txt  > nul
:: if errorlevel 1 echo FAIL: norm_case_1000000_42 & exit /b 1

ECHO ALL CPP TESTS PASSED

:: C тесты
:: форматирование 
clang-format -i t3_bbox_c\main.c

:: сборка и линковка main.c 
cl -c /Fo:t3_bbox_c\main.obj /std:c17 /W4 /permissive- /Od /Zi /MDd /fsanitize=address t3_bbox_c\main.c
if errorlevel 1 echo FAIL: main_c_compilation_error & exit /b 1
link /DEBUG /OUT:t3_bbox_c\main.exe t3_bbox_c\main.obj
if errorlevel 1 echo FAIL: main_c_link_error & exit /b 1
                                                            
:: тесты с кодами ошибок
echo C TESTS

type nul | t3_bbox_c\main.exe > nul 2> build\err.txt
if not errorlevel 66 echo FAIL: empty_case & exit /b 1
%DEFtoOEM%
findstr /C:"Входные данные отсутствуют" build\err.txt > nul
if errorlevel 1 echo FAIL: no info in cerr about empty input & %OEMtoDEF% & exit /b 1          

:: не число в потоке
echo 3 2 bam | t3_bbox_c\main.exe > nul 2> build\err.txt 
if errorlevel 66 echo FAIL: not_digit_case & exit /b 1 
if not errorlevel 65 echo FAIL: not_digit_case & exit /b 1 
%DEFtoOEM%
findstr /C:"Строка 1. Нечисловые данные: 3 2 bam" build\err.txt > nul
if errorlevel 1 echo FAIL: not_digit_case: not found info in cerr & %OEMtoDEF% & exit /b 1

:: недостаточное кол-во аргументов
echo 2 3 | t3_bbox_c\main.exe > nul 2> build\err.txt
if not errorlevel 65 echo FAIL: wrong code too_few_args_case & exit /b 1
if errorlevel 66 echo FAIL: wrong code too_few_args_case & exit /b 1
%DEFtoOEM%
findstr /C:"Строка 1. Ожидались координаты X Y Z. Получено: 2 3" build\err.txt > nul
if errorlevel 1 echo FAIL: too_few_args_case: not found info in cerr & %OEMtoDEF% & exit /b 1

:: если всё удачно, пересохранить err.txt в 1251
%OEMtoDEF%

:: тесты на визуально понятных данных

echo 1.5 2.5 3.5 | t3_bbox_c\main.exe > build\main_out.txt
if errorlevel 1 echo FAIL: test1 main exit code & exit /b 1
%DEFtoOEMmain%
findstr /C:"Количество точек: 1" build\main_out.txt > nul 
if errorlevel 1 echo FAIL: test1_wrong_quantity & %OEMtoDEFmain% & exit /b 1
findstr /C:"Координаты огранич. паралеллепипеда x: [ 1.500; 1.500 ] y: [ 2.500; 2.500 ] z: [ 3.500; 3.500 ]" build\main_out.txt > nul
if errorlevel 1 echo FAIL: test1_wrong_boundaries & %OEMtoDEFmain% & exit /b 1
findstr /C:"Координаты центроида x: 1.500, y: 2.500, z: 3.500" build\main_out.txt > nul
if errorlevel 1 echo FAIL: test1_wrong_center_coords & %OEMtoDEFmain% & exit /b 1
findstr /C:"Среднее расстояние от точек до центроида: 0.000" build\main_out.txt > nul
if errorlevel 1 echo FAIL: test1_wrong_avg_dist & %OEMtoDEFmain% & exit /b 1


(echo 0 0 0 & echo 1 0 0 & echo 0 1 0) | t3_bbox_c\main.exe > build\main_out.txt
if errorlevel 1 echo FAIL: test2 main exit code & exit /b 1
%DEFtoOEMmain%
findstr /C:"Количество точек: 3" build\main_out.txt > nul 
if errorlevel 1 echo FAIL: test2_wrong_quantity & %OEMtoDEFmain% & exit /b 1
findstr /C:"Координаты огранич. паралеллепипеда x: [ 0.000; 1.000 ] y: [ 0.000; 1.000 ] z: [ 0.000; 0.000 ]" build\main_out.txt > nul
if errorlevel 1 echo FAIL: test2_wrong_boundaries & %OEMtoDEFmain% & exit /b 1
findstr /C:"Координаты центроида x: 0.333, y: 0.333, z: 0.000" build\main_out.txt > nul
if errorlevel 1 echo FAIL: test2_wrong_center_coords & %OEMtoDEFmain% & exit /b 1
findstr /C:"Среднее расстояние от точек до центроида: 0.654" build\main_out.txt > nul
if errorlevel 1 echo FAIL: test2_wrong_avg_dist & %OEMtoDEFmain% & exit /b 1


(echo 0 -16 0 & echo 0 16 0 & echo 16 0 0 & echo -16 0 0) | t3_bbox_c\main.exe > build\main_out.txt
if errorlevel 1 echo FAIL: test3 main exit code & exit /b 1
%DEFtoOEMmain%
findstr /C:"Количество точек: 4" build\main_out.txt > nul 
if errorlevel 1 echo FAIL: test3_wrong_quantity & %OEMtoDEFmain% & exit /b 1
findstr /C:"Координаты огранич. паралеллепипеда x: [ -16.000; 16.000 ] y: [ -16.000; 16.000 ] z: [ 0.000; 0.000 ]" build\main_out.txt > nul
if errorlevel 1 echo FAIL: test3_wrong_boundaries & %OEMtoDEFmain% & exit /b 1
findstr /C:"Координаты центроида x: 0.000, y: 0.000, z: 0.000" build\main_out.txt > nul
if errorlevel 1 echo FAIL: test3_wrong_center_coords & %OEMtoDEFmain% & exit /b 1
findstr /C:"Среднее расстояние от точек до центроида: 16.000" build\main_out.txt > nul
if errorlevel 1 echo FAIL: test3_wrong_avg_dist & %OEMtoDEFmain% & exit /b 1
%OEMtoDEFmain%

ECHO ALL C TESTS PASSED 