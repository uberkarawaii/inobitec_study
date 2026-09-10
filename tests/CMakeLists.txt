add_executable(run_case run_case.cpp)
configure_target(run_case OFF)

add_executable(check check.cpp)
configure_target(check OFF)

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/results")

# test_name - имя таргета теста
# exe - программа которая будет проверяться
# args - аргументы командной строки для exe
# input - входные данные для exe
# expected_excode - ожидаемый код завершения
# check_flag - для check (--equal / --contains)
# expected_result - ожидаемый вывод программы; НЕ абсолют. путь, а просто имя
# stream - поток, вывод которого будет сравниваться с expected_result
function(add_case test_name exe args input expected_excode check_flag expected_result stream)

# по этим аргументам будет развилка, так что проверка сразу. если что падает тут до первого теста
# разнесены по двум if т.к. это независимые проверки
# при mode=FATAL_ERROR остановка обработки и генерации. падение CMake c ненулевым кодом

if(NOT "${check_flag}" STREQUAL "CONTAINS" AND NOT "${check_flag}" STREQUAL "EQ")
  message(FATAL_ERROR "unknown flag for check. expected CONTAINS for containing and EQ for equality")
endif()

if(NOT ${stream} EQUAL 1 AND NOT ${stream} EQUAL 2)
  message(FATAL_ERROR "unknown stream type. expected 1 for stdout and 2 for stderr" )
endif()

# имя теста на код: test_name + _code
set(CODE_TEST "${test_name}_code")
# имя файла stdout
set(STDOUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/results/${test_name}.out")
# имя файла stderr
set(STDERR_FILE "${CMAKE_CURRENT_BINARY_DIR}/results/${test_name}.err")

# входное знаение м.б. или файлом, или папкой (для теста на IO fail)
# для относит. путей считается, что это файл и он в /input_data; абсолютные берутся как есть
if(IS_ABSOLUTE "${input}")
  set(INPUT_PATH "${input}")
else()
  set(INPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/input_data/${input}")
endif() 

# тест_1 на код всегда безусловен и одинаков по сигнатуре
add_test(NAME ${CODE_TEST} COMMAND $<TARGET_FILE:run_case> ${expected_excode}
 ${INPUT_PATH}
 ${STDOUT_FILE}
 ${STDERR_FILE}
 --
 $<TARGET_FILE:${exe}>
 ${args})
# args без кавычек чтобы не ломалось на пустоте и на "лишних" аргументах

# устновка имени выходного файла и теста в зависимоти от потока stdout(1)/stderr(2) 
if(${stream} EQUAL 1)
  set(PROGRAMM_RESULT ${STDOUT_FILE})
  set(RESULT_TEST "${test_name}_stdout")
elseif(${stream} EQUAL 2)
  set(PROGRAMM_RESULT ${STDERR_FILE})
  set(RESULT_TEST "${test_name}_stderr")
endif()

# установка флага для check - совпадение / содержание. зависит от check_flag
if("${check_flag}" STREQUAL "CONTAINS")
  set(CHECK_FLAG --contains)
elseif("${check_flag}" STREQUAL "EQ")
  set(CHECK_FLAG --equal)
endif()


# тест_2 на проверку файла или выхода, или ошибки
# тут учитывается папка expected_result, потому оно само - просто имя
add_test(NAME ${RESULT_TEST} COMMAND 
 $<TARGET_FILE:check> 
 ${CHECK_FLAG} 
 ${PROGRAMM_RESULT} 
 ${CMAKE_CURRENT_SOURCE_DIR}/expect/${expected_result})

# ${CMAKE_SOURCE_DIR} - изначальный корень с корневым  CMakeLists.txt
# ${CMAKE_CURRENT_SOURCE_DIR} - подпапка в корне с текущим CMakeLists.txt
# по аналогии и с ${CMAKE_BINARY_DIR}/${CMAKE_CURRENT_BINARY_DIR}

# фикстуры чтобы тесты шли в порядке: 1) тест на код 2) проверка выходного файла
# тест на код CODE_TEST порождает сущность test_name 
set_tests_properties(${CODE_TEST} PROPERTIES FIXTURES_SETUP ${test_name})
# тест с проверкой выходного файла RESULT_TEST нуждается в сущности test_name, 
# которую породит CODE_TEST. поэтому CODE_TEST, потом - RESULT_TEST  
set_tests_properties(${RESULT_TEST} PROPERTIES FIXTURES_REQUIRED ${test_name})
  
endfunction()

# в cases.cmake находятся все тест-кейсы
include(cases.cmake)
