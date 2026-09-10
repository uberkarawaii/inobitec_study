# в переменных: exit-коды, out/err потоки как 1/2, пустые аргументы для программы
set(SUCCESS 0)
set(USAGE 64)
set(DATA 65)
set(NO_INPUT 66)
set(IO_FAIL 74)
set(OUT 1)
set(ERR 2)
set(EMPTY_ARG "")
# для достижения io-fail
set(FOLDER_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/input_data")

# === T1 C TESTS ====
# входные данные с ошибками
add_case(t1_c_empty t1_c "${EMPTY_ARG}" t1_nul "${NO_INPUT}" CONTAINS t1_nul "${ERR}")
add_case(t1_c_abc t1_c "${EMPTY_ARG}" t1_abc "${DATA}" CONTAINS t1_abc "${ERR}")
add_case(t1_c_float t1_c "${EMPTY_ARG}" t1_float "${DATA}" CONTAINS t1_float "${ERR}")
add_case(t1_c_high t1_c "${EMPTY_ARG}" t1_high "${USAGE}" CONTAINS t1_high "${ERR}")
# нормальные входные данные
add_case(t1_c_norm t1_c "${EMPTY_ARG}" t1_test1 "${SUCCESS}" EQ t1_test1 "${OUT}")

# === T1 CPP TESTS ====
# входные данные с ошибками
add_case(t1_cpp_empty t1_cpp "${EMPTY_ARG}" t1_nul "${NO_INPUT}" CONTAINS t1_nul "${ERR}")
add_case(t1_cpp_abc t1_cpp "${EMPTY_ARG}" t1_abc "${DATA}" CONTAINS t1_abc "${ERR}")
add_case(t1_cpp_float t1_cpp "${EMPTY_ARG}" t1_float "${DATA}" CONTAINS t1_float "${ERR}")
add_case(t1_cpp_high t1_cpp "${EMPTY_ARG}" t1_high "${USAGE}" CONTAINS t1_high "${ERR}")
# нормальные входные данные
add_case(t1_cpp_norm t1_cpp "${EMPTY_ARG}" t1_test1 "${SUCCESS}" EQ t1_test1 "${OUT}")

# === T2 C TESTS ===
# входные данные с ошибками
add_case(t2_c_eof_name t2_c "${EMPTY_ARG}" t2_eof_name "${NO_INPUT}" CONTAINS t2_eof_name "${ERR}")
add_case(t2_c_empty_name t2_c "${EMPTY_ARG}" t2_empty_name "${DATA}" CONTAINS t2_empty_name "${ERR}")
add_case(t2_c_eof_vertexes t2_c "${EMPTY_ARG}" t2_eof_vertexes "${NO_INPUT}" CONTAINS t2_eof_vertexes "${ERR}")
add_case(t2_c_empty_vertexes t2_c "${EMPTY_ARG}" t2_empty_vertexes "${DATA}" CONTAINS t2_empty_vertexes "${ERR}")
add_case(t2_c_fractional_vertexes t2_c "${EMPTY_ARG}" t2_fractional_vertexes "${DATA}" CONTAINS t2_fractional_vertexes "${ERR}")
add_case(t2_c_nan_vertexes t2_c "${EMPTY_ARG}" t2_nan_vertexes "${DATA}" CONTAINS t2_nan_vertexes "${ERR}")
add_case(t2_c_negative_vertexes t2_c "${EMPTY_ARG}" t2_negative_vertexes "${USAGE}" CONTAINS t2_negative_vertexes "${ERR}")
# нормальные входные данные
add_case(t2_c_norm t2_c "${EMPTY_ARG}" t2_input "${SUCCESS}" EQ t2_norm "${OUT}")

# === T2 CPP TESTS ===
# входные данные с ошибками
add_case(t2_cpp_eof_name t2_cpp "${EMPTY_ARG}" t2_eof_name "${NO_INPUT}" CONTAINS t2_eof_name "${ERR}")
add_case(t2_cpp_empty_name t2_cpp "${EMPTY_ARG}" t2_empty_name "${DATA}" CONTAINS t2_empty_name "${ERR}")
add_case(t2_cpp_eof_vertexes t2_cpp "${EMPTY_ARG}" t2_eof_vertexes "${NO_INPUT}" CONTAINS t2_eof_vertexes "${ERR}")
add_case(t2_cpp_empty_vertexes t2_cpp "${EMPTY_ARG}" t2_empty_vertexes "${DATA}" CONTAINS t2_empty_vertexes "${ERR}")
add_case(t2_cpp_fractional_vertexes t2_cpp "${EMPTY_ARG}" t2_fractional_vertexes "${DATA}" CONTAINS t2_fractional_vertexes "${ERR}")
add_case(t2_cpp_nan_vertexes t2_cpp "${EMPTY_ARG}" t2_nan_vertexes "${DATA}" CONTAINS t2_nan_vertexes "${ERR}")
add_case(t2_cpp_negative_vertexes t2_cpp "${EMPTY_ARG}" t2_negative_vertexes "${USAGE}" CONTAINS t2_negative_vertexes "${ERR}")
# нормальные входные данные
add_case(t2_cpp_norm t2_cpp "${EMPTY_ARG}" t2_input "${SUCCESS}" EQ t2_norm "${OUT}")

# === T3 C TESTS ===
# тест на io-fail - только под UNIX
if(UNIX)
  add_case(t3_c_io t3_c "${EMPTY_ARG}" "${FOLDER_INPUT}" "${IO_FAIL}" CONTAINS t3_t4_io_fail "${ERR}")
endif()
# входные данные с ошибками
add_case(t3_c_empty t3_c "${EMPTY_ARG}" t3_empty_case "${NO_INPUT}" CONTAINS t3_empty_case "${ERR}")
add_case(t3_c_not_digit_case t3_c "${EMPTY_ARG}" t3_not_digit_case "${DATA}" CONTAINS t3_not_digit_case "${ERR}")
add_case(t3_c_too_few_args_case t3_c "${EMPTY_ARG}" t3_too_few_args_case "${DATA}" CONTAINS t3_too_few_args_case "${ERR}")
add_case(t3_c_too_much_args_case t3_c "${EMPTY_ARG}" t3_too_much_args_case "${DATA}" CONTAINS t3_too_much_args_case "${ERR}")
add_case(t3_c_extra_nan t3_c "${EMPTY_ARG}" t3_extra_nan "${DATA}" CONTAINS t3_extra_nan "${ERR}")
add_case(t3_c_x_y_z_4k t3_c "${EMPTY_ARG}" t3_x_y_z_4k "${DATA}" CONTAINS t3_x_y_z_4k "${ERR}")
add_case(t3_c_inf_coord t3_c "${EMPTY_ARG}" t3_inf_coord "${DATA}" CONTAINS t3_inf_coord "${ERR}")
add_case(t3_c_x_ky_z t3_c "${EMPTY_ARG}" t3_x_ky_z "${DATA}" CONTAINS t3_x_ky_z "${ERR}")
add_case(t3_c_x_yk_z t3_c "${EMPTY_ARG}" t3_x_yk_z "${DATA}" CONTAINS t3_x_yk_z "${ERR}")
# нормальные входные данные
add_case(t3_c_test1 t3_c "${EMPTY_ARG}" t3_test1 "${SUCCESS}" EQ t3_test1 "${OUT}")
add_case(t3_c_test2 t3_c "${EMPTY_ARG}" t3_test2 "${SUCCESS}" EQ t3_test2 "${OUT}") 
add_case(t3_c_test3 t3_c "${EMPTY_ARG}" t3_test3 "${SUCCESS}" EQ t3_test3 "${OUT}")
add_case(t3_c_scientific t3_c "${EMPTY_ARG}" t3_scientific "${SUCCESS}" EQ t3_scientific "${OUT}")

# === T3 CPP TESTS ===
# тест на io-fail - только под UNIX
if(UNIX)
  add_case(t3_cpp_io t3_cpp "${EMPTY_ARG}" "${FOLDER_INPUT}" "${IO_FAIL}" CONTAINS t3_t4_io_fail "${ERR}")
endif()
# входные данные с ошибками
add_case(t3_cpp_empty t3_cpp "${EMPTY_ARG}" t3_empty_case "${NO_INPUT}" CONTAINS t3_empty_case "${ERR}")
add_case(t3_cpp_not_digit_case t3_cpp "${EMPTY_ARG}" t3_not_digit_case "${DATA}" CONTAINS t3_not_digit_case "${ERR}")
add_case(t3_cpp_too_few_args_case t3_cpp "${EMPTY_ARG}" t3_too_few_args_case "${DATA}" CONTAINS t3_too_few_args_case "${ERR}")
add_case(t3_cpp_too_much_args_case t3_cpp "${EMPTY_ARG}" t3_too_much_args_case "${DATA}" CONTAINS t3_too_much_args_case "${ERR}")
add_case(t3_cpp_extra_nan t3_cpp "${EMPTY_ARG}" t3_extra_nan "${DATA}" CONTAINS t3_extra_nan "${ERR}")
add_case(t3_cpp_x_y_z_4k t3_cpp "${EMPTY_ARG}" t3_x_y_z_4k "${DATA}" CONTAINS t3_x_y_z_4k "${ERR}")
add_case(t3_cpp_inf_coord t3_cpp "${EMPTY_ARG}" t3_inf_coord "${DATA}" CONTAINS t3_inf_coord "${ERR}")
add_case(t3_cpp_x_ky_z t3_cpp "${EMPTY_ARG}" t3_x_ky_z "${DATA}" CONTAINS t3_x_ky_z "${ERR}")
add_case(t3_cpp_x_yk_z t3_cpp "${EMPTY_ARG}" t3_x_yk_z "${DATA}" CONTAINS t3_x_yk_z "${ERR}")
# нормальные входные данные
add_case(t3_cpp_test1 t3_cpp "${EMPTY_ARG}" t3_test1 "${SUCCESS}" EQ t3_test1 "${OUT}")
add_case(t3_cpp_test2 t3_cpp "${EMPTY_ARG}" t3_test2 "${SUCCESS}" EQ t3_test2 "${OUT}") 
add_case(t3_cpp_test3 t3_cpp "${EMPTY_ARG}" t3_test3 "${SUCCESS}" EQ t3_test3 "${OUT}")
add_case(t3_cpp_scientific t3_cpp "${EMPTY_ARG}" t3_scientific "${SUCCESS}" EQ t3_scientific "${OUT}")

# === РАДИУСЫ ===
set(RAD_TOO_MUCH 2 3)
set(RAD_SYMB 4ch)
set(RAD_INF inf)
set(RAD_NEG -10)
set(RAD1 1)
set(RAD5 5)
set(RAD8 8)
set(RAD10 10)
set(RAD100 100)

# === T4 C TESTS ===
# тест на io-fail - только под UNIX
if(UNIX)
  add_case(t4_c_io t4_c "${RAD1}" "${FOLDER_INPUT}" "${IO_FAIL}" CONTAINS t3_t4_io_fail "${ERR}")
endif()
# плохие значения радиуса
# t4_bad_radius - для наличия потока input, сам по себе не имеет значения
add_case(t4_c_radius_empty t4_c "${EMPTY_ARG}" t4_bad_radius "${USAGE}" CONTAINS t4_radius_empty "${ERR}")
add_case(t4_c_radius_too_much t4_c "${RAD_TOO_MUCH}" t4_bad_radius "${USAGE}" CONTAINS t4_radius_too_much "${ERR}")
add_case(t4_c_radius_symbol t4_c "${RAD_SYMB}" t4_bad_radius "${USAGE}" CONTAINS t4_radius_symbol "${ERR}")
add_case(t4_c_radius_inf t4_c "${RAD_INF}" t4_bad_radius "${USAGE}" CONTAINS t4_radius_inf "${ERR}")
add_case(t4_c_radius_negative t4_c "${RAD_NEG}" t4_bad_radius "${USAGE}" CONTAINS t4_radius_negative "${ERR}")
# плохие точки
add_case(t4_c_symbol_coords t4_c "${RAD1}" t4_symbol_coords "${DATA}" CONTAINS t4_symbol_coords "${ERR}")
add_case(t4_c_too_few_coords t4_c "${RAD1}" t4_too_few_coords "${DATA}" CONTAINS t4_too_few_coords "${ERR}")
add_case(t4_c_empty_coords t4_c "${RAD1}" t4_empty_coords "${NO_INPUT}" CONTAINS t4_empty_coords "${ERR}")
add_case(t4_c_too_much_coords t4_c "${RAD1}" t4_too_much_coords "${DATA}" CONTAINS t4_too_much_coords "${ERR}")
add_case(t4_c_extra_nan t4_c "${RAD1}" t4_extra_nan "${DATA}" CONTAINS t4_extra_nan "${ERR}")
add_case(t4_c_x_y_z_4k t4_c "${RAD1}" t4_x_y_z_4k "${DATA}" CONTAINS t4_x_y_z_4k "${ERR}")
add_case(t4_c_x_y_z_k4 t4_c "${RAD1}" t4_x_y_z_k4 "${DATA}" CONTAINS t4_x_y_z_k4 "${ERR}")
add_case(t4_c_inf_coord t4_c "${RAD1}" t4_inf_coord "${DATA}" CONTAINS t4_inf_coord "${ERR}")
add_case(t4_c_x_ky_z t4_c "${RAD1}" t4_x_ky_z "${DATA}" CONTAINS t4_x_ky_z "${ERR}")
add_case(t4_c_x_yk_z t4_c "${RAD1}" t4_x_yk_z "${DATA}" CONTAINS t4_x_yk_z "${ERR}")
# нормальные данные
# единственная точка и она проходит
add_case(t4_c_test1 t4_c "${RAD8}" t4_test1 "${SUCCESS}" EQ t4_test1 "${OUT}")
# много точек и только одна проходит
add_case(t4_c_test2 t4_c "${RAD10}" t4_test2 "${SUCCESS}" EQ t4_test2 "${OUT}")
# одна точка, но она не проходит
add_case(t4_c_test3 t4_c "${RAD5}" t4_test3 "${SUCCESS}" EQ t4_test3 "${OUT}")
# входные данные с минусом и экспонентой 
add_case(t4_c_scientific t4_c "${RAD100}" t4_scientific "${SUCCESS}" EQ t4_scientific "${OUT}")

# === T4 CPP TESTS ===
# тест на io-fail - только под UNIX
if(UNIX)
  add_case(t4_cpp_io t4_cpp "${RAD1}" "${FOLDER_INPUT}" "${IO_FAIL}" CONTAINS t3_t4_io_fail "${ERR}")
endif()
# плохие значения радиуса
add_case(t4_cpp_radius_empty t4_cpp "${EMPTY_ARG}" t4_bad_radius "${USAGE}" CONTAINS t4_radius_empty "${ERR}")
add_case(t4_cpp_radius_too_much t4_cpp "${RAD_TOO_MUCH}" t4_bad_radius "${USAGE}" CONTAINS t4_radius_too_much "${ERR}")
add_case(t4_cpp_radius_symbol t4_cpp "${RAD_SYMB}" t4_bad_radius "${USAGE}" CONTAINS t4_radius_symbol "${ERR}")
add_case(t4_cpp_radius_inf t4_cpp "${RAD_INF}" t4_bad_radius "${USAGE}" CONTAINS t4_radius_inf "${ERR}")
add_case(t4_cpp_radius_negative t4_cpp "${RAD_NEG}" t4_bad_radius "${USAGE}" CONTAINS t4_radius_negative "${ERR}")
# плохие точки
add_case(t4_cpp_symbol_coords t4_cpp "${RAD1}" t4_symbol_coords "${DATA}" CONTAINS t4_symbol_coords "${ERR}")
add_case(t4_cpp_too_few_coords t4_cpp "${RAD1}" t4_too_few_coords "${DATA}" CONTAINS t4_too_few_coords "${ERR}")
add_case(t4_cpp_empty_coords t4_cpp "${RAD1}" t4_empty_coords "${NO_INPUT}" CONTAINS t4_empty_coords "${ERR}")
add_case(t4_cpp_too_much_coords t4_cpp "${RAD1}" t4_too_much_coords "${DATA}" CONTAINS t4_too_much_coords "${ERR}")
add_case(t4_cpp_extra_nan t4_cpp "${RAD1}" t4_extra_nan "${DATA}" CONTAINS t4_extra_nan "${ERR}")
add_case(t4_cpp_x_y_z_4k t4_cpp "${RAD1}" t4_x_y_z_4k "${DATA}" CONTAINS t4_x_y_z_4k "${ERR}")
add_case(t4_cpp_x_y_z_k4 t4_cpp "${RAD1}" t4_x_y_z_k4 "${DATA}" CONTAINS t4_x_y_z_k4 "${ERR}")
add_case(t4_cpp_inf_coord t4_cpp "${RAD1}" t4_inf_coord "${DATA}" CONTAINS t4_inf_coord "${ERR}")
add_case(t4_cpp_x_ky_z t4_cpp "${RAD1}" t4_x_ky_z "${DATA}" CONTAINS t4_x_ky_z "${ERR}")
add_case(t4_cpp_x_yk_z t4_cpp "${RAD1}" t4_x_yk_z "${DATA}" CONTAINS t4_x_yk_z "${ERR}")
# нормальные данные
add_case(t4_cpp_test1 t4_cpp "${RAD8}" t4_test1 "${SUCCESS}" EQ t4_test1 "${OUT}")
add_case(t4_cpp_test2 t4_cpp "${RAD10}" t4_test2 "${SUCCESS}" EQ t4_test2 "${OUT}")
add_case(t4_cpp_test3 t4_cpp "${RAD5}" t4_test3 "${SUCCESS}" EQ t4_test3 "${OUT}")
add_case(t4_cpp_scientific t4_cpp "${RAD100}" t4_scientific "${SUCCESS}" EQ t4_scientific "${OUT}")
