#pragma once
#include <vector>

#include "../../common/exit_codes.hpp"
#include "common_cases.hpp"
#include "e2e_case.hpp"
#include "expected_points.hpp"
#include "expected_t3.hpp"

namespace e2e {
inline std::vector<Case> make_t3_cases() {
    std::vector<Case> v = {
        // входные данные с ошибками
        {"t3_empty", {}, "t3_empty_case", exit_code::no_in, Stream::err, kT3EmptyCase},
        {"t3_not_digit_case", {}, "t3_not_digit_case", exit_code::data, Stream::err, kT3NotDigitCase},
        {"t3_too_few_args_case", {}, "t3_too_few_args_case", exit_code::data, Stream::err, kPointTooFewCoords},
        {"t3_too_much_args_case", {}, "t3_too_much_args_case", exit_code::data, Stream::err, kPointTooMuchCoords},
        {"t3_extra_nan", {}, "t3_extra_nan", exit_code::data, Stream::err, kPointExtraNan},
        {"t3_x_y_z_4k", {}, "t3_x_y_z_4k", exit_code::data, Stream::err, kPointXyz4K},
        {"t3_x_y_z_k4", {}, "t3_x_y_z_k4", exit_code::data, Stream::err, kPointXyzK4},
        {"t3_inf_coord", {}, "t3_inf_coord", exit_code::data, Stream::err, kPointInfCoord},
        {"t3_x_ky_z", {}, "t3_x_ky_z", exit_code::data, Stream::err, kPointXKyZ},
        {"t3_x_yk_z", {}, "t3_x_yk_z", exit_code::data, Stream::err, kPointXYkZ},
        {"t3_double_plus", {}, "t3_double_plus", exit_code::data, Stream::err, kT3DoublePlus},
        {"t3_plus_abc", {}, "t3_plus_abc", exit_code::data, Stream::err, kT3PlusAbc},
        {"t3_plus_minus_coord", {}, "t3_t4_plus_minus_coord", exit_code::data, Stream::err, kPointPlusMinusCoord},
        {"t3_plus_minus_extra", {}, "t3_t4_plus_minus_extra", exit_code::data, Stream::err, kPointPlusMinusExtra},

        // нормальные входные данные
        {"t3_test1", {}, "t3_test1", exit_code::success, Stream::out, kT3Test1},
        {"t3_test2", {}, "t3_test2", exit_code::success, Stream::out, kT3Test2},
        {"t3_test3", {}, "t3_test3", exit_code::success, Stream::out, kT3Test3},
        {"t3_scientific", {}, "t3_scientific", exit_code::success, Stream::out, kT3Scientific},

        // hex / overflow / подформа записи
        {"t3_hex_coord", {}, "t3_hex_coord", exit_code::data, Stream::err, kPointHexCoord},
        {"t3_hex_coord_upper", {}, "t3_hex_coord_upper", exit_code::data, Stream::err, kPointHexCoordUpper},
        {"t3_hex_extra_coord", {}, "t3_hex_extra_coord", exit_code::data, Stream::err, kPointHexExtraCoord},
        {"t3_overflow_coord", {}, "t3_overflow_coord", exit_code::data, Stream::err, kPointOverflowCoord},
        {"t3_underflow_coord", {}, "t3_underflow_coord", exit_code::data, Stream::err, kPointUnderflowCoord},
        {"t3_subnormal_coord", {}, "t3_subnormal_coord", exit_code::success, Stream::out, kT3SubnormalCoord},

        // вывод совпадает с t3_test1: та же точка (1.5, 2.5, 3.5), отличаетс€ ведущий пробел во входе
        {"t3_leading_spaces", {}, "t3_leading_spaces", exit_code::success, Stream::out, kT3Test1},
        {"t3_exp_plus", {}, "t3_exp_plus", exit_code::success, Stream::out, kT3ExpPlus},
        {"t3_exp_upper", {}, "t3_exp_upper", exit_code::success, Stream::out, kT3ExpUpper},
        {"t3_exp_negative", {}, "t3_exp_negative", exit_code::success, Stream::out, kT3ExpNegative},
        {"t3_infinity_coord", {}, "t3_infinity_coord", exit_code::data, Stream::err, kPointInfinityCoord},
        {"t3_nan_payload_coord", {}, "t3_nan_payload_coord", exit_code::data, Stream::err, kPointNanPayloadCoord},
        {"t3_comma_coord", {}, "t3_comma_coord", exit_code::data, Stream::err, kPointCommaCoord},
        {"t3_dot_trailing_coord", {}, "t3_dot_trailing_coord", exit_code::success, Stream::out, kT3DotTrailingCoord},
        {"t3_dot_leading_coord", {}, "t3_dot_leading_coord", exit_code::success, Stream::out, kT3DotLeadingCoord},

        // справка
        {"t3_help", {"--help"}, kEmptyInput, exit_code::success, Stream::out, kT3Help},
        {"t3_help_short", {"-h"}, kEmptyInput, exit_code::success, Stream::out, kT3Help},
        {"t3_help_extra", {"--help", "extra"}, kEmptyInput, exit_code::success, Stream::out, kT3Help},
        {"t3_help_short_extra", {"-h", "extra"}, kEmptyInput, exit_code::success, Stream::out, kT3Help},
    };
#ifndef _WIN32
    // io-fail: stdin Ч каталог, а не файл; под Windows такое не спровоцировать
    v.insert(v.begin(), Case{"t3_io", {}, "", exit_code::io_fail, Stream::err, kIoFail, true});
#endif
    v.insert(v.end(), kVersionCases.begin(), kVersionCases.end());
    v.insert(v.end(), kUnknownArgCases.begin(), kUnknownArgCases.end());
    return v;
}
inline const std::vector<Case> t3_cases = make_t3_cases();
} // namespace e2e
