# cmake -DCOVERAGE_EXE=... -DCTEST_EXE=... -DBUILD_DIR=... -DSOURCE_DIR=... \
#       -DSETTINGS_FILE=... -DREPORT_DIR=... -DEXPECTED_MODULES=... -P cmake/coverage.cmake
foreach(v COVERAGE_BACKEND CTEST_EXE BUILD_DIR SOURCE_DIR REPORT_DIR
          COVERAGE_UNION_EXE EXPECTED_SOURCES)
  # если хотя бы одна из обязат.переменных не определена или пустая - fatal error
  if(NOT DEFINED ${v} OR "${${v}}" STREQUAL "")
    message(FATAL_ERROR "${v} is not set")
  endif()
endforeach()

if(NOT COVERAGE_BACKEND STREQUAL "msvc" AND NOT COVERAGE_BACKEND STREQUAL "gcovr")
  message(FATAL_ERROR "COVERAGE_BACKEND must be 'msvc' or 'gcovr'; got '${COVERAGE_BACKEND}'")
endif()

# проверка существования компонентов в зависимости от платформы
if(COVERAGE_BACKEND STREQUAL "msvc")
  foreach(v COVERAGE_EXE SETTINGS_FILE EXPECTED_MODULES)
    if(NOT DEFINED ${v} OR "${${v}}" STREQUAL "")
      message(FATAL_ERROR "${v} is not set for COVERAGE_BACKEND=msvc")
    endif()
  endforeach()
else()
  if(NOT DEFINED GCOVR_EXE OR "${GCOVR_EXE}" STREQUAL "")
    message(FATAL_ERROR "GCOVR_EXE is not set for COVERAGE_BACKEND=gcovr")
  endif()
endif()

# RESULT_VARIABLE: число при запуске, строка ошибки - если процесс не удалось запустить
function(describe_process_result result out_desc out_ok)
  if("${result}" MATCHES "^-?[0-9]+$")
    set(${out_desc} "exited with code ${result}" PARENT_SCOPE)
    if("${result}" EQUAL 0)
      set(${out_ok} TRUE PARENT_SCOPE)
    else()
      set(${out_ok} FALSE PARENT_SCOPE)
    endif()
  else()
    set(${out_desc} "could not be started: ${result}" PARENT_SCOPE)
    set(${out_ok} FALSE PARENT_SCOPE)
  endif()
endfunction()

file(MAKE_DIRECTORY "${REPORT_DIR}")
# старые отчёты удаляются в начале каждого прогона
file(REMOVE "${REPORT_DIR}/coverage.coverage"
            "${REPORT_DIR}/coverage.cobertura.xml"
            "${REPORT_DIR}/coverage.xml"
            "${REPORT_DIR}/coverage_summary.txt")

set(_cobertura_xml "${REPORT_DIR}/coverage.cobertura.xml")
set(_summary "${REPORT_DIR}/coverage_summary.txt")
set(_raw "${REPORT_DIR}/coverage.coverage")
set(_xml "${REPORT_DIR}/coverage.xml")

# WINDOWS COVERAGE + .cobertura.xml + native xml
if(COVERAGE_BACKEND STREQUAL "msvc")
  # .coverage - raw отчёт
  execute_process(
    COMMAND "${COVERAGE_EXE}" collect
          --nologo
          --settings "${SETTINGS_FILE}"
          -o "${_raw}"
          -f coverage
          -- "${CTEST_EXE}" --test-dir "${BUILD_DIR}" --output-on-failure --no-tests=error
             -E "format-check|e2e/runner/"
    WORKING_DIRECTORY "${SOURCE_DIR}"
    RESULT_VARIABLE _collect_rc)

  if(NOT EXISTS "${_raw}")
    message(FATAL_ERROR "coverage file was not created: ${_raw}")
  endif()

  # .coverage -> .cobertura.xml
  execute_process(
    COMMAND "${COVERAGE_EXE}" merge --nologo
          -o "${_cobertura_xml}" -f cobertura "${_raw}"
    RESULT_VARIABLE _merge_rc)

  # вызов хелпера для обработки - это код или строка
  describe_process_result("${_merge_rc}" _merge_desc _merge_ok)
  if(NOT _merge_ok)
    message(FATAL_ERROR "coverage merge (cobertura) ${_merge_desc}")
  endif()

  if(NOT EXISTS "${_cobertura_xml}")
    message(FATAL_ERROR "cobertura report was not created: ${_cobertura_xml}")
  endif()

  # .coverage -> .xml
  execute_process(
    COMMAND "${COVERAGE_EXE}" merge --nologo
          -o "${_xml}" -f xml "${_raw}"
    RESULT_VARIABLE _merge_xml)

  # вызов хелпера для обработки - это код или строка
  describe_process_result("${_merge_xml}" _merge_xml_desc _merge_xml_desc_ok)
  if(NOT _merge_xml_desc_ok)
    message(FATAL_ERROR "coverage merge (xml) ${_merge_xml_desc}")
  endif()

  if(NOT EXISTS "${_xml}")
    message(FATAL_ERROR "xml report was not created: ${_xml}")
  endif()

  message(STATUS "coverage report (cobertura): ${_cobertura_xml}")
  message(STATUS "coverage report (native xml): ${_xml}")

# LINUX COVERAGE + cobertura.xml
else() # gcovr
  # удаление старого stale .gcda: после смены .gcno старые счётчики могут дать mismatch
  file(GLOB_RECURSE _gcda_files "${BUILD_DIR}/*.gcda")
  if(_gcda_files)
    file(REMOVE ${_gcda_files})
  endif()

  # тесты; rc уйдёт в _collect_rc и будет обработан в конце
  execute_process(
    COMMAND "${CTEST_EXE}" --test-dir "${BUILD_DIR}" --output-on-failure --no-tests=error
            -E "format-check|e2e/runner/"
    WORKING_DIRECTORY "${SOURCE_DIR}"
    RESULT_VARIABLE _collect_rc)

  # Cobertura; пути в отчёте будут относительны --root
  execute_process(
    COMMAND "${GCOVR_EXE}"
            --root "${SOURCE_DIR}"
            --cobertura "${_cobertura_xml}"
            --cobertura-pretty
            --filter "^common/(geometry|string_utils|cli)\\.(c|cpp)$"
            --filter "^t[1-4]_[^/]*/main\\.(c|cpp)$"
            "${BUILD_DIR}"
    WORKING_DIRECTORY "${SOURCE_DIR}"
    RESULT_VARIABLE _gcovr_rc)

  # вызов хелпера для обработки - это код или строка
  describe_process_result("${_gcovr_rc}" _gcovr_desc _gcovr_ok)
  if(NOT _gcovr_ok)
    message(FATAL_ERROR "gcovr ${_gcovr_desc}")
  endif()

  if(NOT EXISTS "${_cobertura_xml}")
    message(FATAL_ERROR "cobertura report was not created: ${_cobertura_xml}")
  endif()

  message(STATUS "coverage report (cobertura): ${_cobertura_xml}")
endif()
# вызов хелпера - это код завершения или строка
describe_process_result("${_collect_rc}" _collect_desc _collect_ok)

# переменная-путь до отчёта 
set(_summary "${REPORT_DIR}/coverage_summary.txt")
# построение человекочитаемого отчёта по .cobertura.xml, ex-code процесса - _summary_rc
if(COVERAGE_BACKEND STREQUAL "msvc")
  execute_process(
    COMMAND "${COVERAGE_UNION_EXE}" --cobertura "${_cobertura_xml}"
            --native "${_xml}"
            --modules "${EXPECTED_MODULES}"
            --source-root "${SOURCE_DIR}" --expected "${EXPECTED_SOURCES}"
            --output "${_summary}"
    RESULT_VARIABLE "_summary_rc")
else()
  execute_process(
    COMMAND "${COVERAGE_UNION_EXE}" --cobertura "${_cobertura_xml}"
            --source-root "${SOURCE_DIR}" --expected "${EXPECTED_SOURCES}"
            --output "${_summary}"
    RESULT_VARIABLE "_summary_rc")
endif()
# вызов хелпера - это код завершения или строка
describe_process_result("${_summary_rc}" _summary_desc _summary_ok)

# накапливаем ошибки в _failures и выводим их в конце (если хоть одна случилась)
set(_failures "")
# проверяем код завершения и если он != 0, то накапливаем информацию об этом
if(NOT _summary_ok)
  string(APPEND _failures "coverage union report ${_summary_desc}\n")
endif()

# проверяем, что файл отчёта существует
if(EXISTS "${_summary}")
  message(STATUS "coverage union report: ${_summary}")
else()
  string(APPEND _failures "coverage union report was not created: ${_summary}\n")
endif()

# или тесты красные, или инструментация удалась не полностью (некоторые процессы остались непокрыты)
if(NOT _collect_ok)
  if(COVERAGE_BACKEND STREQUAL "msvc")
    string(APPEND _failures "coverage collection ${_collect_desc}\n")
  else()
    string(APPEND _failures "tests ${_collect_desc}\n")
  endif()
endif()

# если есть создание отчёта с ненулевым кодом и/или не создан отчёт и/или ненулевой _collect_rc
if(_failures)
  set(_written "${_cobertura_xml}")
  # условно для native xml, т.к. под линуксом его нет
  if(DEFINED _xml AND EXISTS "${_xml}")
    string(APPEND _written "\n${_xml}")
  endif()
  if(EXISTS "${_summary}")
    string(APPEND _written "\n${_summary}")
  endif()
  message(FATAL_ERROR "${_failures}reports were still written to:\n${_written}")
endif()
