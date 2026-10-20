# Boost.Process Ч компилируема€ библиотека, не header-only
find_package(Boost CONFIG REQUIRED COMPONENTS process)
check_component(Boost "${Boost_VERSION}" "${LIB_BOOST_TESTED}" "0")

# ф-ци€ дл€ добавлени€ е2е тестов. вынесена т.к. это дублирование дл€ t1-t4
function(add_e2e_test target src exe_c exe_cpp)
  add_executable(${target} ${src})
  target_link_libraries(${target} PRIVATE e2e_runner GTest::gtest_main)
  target_include_directories(${target} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}")
  target_compile_definitions(${target} PRIVATE
    E2E_EXE_C="$<TARGET_FILE:${exe_c}>"
    E2E_EXE_CPP="$<TARGET_FILE:${exe_cpp}>"
    E2E_INPUT_DIR="${CMAKE_SOURCE_DIR}/tests/input_data"
    E2E_RESULTS_DIR="${CMAKE_CURRENT_BINARY_DIR}/results")
  configure_target(${target} OFF)
  deploy_vcpkg_dlls(${target})
  add_dependencies(${target} ${exe_c} ${exe_cpp})
  gtest_discover_tests(${target} TEST_PREFIX "e2e/${target}/" NO_PRETTY_VALUES)
endfunction()

# ќбщий e2e-раннер: FILE* + boost::process + чтение потоков ("как ранее" run_case)
add_library(e2e_runner STATIC e2e_runner.cpp)
target_include_directories(e2e_runner PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(e2e_runner PUBLIC Boost::process)
configure_target(e2e_runner OFF)

#  аталог прогонов и version_expected.h из PROJECT_VERSION
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/results")
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/version_expected.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/version_expected.h"
  @ONLY)

# t1-t4 c/c++ e2e тесты
add_e2e_test(t1_e2e t1_e2e.cpp t1_c t1_cpp)
add_e2e_test(t2_e2e t2_e2e.cpp t2_c t2_cpp)
add_e2e_test(t3_e2e t3_e2e.cpp t3_c t3_cpp)

# тест open_file, при ситуации несуществующего input/output
add_executable(e2e_runner_test e2e_runner_test.cpp)
target_link_libraries(e2e_runner_test PRIVATE e2e_runner GTest::gtest_main)
target_compile_definitions(e2e_runner_test PRIVATE
  E2E_INPUT_DIR="${CMAKE_SOURCE_DIR}/tests/input_data"
  E2E_RESULTS_DIR="${CMAKE_CURRENT_BINARY_DIR}/results")
configure_target(e2e_runner_test OFF)
deploy_vcpkg_dlls(e2e_runner_test)
gtest_discover_tests(e2e_runner_test TEST_PREFIX "e2e/runner/")
