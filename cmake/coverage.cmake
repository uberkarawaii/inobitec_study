# cmake -DCOVERAGE_EXE=... -DCTEST_EXE=... -DBUILD_DIR=... -DSOURCE_DIR=... \
#       -DSETTINGS_FILE=... -DREPORT_DIR=... -P cmake/coverage.cmake

foreach(v COVERAGE_EXE CTEST_EXE BUILD_DIR SOURCE_DIR SETTINGS_FILE REPORT_DIR)
  if(NOT DEFINED ${v} OR "${${v}}" STREQUAL "")
    message(FATAL_ERROR "${v} is not set")
  endif()
endforeach()

file(MAKE_DIRECTORY "${REPORT_DIR}")
# старые отчёты удаляются в начале каждого прогона
file(REMOVE "${REPORT_DIR}/coverage.coverage"
            "${REPORT_DIR}/coverage.cobertura.xml"
            "${REPORT_DIR}/coverage.xml")

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

# печать метрики с line-rate из .cobertura.xml
file(READ "${_cobertura_xml}" _cobertura_xml_text)
string(REGEX MATCH "line-rate=\"([0-9.]+)\"" _line_rate "${_cobertura_xml_text}")
if(CMAKE_MATCH_1)
  message(STATUS "coverage line rate: ${CMAKE_MATCH_1}")
endif()
# печать метрики block_coverage из .xml
# TODO сделать при появлении агрегатора

message(STATUS "coverage report (cobertura): ${_cobertura_xml}")
message(STATUS "coverage report (native xml): ${_xml}")

# или тесты красные, или инструментация удалась не полностью (некоторые процессы остались непокрыты). но отчёты есть
if(NOT _collect_rc EQUAL 0)
  message(FATAL_ERROR "coverage collection exited with code ${_collect_rc}; reports were still written to:\n"
                       "${_cobertura_xml}\n"
                       "${_xml}")
endif()
