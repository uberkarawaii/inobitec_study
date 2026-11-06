# cmake -DCOVERAGE_EXE=... -DCTEST_EXE=... -DBUILD_DIR=... -DSOURCE_DIR=... \
#       -DSETTINGS_FILE=... -DREPORT_DIR=... -DEXPECTED_MODULES=... -P cmake/coverage.cmake

foreach(v COVERAGE_EXE CTEST_EXE BUILD_DIR SOURCE_DIR SETTINGS_FILE 
          REPORT_DIR COVERAGE_UNION_EXE EXPECTED_SOURCES EXPECTED_MODULES)
  if(NOT DEFINED ${v} OR "${${v}}" STREQUAL "")
    message(FATAL_ERROR "${v} is not set")
  endif()
endforeach()

file(MAKE_DIRECTORY "${REPORT_DIR}")
# старые отчёты удаляются в начале каждого прогона
file(REMOVE "${REPORT_DIR}/coverage.coverage"
            "${REPORT_DIR}/coverage.cobertura.xml"
            "${REPORT_DIR}/coverage.xml"
            "${REPORT_DIR}/coverage_summary.txt")

set(_raw "${REPORT_DIR}/coverage.coverage")
set(_cobertura_xml "${REPORT_DIR}/coverage.cobertura.xml")
set(_xml "${REPORT_DIR}/coverage.xml")

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
if(NOT _merge_rc EQUAL 0)
  message(FATAL_ERROR "coverage merge (cobertura) failed (exit code ${_merge_rc})")
endif()

if(NOT EXISTS "${_cobertura_xml}")
  message(FATAL_ERROR "cobertura report was not created: ${_cobertura_xml}")
endif()

# .coverage -> .xml
execute_process(
  COMMAND "${COVERAGE_EXE}" merge --nologo
          -o "${_xml}" -f xml "${_raw}"
  RESULT_VARIABLE _merge_xml)
if(NOT _merge_xml EQUAL 0)
  message(FATAL_ERROR "coverage merge (xml) failed (exit code ${_merge_xml})")
endif()

if(NOT EXISTS "${_xml}")
  message(FATAL_ERROR "xml report was not created: ${_xml}")
endif()

message(STATUS "coverage report (cobertura): ${_cobertura_xml}")
message(STATUS "coverage report (native xml): ${_xml}")

# переменная-путь до отчёта 
set(_summary "${REPORT_DIR}/coverage_summary.txt")
# построение человекочитаемого отчёта по .cobertura.xml, ex-code процесса - _summary_rc
execute_process(COMMAND "${COVERAGE_UNION_EXE}" --cobertura "${_cobertura_xml}"
                --native "${_xml}"
                --modules "${EXPECTED_MODULES}"
                --source-root "${SOURCE_DIR}" --expected "${EXPECTED_SOURCES}"
                --output "${_summary}" RESULT_VARIABLE "_summary_rc")

# накапливаем ошибки в _failures и выводим их в конце (если хоть одна случилась)
set(_failures "")
# проверяем код завершения и если он != 0, то накапливаем информацию об этом
if(NOT _summary_rc EQUAL 0)
  string(APPEND _failures "coverage union report exited with code ${_summary_rc}\n")
endif()

# проверяем, что файл отчёта существует
if(EXISTS "${_summary}")
  message(STATUS "coverage union report: ${_summary}")
else()
  string(APPEND _failures "coverage union report was not created: ${_summary}\n")
endif()


# или тесты красные, или инструментация удалась не полностью (некоторые процессы остались непокрыты)
if(NOT _collect_rc EQUAL 0)
  string(APPEND _failures "coverage collection exited with code ${_collect_rc}\n")
endif()

# если есть создание отчёта с ненулевым кодом и/или не создан отчёт и/или ненулевой _collect_rc
if(_failures)
  set(_written "${_cobertura_xml}\n${_xml}")
  # если summary был создан, то прибавляем его к существующим отчётам
  if(EXISTS "${_summary}")
    string(APPEND _written "\n${_summary}")
  endif()
  message(FATAL_ERROR "${_failures}reports were still written to:\n${_written}")
endif()
