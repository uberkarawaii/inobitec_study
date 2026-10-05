# === UNIT TESTS ===
# на каждую тестируемую ф-ию - свой .exe; так с каждой стороны - на с, на с++
# конфигурация ON (с санитайзером) - по таким же правилам как библиотеки
# т.к. есть линк с библиотекой и нужно соответствие м-у собой 

# parse_point (geometry + string_utils)
add_executable(parse_point_cpp parse_point.cpp)
target_link_libraries(parse_point_cpp PRIVATE common_cpp_static GTest::gtest_main)
configure_target(parse_point_cpp ON)
# увидит все кейсы и сделает их отдельными тестами, а не под-тестами в составе одной сущности
# опред. префикс для фильтрации через пресеты, по имени
# убирать GetParam() - разнящееся м-у сборками число - из имени теста
gtest_discover_tests(parse_point_cpp
  TEST_PREFIX "parse_point_cpp/"
  NO_PRETTY_VALUES)

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

# вынесенные в common ф-ции, которые до этого были внутри задач (или в виде ф-ций, или даже не вынесенные)
# point_distance (geometry)
add_executable(point_distance_cpp point_distance.cpp)
target_link_libraries(point_distance_cpp PRIVATE common_cpp_static)
configure_target(point_distance_cpp ON)
add_test(NAME point_distance_cpp COMMAND point_distance_cpp)

add_executable(point_distance_c point_distance.c)
target_link_libraries(point_distance_c PRIVATE common_c_static)
configure_target(point_distance_c ON)
add_test(NAME point_distance_c COMMAND point_distance_c)

# centroid (geometry)
add_executable(centroid_cpp centroid.cpp)
target_link_libraries(centroid_cpp PRIVATE common_cpp_static)
configure_target(centroid_cpp ON)
add_test(NAME centroid_cpp COMMAND centroid_cpp)

add_executable(centroid_c centroid.c)
target_link_libraries(centroid_c PRIVATE common_c_static)
configure_target(centroid_c ON)
add_test(NAME centroid_c COMMAND centroid_c)

# polygon_vertex (geometry)
add_executable(polygon_vertex_cpp polygon_vertex.cpp)
target_link_libraries(polygon_vertex_cpp PRIVATE common_cpp_static)
configure_target(polygon_vertex_cpp ON)
add_test(NAME polygon_vertex_cpp COMMAND polygon_vertex_cpp)

add_executable(polygon_vertex_c polygon_vertex.c)
target_link_libraries(polygon_vertex_c PRIVATE common_c_static)
configure_target(polygon_vertex_c ON)
add_test(NAME polygon_vertex_c COMMAND polygon_vertex_c)

# parse_radius (string_utils)
add_executable(parse_radius_cpp parse_radius.cpp)
target_link_libraries(parse_radius_cpp PRIVATE string_utils_cpp_static)
configure_target(parse_radius_cpp ON)
add_test(NAME parse_radius_cpp COMMAND parse_radius_cpp)

add_executable(parse_radius_c parse_radius.c)
target_link_libraries(parse_radius_c PRIVATE string_utils_c_static)
configure_target(parse_radius_c ON)
add_test(NAME parse_radius_c COMMAND parse_radius_c)

# vertex_form_index (string_utils)
add_executable(vertex_form_index_cpp vertex_form_index.cpp)
target_link_libraries(vertex_form_index_cpp PRIVATE string_utils_cpp_static)
configure_target(vertex_form_index_cpp ON)
add_test(NAME vertex_form_index_cpp COMMAND vertex_form_index_cpp)

add_executable(vertex_form_index_c vertex_form_index.c)
target_link_libraries(vertex_form_index_c PRIVATE string_utils_c_static)
configure_target(vertex_form_index_c ON)
add_test(NAME vertex_form_index_c COMMAND vertex_form_index_c)

