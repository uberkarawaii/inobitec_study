:: проверка того что make и cmake выдают идентичный результат 
:: и программы ведут себя одинаково
@echo off
chcp 1251 >nul

set t1_cpp_make=build\debug\t1_dist_matrix_cpp\main.exe
set t1_c_make=build\debug\t1_dist_matrix_c\main.exe
set t1_cpp_cmake=build\cmake_debug\t1_dist_matrix_cpp\main.exe
set t1_c_cmake=build\cmake_debug\t1_dist_matrix_c\main.exe

:: test-программы
set run_case=build\debug\tools\run_case.exe
set check=build\debug\tools\check.exe

:: запуски
%t1_cpp_cmake% < tests/input_data/t1_abc > nul 2> nul
set last_errlevel=%errorlevel%
%run_case% "%t1_cpp_make% < tests/input_data/t1_abc > nul 2> nul" %last_errlevel%
if errorlevel 1 echo t1_cpp abc уровни ошибки разные & exit /b 1

%t1_cpp_cmake% < tests/input_data/t1_float > nul 2> nul
set last_errlevel=%errorlevel%
%run_case% "%t1_cpp_make% < tests/input_data/t1_float > nul 2> nul" %last_errlevel%
if errorlevel 1 echo t1_cpp float уровни ошибки разные & exit /b 1

%t1_cpp_cmake% < tests/input_data/t1_low > nul 2> nul
set last_errlevel=%errorlevel%
%run_case% "%t1_cpp_make% < tests/input_data/t1_low > nul 2> nul" %last_errlevel%
if errorlevel 1 echo t1_cpp low уровни ошибки разные & exit /b 1

%t1_cpp_cmake% < tests/input_data/t1_high > nul 2> nul
set last_errlevel=%errorlevel%
%run_case% "%t1_cpp_make% < tests/input_data/t1_high > nul 2> nul" %last_errlevel%
if errorlevel 1 echo t1_cpp high уровни ошибки разные & exit /b 1

%t1_cpp_cmake% < tests/input_data/t1_nul > nul 2> nul
set last_errlevel=%errorlevel%
%run_case% "%t1_cpp_make% < tests/input_data/t1_nul > nul 2> nul" %last_errlevel%
if errorlevel 1 echo t1_cpp nul уровни ошибки разные & exit /b 1

if not exist build mkdir build
if not exist build\temp mkdir build\temp
%t1_cpp_cmake% < tests/input_data/t1_test1 > build/temp/t1_cpp_cmake_test1 2> nul
set last_errlevel=%errorlevel%
%run_case% "%t1_cpp_make% < tests/input_data/t1_test1 > build/temp/t1_cpp_make_test1 2> nul" %last_errlevel%
if errorlevel 1 echo t1_cpp test1 уровни ошибки разные & exit /b 1
%check% --equal build/temp/t1_cpp_cmake_test1 build/temp/t1_cpp_make_test1
if errorlevel 1 echo t1_cpp test1 cmake и make - разные выходные данные & exit /b 1 

echo t1_cpp уровни сошлись

%t1_c_cmake% < tests/input_data/t1_abc > nul 2> nul 
set last_errlevel=%errorlevel%
%run_case% "%t1_c_make% < tests/input_data/t1_abc > nul 2> nul" %last_errlevel%
if errorlevel 1 echo t1_c abc уровни ошибки разные & exit /b 1
%t1_c_cmake% < tests/input_data/t1_float > nul 2> nul
set last_errlevel=%errorlevel%
%run_case% "%t1_c_make% < tests/input_data/t1_float > nul 2> nul" %last_errlevel%
if errorlevel 1 echo t1_c float уровни ошибки разные & exit /b 1

%t1_c_cmake% < tests/input_data/t1_low > nul 2> nul
set last_errlevel=%errorlevel%
%run_case% "%t1_c_make% < tests/input_data/t1_low > nul 2> nul" %last_errlevel%
if errorlevel 1 echo t1_c low уровни ошибки разные & exit /b 1

%t1_c_cmake% < tests/input_data/t1_high > nul 2> nul
set last_errlevel=%errorlevel%
%run_case% "%t1_c_make% < tests/input_data/t1_high > nul 2> nul" %last_errlevel%
if errorlevel 1 echo t1_c high уровни ошибки разные & exit /b 1

%t1_c_cmake% < tests/input_data/t1_nul > nul 2> nul
set last_errlevel=%errorlevel%
%run_case% "%t1_c_make% < tests/input_data/t1_nul > nul 2> nul" %last_errlevel%
if errorlevel 1 echo t1_c nul уровни ошибки разные & exit /b 1

if not exist build mkdir build
if not exist build\temp mkdir build\temp
%t1_c_cmake% < tests/input_data/t1_test1 > build/temp/t1_c_cmake_test1 2> nul
set last_errlevel=%errorlevel%
%run_case% "%t1_c_make% < tests/input_data/t1_test1 > build/temp/t1_c_make_test1 2> nul" %last_errlevel%
if errorlevel 1 echo t1_c test1 уровни ошибки разные & exit /b 1
%check% --equal build/temp/t1_c_cmake_test1 build/temp/t1_c_make_test1
if errorlevel 1 echo t1_c test1 cmake и make - разные выходные данные & exit /b 1

echo t1_c уровни сошлись
