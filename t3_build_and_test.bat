:: выключить вывод bat команд + русская кодировка
@echo off
chcp 1251 > nul

:: формат 
clang-format -i t3_bbox_cpp\main.cpp gen_cloud\main.cpp

:: сборка + линковка main
cl -c /Fo:t3_bbox_cpp\main.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address t3_bbox_cpp\main.cpp
link /DEBUG /OUT:t3_bbox_cpp\main.exe t3_bbox_cpp\main.obj 
:: сборка + линковка генератора облака чисел
cl -c /Fo:gen_cloud\main.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address gen_cloud\main.cpp
link /DEBUG /OUT:gen_cloud\gen_cloud.exe gen_cloud\main.obj
:: если были ошибки при сборке / линковке
if errorlevel 1 echo FAIL: compilation & exit /b 1

:: папка build если её ещё не было
if not exist build mkdir build

:: переменные для смены кодировки, чтобы findstr находила то, что ей передают
:: а консоль передаёт ей в OEM 
set "DEFtoOEM=powershell -Command "Set-Content -Path build\err.txt -Encoding OEM -Value (Get-Content build\err.txt -Encoding Default -Raw)""
:: в конце применятся эта команда, чтобы в err.txt был читаемый текст
set "OEMtoDEF=powershell -Command "Set-Content -Path build\err.txt -Encoding Default -Value (Get-Content build\err.txt -Encoding OEM -Raw)""

:: тесты с main и кодами ошибок

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

:: тесты с t3_bbox_ref.ps1 с разным кол-вом точек

gen_cloud\gen_cloud --size 10 --seed 42 | t3_bbox_cpp\main.exe > build\main_out.txt
if errorlevel 1 echo FAIL: norm_case_10_42: main exit code & exit /b 1
gen_cloud\gen_cloud --size 10 --seed 42 | powershell -ExecutionPolicy Bypass -File t3_bbox_ref.ps1 > build\ref_out.txt
fc build\main_out.txt build\ref_out.txt > nul
if errorlevel 1 echo FAIL: norm_case_10_42 & exit /b 1

gen_cloud\gen_cloud --size 100 --seed 42 | t3_bbox_cpp\main.exe > build\main_out.txt
if errorlevel 1 echo FAIL: norm_case_100_42: main exit code & exit /b 1
gen_cloud\gen_cloud --size 100 --seed 42 | powershell -ExecutionPolicy Bypass -File t3_bbox_ref.ps1 > build\ref_out.txt
fc build\main_out.txt build\ref_out.txt > nul
if errorlevel 1 echo FAIL: norm_case_100_42 & exit /b 1

gen_cloud\gen_cloud --size 1000 --seed 42 | t3_bbox_cpp\main.exe > build\main_out.txt
if errorlevel 1 echo FAIL: norm_case_1000_42: main exit code & exit /b 1
gen_cloud\gen_cloud --size 1000 --seed 42 | powershell -ExecutionPolicy Bypass -File t3_bbox_ref.ps1 > build\ref_out.txt
fc build\main_out.txt build\ref_out.txt  > nul
if errorlevel 1 echo FAIL: norm_case_1000_42 & exit /b 1

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
link /DEBUG /OUT:t3_bbox_c\main.exe t3_bbox_c\main.obj
:: проверка на ошибки сборки / линковки
if errorlevel 1 echo FAIL: compilation & exit /b 1

:: тесты с кодами ошибок
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
echo 4 5 | t3_bbox_c\main.exe > nul 2> build\err.txt
if not errorlevel 65 echo FAIL: wrong code too_few_args_case & exit /b 1
if errorlevel 66 echo FAIL: wrong code too_few_args_case & exit /b 1
%DEFtoOEM%
findstr /C:"Строка 1. Ожидались координаты X Y Z. Получено: 4 5" build\err.txt > nul
if errorlevel 1 echo FAIL: too_few_args_case: not found info in cerr & %OEMtoDEF% & exit /b 1

:: если всё удачно, пересохранить err.txt в 1251
%OEMtoDEF%

:: тесты с t3_bbox_ref.ps1 с разным кол-вом точек
gen_cloud\gen_cloud --size 10 --seed 42 | t3_bbox_c\main.exe > build\main_out.txt
if errorlevel 1 echo FAIL: norm_case_10_42: main exit code & exit /b 1
gen_cloud\gen_cloud --size 10 --seed 42 | powershell -ExecutionPolicy Bypass -File t3_bbox_ref.ps1 > build\ref_out.txt
fc build\main_out.txt build\ref_out.txt > nul
if errorlevel 1 echo FAIL: normal_case_10_42 & exit /b 1

gen_cloud\gen_cloud --size 100 --seed 42 | t3_bbox_c\main.exe > build\main_out.txt
if errorlevel 1 echo FAIL: norm_case_100_42: main exit code & exit /b 1
gen_cloud\gen_cloud --size 100 --seed 42 | powershell -ExecutionPolicy Bypass -File t3_bbox_ref.ps1 > build\ref_out.txt
fc build\main_out.txt build\ref_out.txt > nul
if errorlevel 1 echo FAIL: normal_case_100_42 & exit /b 1

gen_cloud\gen_cloud --size 1000 --seed 42 | t3_bbox_c\main.exe > build\main_out.txt
if errorlevel 1 echo FAIL: norm_case_1000_42: main exit code & exit /b 1
gen_cloud\gen_cloud --size 1000 --seed 42 | powershell -ExecutionPolicy Bypass -File t3_bbox_ref.ps1 > build\ref_out.txt
fc build\main_out.txt build\ref_out.txt > nul
if errorlevel 1 echo FAIL: normal_case_1000_42 & exit /b 1

ECHO ALL C TESTS PASSED 