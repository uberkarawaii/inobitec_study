# Boost.Process — компилируемая библиотека, не header-only
find_package(Boost CONFIG REQUIRED COMPONENTS process)
check_component(Boost "${Boost_VERSION}" "${LIB_BOOST_TESTED}" "0")

# Общий e2e-раннер: FILE* + boost::process + чтение потоков ("как ранее" run_case)
add_library(e2e_runner STATIC e2e_runner.cpp)
target_include_directories(e2e_runner PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(e2e_runner PUBLIC Boost::process)
configure_target(e2e_runner OFF)

# Каталог прогонов и version_expected.h из PROJECT_VERSION
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/results")
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/version_expected.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/version_expected.h"
  @ONLY)

# t1: C и C++ в одном suite (ExeKind х Case)
# t1_e2e.cpp на gtest, берёт Case-данные из t1_cases.hpp
add_executable(t1_e2e t1_e2e.cpp)
target_link_libraries(t1_e2e PRIVATE e2e_runner GTest::gtest_main)
target_include_directories(t1_e2e PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}")
target_compile_definitions(t1_e2e PRIVATE
  E2E_T1_C="$<TARGET_FILE:t1_c>"
  E2E_T1_CPP="$<TARGET_FILE:t1_cpp>"
  E2E_INPUT_DIR="${CMAKE_SOURCE_DIR}/tests/input_data"
  E2E_RESULTS_DIR="${CMAKE_CURRENT_BINARY_DIR}/results")
configure_target(t1_e2e OFF)
# DLL до discovery: gtest_discover_tests запускает exe при сборке
deploy_vcpkg_dlls(t1_e2e)
add_dependencies(t1_e2e t1_c t1_cpp)
gtest_discover_tests(t1_e2e TEST_PREFIX "e2e/t1_e2e/" NO_PRETTY_VALUES)

# тест open_file, при ситуации несуществующего input/output
add_executable(e2e_runner_test e2e_runner_test.cpp)
target_link_libraries(e2e_runner_test PRIVATE e2e_runner GTest::gtest_main)
target_compile_definitions(e2e_runner_test PRIVATE
  E2E_INPUT_DIR="${CMAKE_SOURCE_DIR}/tests/input_data"
  E2E_RESULTS_DIR="${CMAKE_CURRENT_BINARY_DIR}/results")
configure_target(e2e_runner_test OFF)
deploy_vcpkg_dlls(e2e_runner_test)
gtest_discover_tests(e2e_runner_test TEST_PREFIX "e2e/runner/")

