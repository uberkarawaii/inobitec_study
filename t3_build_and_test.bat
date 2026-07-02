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

:: тесты с main и кодами ошибок

:: пустой ввод
type nul | t3_bbox_cpp\main.exe > nul 2> build\err.txt
if not errorlevel 66 echo FAIL: empty_case & exit /b 1
findstr /C:" " build\err.txt > nul
if errorlevel 1 echo FAIL: no info in cerr about empty input & exit /b 1          

:: не число во входном потоке
echo 3 2 bam | t3_bbox_cpp\main.exe > nul 2> build\err.txt 
:: провекрка что выходной код именно 65 - не более и не менее
if errorlevel 66 echo FAIL: not_digit_case & exit /b 1
if not errorlevel 65 echo FAIL: not_digit_case & exit /b 1
:: поиск входной строки в файле с ошибкой
findstr /C:"3 2 bam" build\err.txt > nul
if errorlevel 1 echo FAIL: not_digit_case: no input line is cerr & exit /b 1
:: поиск строки с номером в файле с ошибкой
findstr /C:" 1." build\err.txt > nul
if errorlevel 1 echo FAIL: not_digit_case: no string number in cerr & exit /b 1

:: неполный набор аргументов X Y Z
echo 2 3 | t3_bbox_cpp\main.exe > nul 2> build\err.txt
:: проверка что код == 65
if not errorlevel 65 echo FAIL: too few args case & exit /b 1
if errorlevel 66 echo FAIL: too few args case & exit /b 1
:: проверка что в выходном файле ошибки есть сама входная строка
findstr /C:"2 3" build\err.txt > nul
if errorlevel 1 echo FAIL: too few args case: no input line in cerr & exit /b 1
:: проверка что в выходном файле ошибки есть номер строки с ошибкой
findstr /C:" 1." build\err.txt > nul
if errorlevel 1 echo FAIL: too few args case: no line number in cerr & exit /b 1 

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
findstr /C:" " build\err.txt > nul
if errorlevel 1 echo FAIL: no stderr in empty_case & exit /b 1

:: не число в потоке
echo 2 3 bam | t3_bbox_c\main.exe > nul 2> build\err.txt
if not errorlevel 65 echo FAIL: wrong code not_number_case & exit /b 1
if errorlevel 66 echo FAIL: wrong code not_number_case & exit /b 1
findstr /C:" 1." build\err.txt > nul
if errorlevel 1 echo FAIL: no string number in not_number_case & exit /b 1
findstr /C:"2 3 bam" build\err.txt > nul
if errorlevel 1 echo FAIL: no input string in not_number_case & exit /b 1


:: недостаточное кол-во аргументов
echo 4 5 | t3_bbox_c\main.exe > nul 2> build\err.txt
if not errorlevel 65 echo FAIL: wrong code too_few_args_case & exit /b 1
if errorlevel 66 echo FAIL: wrong code too_few_args_case & exit /b 1
findstr /C:" 1." build\err.txt > nul
if errorlevel 1 echo FAIL: no string number in too_few_args_case & exit /b 1
findstr /C:"4 5" build\err.txt > nul
if errorlevel 1 echo FAIL: no input string in too_few_args_case & exit /b 1

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