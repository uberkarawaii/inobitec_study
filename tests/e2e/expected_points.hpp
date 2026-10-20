// Generated from tests/expect (shared point-parsing expectations for t3/t4). Do not edit manually.
// CP1251 bytes must be preserved.
#pragma once

#include <string_view>

namespace e2e {

// t3_too_few_args_case, t4_too_few_coords
inline constexpr std::string_view kPointTooFewCoords =
    R"E2E(Строка 1 - недостаточно координат. Ожидалось X Y Z, получено: 2 3)E2E";

// t3_too_much_args_case, t4_too_much_coords
inline constexpr std::string_view kPointTooMuchCoords =
    R"E2E(Строка 1 - слишком много координат. Ожидалось X Y Z, получено: 1 2 3 4)E2E";

// t3_x_ky_z, t4_x_ky_z
inline constexpr std::string_view kPointXKyZ = R"E2E(Строка 1. Нечисловые данные: 1 k2 3)E2E";

// t3_x_yk_z, t4_x_yk_z
inline constexpr std::string_view kPointXYkZ = R"E2E(Строка 1. Нечисловые данные: 1 2k 3)E2E";

// t3_x_y_z_4k, t4_x_y_z_4k
inline constexpr std::string_view kPointXyz4K = R"E2E(Строка 1. Нечисловые данные: 1 2 3 4k)E2E";

// t3_x_y_z_k4, t4_x_y_z_k4
inline constexpr std::string_view kPointXyzK4 = R"E2E(Строка 1. Нечисловые данные: 1 2 3 k4)E2E";

// t3_comma_coord, t4_comma_coord
inline constexpr std::string_view kPointCommaCoord = R"E2E(Строка 1. Нечисловые данные: 1,5 2 3)E2E";

// t3_hex_coord, t4_hex_coord
inline constexpr std::string_view kPointHexCoord = R"E2E(Строка 1. Нечисловые данные: 0x10 2 3)E2E";

// t3_hex_coord_upper, t4_hex_coord_upper
inline constexpr std::string_view kPointHexCoordUpper = R"E2E(Строка 1. Нечисловые данные: 0X10 2 3)E2E";

// t3_hex_extra_coord, t4_hex_extra_coord
inline constexpr std::string_view kPointHexExtraCoord = R"E2E(Строка 1. Нечисловые данные: 1 2 3 0x10)E2E";

// t3_inf_coord, t4_inf_coord
inline constexpr std::string_view kPointInfCoord =
    R"E2E(Строка 1. Среди X Y Z обнаружена не конечная координата: inf 0 0)E2E";

// t3_infinity_coord, t4_infinity_coord
inline constexpr std::string_view kPointInfinityCoord =
    R"E2E(Строка 1. Среди X Y Z обнаружена не конечная координата: infinity 0 0)E2E";

// t3_nan_payload_coord, t4_nan_payload_coord
inline constexpr std::string_view kPointNanPayloadCoord =
    R"E2E(Строка 1. Среди X Y Z обнаружена не конечная координата: NAN(2) 0 0)E2E";

// t3_overflow_coord, t4_overflow_coord
inline constexpr std::string_view kPointOverflowCoord =
    R"E2E(Строка 1. Среди X Y Z обнаружена координата, выходящая за допустимый диапазон: 1e400 0 0)E2E";

// t3_underflow_coord, t4_underflow_coord
inline constexpr std::string_view kPointUnderflowCoord =
    R"E2E(Строка 1. Среди X Y Z обнаружена координата, выходящая за допустимый диапазон: 1e-400 0 0)E2E";

// t3_extra_nan, t4_extra_nan
inline constexpr std::string_view kPointExtraNan =
    R"E2E(Строка 1 - слишком много координат. Ожидалось X Y Z, получено: 1 2 3 nan)E2E";

// t3_t4_plus_minus_coord
inline constexpr std::string_view kPointPlusMinusCoord = R"E2E(Строка 1. Нечисловые данные: 1 +-3 2)E2E";

// t3_t4_plus_minus_extra
inline constexpr std::string_view kPointPlusMinusExtra = R"E2E(Строка 1. Нечисловые данные: 1 2 3 +-4)E2E";

// t3_t4_io_fail
inline constexpr std::string_view kIoFail = R"E2E(Строка 1 - сбой ввода-вывода)E2E";

} // namespace e2e
