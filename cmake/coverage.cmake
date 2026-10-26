# cmake -DCOVERAGE_EXE=... -DCTEST_EXE=... -DBUILD_DIR=... -DSOURCE_DIR=... \
#       -DSETTINGS_FILE=... -DREPORT_DIR=... -P cmake/coverage.cmake

foreach(v COVERAGE_EXE CTEST_EXE BUILD_DIR SOURCE_DIR SETTINGS_FILE REPORT_DIR)
  if(NOT DEFINED ${v} OR "${${v}}" STREQUAL "")
    message(FATAL_ERROR "${v} is not set")
  endif()
endforeach()

file(MAKE_DIRECTORY "${REPORT_DIR}")
file(MAKE_DIRECTORY "${REPORT_DIR}/logs")
file(REMOVE "${REPORT_DIR}/coverage.coverage"
            "${REPORT_DIR}/coverage.cobertura.xml")

set(_raw "${REPORT_DIR}/coverage.coverage")
set(_xml "${REPORT_DIR}/coverage.cobertura.xml")

execute_process(
  COMMAND "${COVERAGE_EXE}" collect
          --nologo
          --settings "${SETTINGS_FILE}"
          -o "${_raw}"
          -f coverage
          -l "${REPORT_DIR}/logs/"
          -- "${CTEST_EXE}" --test-dir "${BUILD_DIR}" --output-on-failure --no-tests=error
             -E "format-check|e2e/runner/"
  WORKING_DIRECTORY "${SOURCE_DIR}"
  RESULT_VARIABLE _collect_rc)

if(NOT _collect_rc EQUAL 0)
  message(FATAL_ERROR "coverage collection failed (exit code ${_collect_rc})")
endif()
if(NOT EXISTS "${_raw}")
  message(FATAL_ERROR "coverage file was not created: ${_raw}")
endif()

execute_process(
  COMMAND "${COVERAGE_EXE}" merge --nologo
          -o "${_xml}" -f cobertura "${_raw}"
  RESULT_VARIABLE _merge_rc)
if(NOT _merge_rc EQUAL 0)
  message(FATAL_ERROR "coverage merge failed (exit code ${_merge_rc})")
endif()

if(NOT EXISTS "${_xml}")
  message(FATAL_ERROR "cobertura report was not created: ${_xml}")
endif()

file(READ "${_xml}" _xml_text)
string(REGEX MATCH "line-rate=\"([0-9.]+)\"" _line_rate "${_xml_text}")
if(CMAKE_MATCH_1)
  message(STATUS "coverage line rate: ${CMAKE_MATCH_1}")
endif()

message(STATUS "coverage report: ${_xml}")
