# === UNIT TESTS (первичная версия) ===
# parse_point
add_executable(parse_point_cpp parse_point.cpp)
target_link_libraries(parse_point_cpp PRIVATE common_cpp_static)
# ON: линкуется с ASan-библиотекой, т.к. линк с common_*_static, который уже с ASan - потому нужно для совместимости
configure_target(parse_point_cpp ON)      
add_test(NAME parse_point_cpp COMMAND parse_point_cpp)

add_executable(parse_point_c parse_point.c)
target_link_libraries(parse_point_c PRIVATE common_c_static)
configure_target(parse_point_c ON)
add_test(NAME parse_point_c COMMAND parse_point_c)