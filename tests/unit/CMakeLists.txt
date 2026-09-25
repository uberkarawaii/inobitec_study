# === UNIT TESTS ===
# на каждую тестируемую ф-ию - свой .exe; так с каждой стороны - на с, на с++
# конфигурация ON (с санитайзером) - по таким же правилам как библиотеки
# т.к. есть линк с библиотекой и нужно соответствие м-у собой 

# parse_point (geometry + string_utils)
add_executable(parse_point_cpp parse_point.cpp)
target_link_libraries(parse_point_cpp PRIVATE common_cpp_static)
configure_target(parse_point_cpp ON)
add_test(NAME parse_point_cpp COMMAND parse_point_cpp)

add_executable(parse_point_c parse_point.c)
target_link_libraries(parse_point_c PRIVATE common_c_static)
configure_target(parse_point_c ON)
add_test(NAME parse_point_c COMMAND parse_point_c)

# parse_int32
add_executable(parse_int32_cpp parse_int32.cpp)
target_link_libraries(parse_int32_cpp PRIVATE string_utils_cpp_static)
configure_target(parse_int32_cpp ON)
add_test(NAME parse_int32_cpp COMMAND parse_int32_cpp)

add_executable(parse_int32_c parse_int32.c)
target_link_libraries(parse_int32_c PRIVATE string_utils_c_static)
configure_target(parse_int32_c ON)
add_test(NAME parse_int32_c COMMAND parse_int32_c)

# is_empty
add_executable(is_empty_cpp is_empty.cpp)
target_link_libraries(is_empty_cpp PRIVATE string_utils_cpp_static)
configure_target(is_empty_cpp ON)
add_test(NAME is_empty_cpp COMMAND is_empty_cpp)

add_executable(is_empty_c is_empty.c)
target_link_libraries(is_empty_c PRIVATE string_utils_c_static)
configure_target(is_empty_c ON)
add_test(NAME is_empty_c COMMAND is_empty_c)

# is_hex_prefix (только для C; С++ from_chars - не принимает hex по дефолту)
add_executable(is_hex_prefix_c is_hex_prefix.c)
target_link_libraries(is_hex_prefix_c PRIVATE string_utils_c_static)
configure_target(is_hex_prefix_c ON)
add_test(NAME is_hex_prefix_c COMMAND is_hex_prefix_c)

# trim_string / trim_str
add_executable(trim_string_cpp trim_string.cpp)
target_link_libraries(trim_string_cpp PRIVATE string_utils_cpp_static)
configure_target(trim_string_cpp ON)
add_test(NAME trim_string_cpp COMMAND trim_string_cpp)

add_executable(trim_string_c trim_string.c)
target_link_libraries(trim_string_c PRIVATE string_utils_c_static)
configure_target(trim_string_c ON)
add_test(NAME trim_string_c COMMAND trim_string_c)
