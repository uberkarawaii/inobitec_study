:: отключение вывода команд на экран
@echo off
chcp 1251 > nul

:: форматирование 
clang-format -i t2_passport_cpp\main.cpp t2_passport_cpp\ref.cpp common\string_utils.hpp

:: сборка
cl -c /Fo:t2_passport_cpp\main.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address t2_passport_cpp\main.cpp
link /DEBUG /OUT:t2_passport_cpp\main.exe t2_passport_cpp\main.obj

cl -c /Fo:t2_passport_cpp\ref.obj /std:c++latest /W4 /permissive- /EHsc /Od /Zi /MDd /fsanitize=address t2_passport_cpp\ref.cpp
link /DEBUG /OUT:t2_passport_cpp\ref.exe t2_passport_cpp\ref.obj

:: тесты 
(echo звезда & echo 6) | t2_passport_cpp\main.exe > t2_passport_cpp\main_out.txt
(echo звезда & echo 6) | t2_passport_cpp\ref.exe > t2_passport_cpp\ref_out.txt
fc t2_passport_cpp\main_out.txt t2_passport_cpp\ref_out.txt > nul
if errorlevel 1 echo FAIL: norm-case & exit /b 1

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

echo ALL TESTS PASSED