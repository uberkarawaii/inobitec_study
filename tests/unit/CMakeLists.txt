# === UNIT TESTS ===
# ф-ция с вынесением дублирующихся действия
function(add_unit_gtest target src lib)
  add_executable(${target} ${src})
  target_link_libraries(${target} PRIVATE ${lib} GTest::gtest_main)
  configure_target(${target} ON)
  # gtest-dll-ки через свою ф-цию, не vcpkg applocal-ом. до discover - к его моменту dll уже будут на месте 
  deploy_vcpkg_dlls(${target})
  # увидит все кейсы и сделает их отдельными тестами, а не под-тестами в составе одной сущности
  # опред. префикс для фильтрации через пресеты, по имени
  # убирать GetParam() - разнящееся м-у сборками число - из имени теста
  gtest_discover_tests(${target} TEST_PREFIX "${target}/" NO_PRETTY_VALUES)
endfunction()

# на каждую тестируемую ф-ию - свой .exe; так с каждой стороны - на с, на с++
# конфигурация ON (с санитайзером) - по таким же правилам как библиотеки
# т.к. есть линк с библиотекой и нужно соответствие м-у собой 

# parse_point (geometry + string_utils)
add_unit_gtest(parse_point_cpp parse_point.cpp common_cpp_static)

add_executable(parse_point_c parse_point.c)
target_link_libraries(parse_point_c PRIVATE common_c_static)
configure_target(parse_point_c ON)
add_test(NAME parse_point_c COMMAND parse_point_c)

# parse_int32
add_unit_gtest(parse_int32_cpp parse_int32.cpp string_utils_cpp_static)

add_executable(parse_int32_c parse_int32.c)
target_link_libraries(parse_int32_c PRIVATE string_utils_c_static)
configure_target(parse_int32_c ON)
add_test(NAME parse_int32_c COMMAND parse_int32_c)

# is_empty
add_unit_gtest(is_empty_cpp is_empty.cpp string_utils_cpp_static)

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
add_unit_gtest(trim_string_cpp trim_string.cpp string_utils_cpp_static)

add_executable(trim_string_c trim_string.c)
target_link_libraries(trim_string_c PRIVATE string_utils_c_static)
configure_target(trim_string_c ON)
add_test(NAME trim_string_c COMMAND trim_string_c)

# вынесенные в common ф-ции, которые до этого были внутри задач (или в виде ф-ций, или даже не вынесенные)
# point_distance (geometry)
add_unit_gtest(point_distance_cpp point_distance.cpp common_cpp_static)

add_executable(point_distance_c point_distance.c)
target_link_libraries(point_distance_c PRIVATE common_c_static)
configure_target(point_distance_c ON)
add_test(NAME point_distance_c COMMAND point_distance_c)

# centroid (geometry)
add_unit_gtest(centroid_cpp centroid.cpp common_cpp_static)

add_executable(centroid_c centroid.c)
target_link_libraries(centroid_c PRIVATE common_c_static)
configure_target(centroid_c ON)
add_test(NAME centroid_c COMMAND centroid_c)

# polygon_vertex (geometry)
add_unit_gtest(polygon_vertex_cpp polygon_vertex.cpp common_cpp_static)

add_executable(polygon_vertex_c polygon_vertex.c)
target_link_libraries(polygon_vertex_c PRIVATE common_c_static)
configure_target(polygon_vertex_c ON)
add_test(NAME polygon_vertex_c COMMAND polygon_vertex_c)

# parse_radius (string_utils)
add_unit_gtest(parse_radius_cpp parse_radius.cpp string_utils_cpp_static)

add_executable(parse_radius_c parse_radius.c)
target_link_libraries(parse_radius_c PRIVATE string_utils_c_static)
configure_target(parse_radius_c ON)
add_test(NAME parse_radius_c COMMAND parse_radius_c)

# vertex_form_index (string_utils)
add_unit_gtest(vertex_form_index_cpp vertex_form_index.cpp string_utils_cpp_static)

add_executable(vertex_form_index_c vertex_form_index.c)
target_link_libraries(vertex_form_index_c PRIVATE string_utils_c_static)
configure_target(vertex_form_index_c ON)
add_test(NAME vertex_form_index_c COMMAND vertex_form_index_c)

