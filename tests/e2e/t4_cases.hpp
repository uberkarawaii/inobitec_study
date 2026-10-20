#pragma once
#include <vector>

#include "../../common/exit_codes.hpp"
#include "common_cases.hpp"
#include "e2e_case.hpp"
#include "expected_points.hpp"
#include "expected_t4.hpp"

namespace e2e {
inline std::vector<Case> make_t4_cases() {
    std::vector<Case> v = {
        // плохие значени€ радиуса
        {"t4_radius_empty", {}, "t4_bad_radius", exit_code::usage, Stream::err, kT4RadiusEmpty},
        {"t4_radius_too_much", {"2", "3"}, "t4_bad_radius", exit_code::usage, Stream::err, kT4RadiusTooMuch},
        {"t4_radius_symbol", {"4ch"}, "t4_bad_radius", exit_code::usage, Stream::err, kT4RadiusSymbol},
        {"t4_radius_inf", {"inf"}, "t4_bad_radius", exit_code::usage, Stream::err, kT4RadiusInf},
        {"t4_radius_negative", {"-10"}, "t4_bad_radius", exit_code::usage, Stream::err, kT4RadiusNegative},
        {"t4_radius_plus_plus", {"++10"}, "t4_bad_radius", exit_code::usage, Stream::err, kT4RadiusPlusPlus},
        {"t4_radius_plus_abc", {"+abc"}, "t4_bad_radius", exit_code::usage, Stream::err, kT4RadiusPlusAbc},
        {"t4_radius_plus_minus", {"+-3"}, "t4_bad_radius", exit_code::usage, Stream::err, kT4RadiusPlusMinus},
        {"t4_radius_hex", {"0x10"}, "t4_bad_radius", exit_code::usage, Stream::err, kT4RadiusHex},
        {"t4_radius_hex_upper", {"0X10"}, "t4_bad_radius", exit_code::usage, Stream::err, kT4RadiusHexUpper},
        {"t4_radius_overflow", {"1e400"}, "t4_bad_radius", exit_code::usage, Stream::err, kT4RadiusOverflow},
        {"t4_radius_underflow", {"1e-400"}, "t4_bad_radius", exit_code::usage, Stream::err, kT4RadiusUnderflow},
        {"t4_rad_space_hex", {"   0x10"}, "t4_bad_radius", exit_code::usage, Stream::err, kT4RadiusSpaceHex},
        {"t4_radius_spaces", {"   "}, "t4_bad_radius", exit_code::usage, Stream::err, kT4RadiusSpaces},
        {"t4_radius_inner_space", {"2 3"}, "t4_bad_radius", exit_code::usage, Stream::err, kT4RadiusInnerSpace},
        {"t4_radius_inner_quote", {"2\" 3"}, "t4_bad_radius", exit_code::usage, Stream::err, kT4RadiusInnerQuote},
        {"t4_radius_trailing_backslash",
         {"10\\"},
         "t4_bad_radius",
         exit_code::usage,
         Stream::err,
         kT4RadiusTrailingBackslash},
        {"t4_radius_multi_quote", {"2\"3\""}, "t4_bad_radius", exit_code::usage, Stream::err, kT4RadiusMultiQuote},

        // плохие точки
        {"t4_symbol_coords", {"1"}, "t4_symbol_coords", exit_code::data, Stream::err, kT4SymbolCoords},
        {"t4_too_few_coords", {"1"}, "t4_too_few_coords", exit_code::data, Stream::err, kPointTooFewCoords},
        {"t4_empty_coords", {"1"}, "t4_empty_coords", exit_code::no_in, Stream::err, kT4EmptyCoords},
        {"t4_too_much_coords", {"1"}, "t4_too_much_coords", exit_code::data, Stream::err, kPointTooMuchCoords},
        {"t4_extra_nan", {"1"}, "t4_extra_nan", exit_code::data, Stream::err, kPointExtraNan},
        {"t4_x_y_z_4k", {"1"}, "t4_x_y_z_4k", exit_code::data, Stream::err, kPointXyz4K},
        {"t4_x_y_z_k4", {"1"}, "t4_x_y_z_k4", exit_code::data, Stream::err, kPointXyzK4},
        {"t4_inf_coord", {"1"}, "t4_inf_coord", exit_code::data, Stream::err, kPointInfCoord},
        {"t4_x_ky_z", {"1"}, "t4_x_ky_z", exit_code::data, Stream::err, kPointXKyZ},
        {"t4_x_yk_z", {"1"}, "t4_x_yk_z", exit_code::data, Stream::err, kPointXYkZ},
        {"t4_plus_plus_coord", {"1"}, "t4_plus_plus_coord", exit_code::data, Stream::err, kT4PlusPlusCoord},
        {"t4_plus_abc_coord", {"1"}, "t4_plus_abc_coord", exit_code::data, Stream::err, kT4PlusAbcCoord},
        {"t4_plus_minus_coord", {"1"}, "t3_t4_plus_minus_coord", exit_code::data, Stream::err, kPointPlusMinusCoord},
        {"t4_plus_minus_extra", {"1"}, "t3_t4_plus_minus_extra", exit_code::data, Stream::err, kPointPlusMinusExtra},

        // нормальные данные
        {"t4_test1", {"8"}, "t4_test1", exit_code::success, Stream::out, kT4Test1},
        {"t4_test2", {"10"}, "t4_test2", exit_code::success, Stream::out, kT4Test2},
        {"t4_plus", {"+10"}, "t4_test2", exit_code::success, Stream::out, kT4Test2},
        {"t4_rad_leading_space", {"   10"}, "t4_test2", exit_code::success, Stream::out, kT4Test2},
        {"t4_rad_space_end", {"10   "}, "t4_test2", exit_code::success, Stream::out, kT4Test2},
        {"t4_test3", {"5"}, "t4_test3", exit_code::success, Stream::out, kT4Test3},
        {"t4_scientific", {"100"}, "t4_scientific", exit_code::success, Stream::out, kT4Scientific},
        {"t4_rad_space_plus", {"   +10"}, "t4_test2", exit_code::success, Stream::out, kT4Test2},

        // hex / overflow / подформа записи
        {"t4_hex_coord", {"1"}, "t4_hex_coord", exit_code::data, Stream::err, kPointHexCoord},
        {"t4_hex_coord_upper", {"1"}, "t4_hex_coord_upper", exit_code::data, Stream::err, kPointHexCoordUpper},
        {"t4_hex_extra_coord", {"1"}, "t4_hex_extra_coord", exit_code::data, Stream::err, kPointHexExtraCoord},
        {"t4_overflow_coord", {"1"}, "t4_overflow_coord", exit_code::data, Stream::err, kPointOverflowCoord},
        {"t4_underflow_coord", {"1"}, "t4_underflow_coord", exit_code::data, Stream::err, kPointUnderflowCoord},
        {"t4_subnormal_coord", {"1"}, "t4_subnormal_coord", exit_code::success, Stream::out, kT4SubnormalCoord},
        {"t4_leading_spaces", {"5"}, "t4_leading_spaces", exit_code::success, Stream::out, kT4LeadingSpaces},
        {"t4_exp_plus", {"400"}, "t4_exp_plus", exit_code::success, Stream::out, kT4ExpPlus},
        {"t4_exp_upper", {"400"}, "t4_exp_upper", exit_code::success, Stream::out, kT4ExpUpper},
        {"t4_exp_negative", {"1"}, "t4_exp_negative", exit_code::success, Stream::out, kT4ExpNegative},
        {"t4_infinity_coord", {"1"}, "t4_infinity_coord", exit_code::data, Stream::err, kPointInfinityCoord},
        {"t4_nan_payload_coord", {"1"}, "t4_nan_payload_coord", exit_code::data, Stream::err, kPointNanPayloadCoord},
        {"t4_comma_coord", {"1"}, "t4_comma_coord", exit_code::data, Stream::err, kPointCommaCoord},
        {"t4_dot_trailing_coord", {"8"}, "t4_dot_trailing_coord", exit_code::success, Stream::out, kT4DotTrailingCoord},
        {"t4_dot_leading_coord", {"5"}, "t4_dot_leading_coord", exit_code::success, Stream::out, kT4DotLeadingCoord},

        // справка
        {"t4_help", {"--help"}, kEmptyInput, exit_code::success, Stream::out, kT4Help},
        {"t4_help_short", {"-h"}, kEmptyInput, exit_code::success, Stream::out, kT4Help},
        {"t4_help_extra", {"--help", "extra"}, kEmptyInput, exit_code::success, Stream::out, kT4Help},
        {"t4_help_short_extra", {"-h", "extra"}, kEmptyInput, exit_code::success, Stream::out, kT4Help},
    };
#ifndef _WIN32
    // io-fail: stdin Ч каталог, а не файл; под Windows такое не спровоцировать
    v.insert(v.begin(), Case{"t4_io", {"1"}, "", exit_code::io_fail, Stream::err, kIoFail, true});
#endif
    v.insert(v.end(), kVersionCases.begin(), kVersionCases.end());
    return v;
}
inline const std::vector<Case> t4_cases = make_t4_cases();
} // namespace e2e
