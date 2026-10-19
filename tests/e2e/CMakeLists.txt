# имя пакета - Boost, имя таргета, к которому потом будет линк, - Boost::process
find_package(Boost CONFIG REQUIRED COMPONENTS process)

# это будет child-процесс
add_executable(argv_echo argv_echo.cpp)
configure_target(argv_echo OFF)

# каталог с in/err/out зонда и в нём входной файл 
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/probe")
# для пробной программы входной файл пустой - номинально проверить, что вход с bp::std_in  < in_file сработает
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/probe/probe.in" "")

# child-процессы для проверки exit-кода и полноты файла при большом выводе 
add_executable(exit_with exit_with.cpp)
configure_target(exit_with OFF)
add_executable(big_out big_out.cpp)
configure_target(big_out OFF)

# это будет parent-процесс
add_executable(bp_probe bp_probe.cpp)
target_link_libraries(bp_probe PRIVATE Boost::process)
# bp_probe будет запускать argv_echo как child-процесс
# E2E_ARGV_ECHO - путь до ребёнка
# E2E_PROBE_DIR - путь до каталога in/err/out
# exit_with/big_out - дочерние процессы для понимания exit_code на линуксе и полноты вывода
# E2E_T3_C - на вход будет подан каталог, ожидается io fail, 74
# E2E_T1_C - на вход будет подан t1_abc, ожидается ошибка с сохранившимися chcp1251 Байтами
target_compile_definitions(bp_probe PRIVATE E2E_ARGV_ECHO="$<TARGET_FILE:argv_echo>" 
                                            E2E_PROBE_DIR="${CMAKE_CURRENT_BINARY_DIR}/probe"
                                            E2E_EXIT_WITH="$<TARGET_FILE:exit_with>"
                                            E2E_BIG_OUT="$<TARGET_FILE:big_out>"
                                            E2E_T1_C="$<TARGET_FILE:t1_c>"
                                            E2E_T3_C="$<TARGET_FILE:t3_c>"
                                            E2E_INPUT_DIR="${CMAKE_SOURCE_DIR}/tests/input_data"
                                            E2E_EXPECT_DIR="${CMAKE_SOURCE_DIR}/tests/expect")
# чтобы argv_echo собрался раньше bp_probe - т.к. bp_probe использует argv_echo
add_dependencies(bp_probe argv_echo  exit_with big_out t1_c t3_c)
configure_target(bp_probe OFF)
# boostproc-dll-ки через свою ф-цию, не vcpkg applocal-ом
deploy_vcpkg_dlls(bp_probe)
