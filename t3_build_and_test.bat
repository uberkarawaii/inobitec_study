:: выключить вывод bat команд + русская кодировка
@echo off
chcp 1251 > nul

:: формат 
clang-format -i t3_bbox_cpp\main.cpp gen_cloud\main.cpp
clang-format -i common\string_utils.cpp common\string_utils.hpp
clang-format -i common\geometry.cpp common\geometry.hpp

:: папка build и её подпапки для артефактов
if not exist build mkdir build
if not exist build\common mkdir build\common
if not exist build\t3_bbox_cpp mkdir build\t3_bbox_cpp
if not exist build\t3_bbox_c mkdir build\t3_bbox_c

:: назначение переменной PATH - чтобы потом main.exe искала .dll по этому пути
set "PATH=%~dp0/build/common;%PATH%"

:: сборка (с define COMMON_EXPORTS) и линковка динамической библиотеки 
:: при link-е .dll уходит в build чтобы была перезапись, а не копирование. и PATH уже есть для этого
cl -c /DCOMMON_EXPORTS /Fo:build\common\string_utils_cpp.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address common\string_utils.cpp
if errorlevel 1 echo FAIL: string_utils_cpp_compilation_error & exit /b 1

cl -c /DCOMMON_EXPORTS /Fo:build\common\geometry_cpp.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address common\geometry.cpp
if errorlevel 1 echo FAIL: geometry_cpp_compilation_error & exit /b 1
:: вместе с .dll в build уйдут и .exp и .lib, а также и .pdb
link /DEBUG /DLL /OUT:build\common\common_cpp.dll build\common\string_utils_cpp.obj build\common\geometry_cpp.obj
if errorlevel 1 echo FAIL: dll_cpp_link_error & exit /b 1

:: сборка main и линковка с common.lib 
cl -c /Fo:build\t3_bbox_cpp\main.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address t3_bbox_cpp\main.cpp
if errorlevel 1 echo FAIL: main_cpp_compilation_error & exit /b 1

link /DEBUG /OUT:build\t3_bbox_cpp\main.exe build\t3_bbox_cpp\main.obj build\common\common_cpp.lib
if errorlevel 1 echo FAIL: main_cpp_link_error & exit /b 1

:: переменные для смены кодировки, чтобы findstr находила то, что ей передают
:: а консоль передаёт ей в OEM 
set "DEFtoOEM=powershell -Command "Set-Content -Path build\t3_bbox_cpp\err.txt -Encoding OEM -Value (Get-Content build\t3_bbox_cpp\err.txt -Encoding Default -Raw)""
:: в конце применятся эта команда, чтобы в err.txt был читаемый текст
set "OEMtoDEF=powershell -Command "Set-Content -Path build\t3_bbox_cpp\err.txt -Encoding Default -Value (Get-Content build\t3_bbox_cpp\err.txt -Encoding OEM -Raw)""

:: и для main
set "DEFtoOEMmain=powershell -Command "Set-Content -Path build\t3_bbox_cpp\main_out.txt -Encoding OEM -Value (Get-Content build\t3_bbox_cpp\main_out.txt -Encoding Default -Raw)""
set "OEMtoDEFmain=powershell -Command "Set-Content -Path build\t3_bbox_cpp\main_out.txt -Encoding Default -Value (Get-Content build\t3_bbox_cpp\main_out.txt -Encoding OEM -Raw)""


:: тесты с main и кодами ошибок
echo CPP TESTS
:: пустой ввод
type nul | build\t3_bbox_cpp\main.exe > nul 2> build\t3_bbox_cpp\err.txt
if not errorlevel 66 echo FAIL: empty_case & exit /b 1
%DEFtoOEM%
findstr /C:"Входные данные отсутствуют" build\t3_bbox_cpp\err.txt > nul
if errorlevel 1 echo FAIL: no info in cerr about empty input & %OEMtoDEF% & exit /b 1          

:: не число во входном потоке
echo 3 2 bam | build\t3_bbox_cpp\main.exe > nul 2> build\t3_bbox_cpp\err.txt 
:: провекрка что выходной код именно 65 - не более и не менее
if errorlevel 66 echo FAIL: not_digit_case & exit /b 1 
if not errorlevel 65 echo FAIL: not_digit_case & exit /b 1 
:: поиск вывода ошибочной строки с номером
%DEFtoOEM%
findstr /C:"Строка 1. Нечисловые данные: 3 2 bam" build\t3_bbox_cpp\err.txt > nul
if errorlevel 1 echo FAIL: not_digit_case: not found info in cerr & %OEMtoDEF% & exit /b 1 

:: неполный набор аргументов X Y Z
echo 2 3 | build\t3_bbox_cpp\main.exe > nul 2> build\t3_bbox_cpp\err.txt
:: проверка что код == 65
if not errorlevel 65 echo FAIL: too few args case & exit /b 1
if errorlevel 66 echo FAIL: too few args case & exit /b 1
:: поиск вывода ошибочной строки с номером
%DEFtoOEM%
findstr /C:"Строка 1. Ожидались координаты X Y Z. Получено: 2 3" build\t3_bbox_cpp\err.txt > nul
if errorlevel 1 echo FAIL: too few args case: not found info in cerr & %OEMtoDEF% & exit /b 1

:: если всё удачно, пересохранить err.txt в 1251
%OEMtoDEF%

:: тесты на визуально понятных данных
:: одна точка на вход - она и будет центроидом
echo 1.5 2.5 3.5 | build\t3_bbox_cpp\main.exe > build\t3_bbox_cpp\main_out.txt
if errorlevel 1 echo FAIL: test1 main exit code & exit /b 1
%DEFtoOEMmain%
findstr /C:"Количество точек: 1" build\t3_bbox_cpp\main_out.txt > nul 
if errorlevel 1 echo FAIL: test1_wrong_quantity & %OEMtoDEFmain% & exit /b 1
findstr /C:"Координаты огранич. паралеллепипеда x: [ 1.500; 1.500 ] y: [ 2.500; 2.500 ] z: [ 3.500; 3.500 ]" build\t3_bbox_cpp\main_out.txt > nul
if errorlevel 1 echo FAIL: test1_wrong_boundaries & %OEMtoDEFmain% & exit /b 1
findstr /C:"Координаты центроида x: 1.500, y: 2.500, z: 3.500" build\t3_bbox_cpp\main_out.txt > nul
if errorlevel 1 echo FAIL: test1_wrong_center_coords & %OEMtoDEFmain% & exit /b 1
findstr /C:"Среднее расстояние от точек до центроида: 0.000" build\t3_bbox_cpp\main_out.txt > nul
if errorlevel 1 echo FAIL: test1_wrong_avg_dist & %OEMtoDEFmain% & exit /b 1


(echo 0 0 0 & echo 1 0 0 & echo 0 1 0) | build\t3_bbox_cpp\main.exe > build\t3_bbox_cpp\main_out.txt
if errorlevel 1 echo FAIL: test2 main exit code & exit /b 1
%DEFtoOEMmain%
findstr /C:"Количество точек: 3" build\t3_bbox_cpp\main_out.txt > nul 
if errorlevel 1 echo FAIL: test2_wrong_quantity & %OEMtoDEFmain% & exit /b 1
findstr /C:"Координаты огранич. паралеллепипеда x: [ 0.000; 1.000 ] y: [ 0.000; 1.000 ] z: [ 0.000; 0.000 ]" build\t3_bbox_cpp\main_out.txt > nul
if errorlevel 1 echo FAIL: test2_wrong_boundaries & %OEMtoDEFmain% & exit /b 1
findstr /C:"Координаты центроида x: 0.333, y: 0.333, z: 0.000" build\t3_bbox_cpp\main_out.txt > nul
if errorlevel 1 echo FAIL: test2_wrong_center_coords & %OEMtoDEFmain% & exit /b 1
findstr /C:"Среднее расстояние от точек до центроида: 0.654" build\t3_bbox_cpp\main_out.txt > nul
if errorlevel 1 echo FAIL: test2_wrong_avg_dist & %OEMtoDEFmain% & exit /b 1

:: при сложениях точек по итогу сумма будет = 0
(echo 0 -16 0 & echo 0 16 0 & echo 16 0 0 & echo -16 0 0) | build\t3_bbox_cpp\main.exe > build\t3_bbox_cpp\main_out.txt
if errorlevel 1 echo FAIL: test3 main exit code & exit /b 1
%DEFtoOEMmain%
findstr /C:"Количество точек: 4" build\t3_bbox_cpp\main_out.txt > nul 
if errorlevel 1 echo FAIL: test3_wrong_quantity & %OEMtoDEFmain% & exit /b 1
findstr /C:"Координаты огранич. паралеллепипеда x: [ -16.000; 16.000 ] y: [ -16.000; 16.000 ] z: [ 0.000; 0.000 ]" build\t3_bbox_cpp\main_out.txt > nul
if errorlevel 1 echo FAIL: test3_wrong_boundaries & %OEMtoDEFmain% & exit /b 1
findstr /C:"Координаты центроида x: 0.000, y: 0.000, z: 0.000" build\t3_bbox_cpp\main_out.txt > nul
if errorlevel 1 echo FAIL: test3_wrong_center_coords & %OEMtoDEFmain% & exit /b 1
findstr /C:"Среднее расстояние от точек до центроида: 16.000" build\t3_bbox_cpp\main_out.txt > nul
if errorlevel 1 echo FAIL: test3_wrong_avg_dist & %OEMtoDEFmain% & exit /b 1
%OEMtoDEFmain%

ECHO ALL CPP TESTS PASSED

:: C тесты
:: форматирование 
clang-format -i t3_bbox_c\main.c common\exit_codes.h
clang-format -i common\geometry.c common\geometry.h
clang-format -i common\string_utils.c common\string_utils.h

:: переназначение переменных кодировки, т.к. теперь все артефакты в build
set "DEFtoOEM=powershell -Command "Set-Content -Path build\t3_bbox_c\err.txt -Encoding OEM -Value (Get-Content build\t3_bbox_c\err.txt -Encoding Default -Raw)""
set "OEMtoDEF=powershell -Command "Set-Content -Path build\t3_bbox_c\err.txt -Encoding Default -Value (Get-Content build\t3_bbox_c\err.txt -Encoding OEM -Raw)""

set "DEFtoOEMmain=powershell -Command "Set-Content -Path build\t3_bbox_c\main_out.txt -Encoding OEM -Value (Get-Content build\t3_bbox_c\main_out.txt -Encoding Default -Raw)""
set "OEMtoDEFmain=powershell -Command "Set-Content -Path build\t3_bbox_c\main_out.txt -Encoding Default -Value (Get-Content build\t3_bbox_c\main_out.txt -Encoding OEM -Raw)""


:: сборка и линковка динамич. библиотеки с define COMMON_EXPORTS
:: итоговый файл уходит в build, и main.exe найдёт его, т к PATH уже настроен для этого
cl -c /DCOMMON_EXPORTS /Fo:build\common\string_utils_c.obj /std:c17 /W4 /permissive- /Od /Zi /MDd /fsanitize=address common\string_utils.c
if errorlevel 1 echo FAIL: string_utils_c_compilation_error & exit /b 1

cl -c /DCOMMON_EXPORTS /Fo:build\common\geometry_c.obj /std:c17 /W4 /permissive- /Od /Zi /MDd /fsanitize=address common\geometry.c
if errorlevel 1 echo FAIL: geometry_c_compilation_error & exit /b 1

link /DEBUG /DLL /OUT:build\common\common_c.dll build\common\string_utils_c.obj build\common\geometry_c.obj
if errorlevel 1 echo FAIL: dll_c_link_error & exit /b 1

:: сборка и линковка main с common.lib 
cl -c /Fo:build\t3_bbox_c\main.obj /std:c17 /W4 /permissive- /Od /Zi /MDd /fsanitize=address t3_bbox_c\main.c
if errorlevel 1 echo FAIL: main_c_compilation_error & exit /b 1

link /DEBUG /OUT:build\t3_bbox_c\main.exe build\t3_bbox_c\main.obj build\common\common_c.lib
if errorlevel 1 echo FAIL: main_c_link_error & exit /b 1
                                                            
:: тесты с кодами ошибок
echo C TESTS

type nul | build\t3_bbox_c\main.exe > nul 2> build\t3_bbox_c\err.txt
if not errorlevel 66 echo FAIL: empty_case & exit /b 1
%DEFtoOEM%
findstr /C:"Входные данные отсутствуют" build\t3_bbox_c\err.txt > nul
if errorlevel 1 echo FAIL: no info in cerr about empty input & %OEMtoDEF% & exit /b 1          

:: не число в потоке
echo 3 2 bam | build\t3_bbox_c\main.exe > nul 2> build\t3_bbox_c\err.txt 
if errorlevel 66 echo FAIL: not_digit_case & exit /b 1 
if not errorlevel 65 echo FAIL: not_digit_case & exit /b 1 
%DEFtoOEM%
findstr /C:"Строка 1. Нечисловые данные: 3 2 bam" build\t3_bbox_c\err.txt > nul
if errorlevel 1 echo FAIL: not_digit_case: not found info in cerr & %OEMtoDEF% & exit /b 1

:: недостаточное кол-во аргументов
echo 2 3 | build\t3_bbox_c\main.exe > nul 2> build\t3_bbox_c\err.txt
if not errorlevel 65 echo FAIL: wrong code too_few_args_case & exit /b 1
if errorlevel 66 echo FAIL: wrong code too_few_args_case & exit /b 1
%DEFtoOEM%
findstr /C:"Строка 1. Ожидались координаты X Y Z. Получено: 2 3 " build\t3_bbox_c\err.txt > nul
if errorlevel 1 echo FAIL: too_few_args_case: not found info in cerr & %OEMtoDEF% & exit /b 1

:: если всё удачно, пересохранить err.txt в 1251
%OEMtoDEF%

:: тесты на визуально понятных данных

echo 1.5 2.5 3.5 | build\t3_bbox_c\main.exe > build\t3_bbox_c\main_out.txt
if errorlevel 1 echo FAIL: test1 main exit code & exit /b 1
%DEFtoOEMmain%
findstr /C:"Количество точек: 1" build\t3_bbox_c\main_out.txt > nul 
if errorlevel 1 echo FAIL: test1_wrong_quantity & %OEMtoDEFmain% & exit /b 1
findstr /C:"Координаты огранич. паралеллепипеда x: [ 1.500; 1.500 ] y: [ 2.500; 2.500 ] z: [ 3.500; 3.500 ]" build\t3_bbox_c\main_out.txt > nul
if errorlevel 1 echo FAIL: test1_wrong_boundaries & %OEMtoDEFmain% & exit /b 1
findstr /C:"Координаты центроида x: 1.500, y: 2.500, z: 3.500" build\t3_bbox_c\main_out.txt > nul
if errorlevel 1 echo FAIL: test1_wrong_center_coords & %OEMtoDEFmain% & exit /b 1
findstr /C:"Среднее расстояние от точек до центроида: 0.000" build\t3_bbox_c\main_out.txt > nul
if errorlevel 1 echo FAIL: test1_wrong_avg_dist & %OEMtoDEFmain% & exit /b 1


(echo 0 0 0 & echo 1 0 0 & echo 0 1 0) | build\t3_bbox_c\main.exe > build\t3_bbox_c\main_out.txt
if errorlevel 1 echo FAIL: test2 main exit code & exit /b 1
%DEFtoOEMmain%
findstr /C:"Количество точек: 3" build\t3_bbox_c\main_out.txt > nul 
if errorlevel 1 echo FAIL: test2_wrong_quantity & %OEMtoDEFmain% & exit /b 1
findstr /C:"Координаты огранич. паралеллепипеда x: [ 0.000; 1.000 ] y: [ 0.000; 1.000 ] z: [ 0.000; 0.000 ]" build\t3_bbox_c\main_out.txt > nul
if errorlevel 1 echo FAIL: test2_wrong_boundaries & %OEMtoDEFmain% & exit /b 1
findstr /C:"Координаты центроида x: 0.333, y: 0.333, z: 0.000" build\t3_bbox_c\main_out.txt > nul
if errorlevel 1 echo FAIL: test2_wrong_center_coords & %OEMtoDEFmain% & exit /b 1
findstr /C:"Среднее расстояние от точек до центроида: 0.654" build\t3_bbox_c\main_out.txt > nul
if errorlevel 1 echo FAIL: test2_wrong_avg_dist & %OEMtoDEFmain% & exit /b 1


(echo 0 -16 0 & echo 0 16 0 & echo 16 0 0 & echo -16 0 0) | build\t3_bbox_c\main.exe > build\t3_bbox_c\main_out.txt
if errorlevel 1 echo FAIL: test3 main exit code & exit /b 1
%DEFtoOEMmain%
findstr /C:"Количество точек: 4" build\t3_bbox_c\main_out.txt > nul 
if errorlevel 1 echo FAIL: test3_wrong_quantity & %OEMtoDEFmain% & exit /b 1
findstr /C:"Координаты огранич. паралеллепипеда x: [ -16.000; 16.000 ] y: [ -16.000; 16.000 ] z: [ 0.000; 0.000 ]" build\t3_bbox_c\main_out.txt > nul
if errorlevel 1 echo FAIL: test3_wrong_boundaries & %OEMtoDEFmain% & exit /b 1
findstr /C:"Координаты центроида x: 0.000, y: 0.000, z: 0.000" build\t3_bbox_c\main_out.txt > nul
if errorlevel 1 echo FAIL: test3_wrong_center_coords & %OEMtoDEFmain% & exit /b 1
findstr /C:"Среднее расстояние от точек до центроида: 16.000" build\t3_bbox_c\main_out.txt > nul
if errorlevel 1 echo FAIL: test3_wrong_avg_dist & %OEMtoDEFmain% & exit /b 1
%OEMtoDEFmain%

ECHO ALL C TESTS PASSED 
exit /b 0