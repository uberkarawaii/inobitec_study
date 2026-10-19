#pragma once
#include <vector>

#include "../../common/exit_codes.hpp"
#include "common_cases.hpp"
#include "e2e_case.hpp"
#include "expected_t1.hpp"

namespace e2e {
inline std::vector<Case> make_t1_cases() {
    std::vector<Case> v = {
        // с ошибками
        {"t1_empty", {}, "t1_nul", exit_code::no_in, Stream::err, kT1Nul},
        {"t1_abc", {}, "t1_abc", exit_code::data, Stream::err, kT1Abc},
        {"t1_spaces", {}, "t1_spaces", exit_code::no_in, Stream::err, kT1Nul},
        {"t1_float", {}, "t1_float", exit_code::data, Stream::err, kT1Float},
        {"t1_high", {}, "t1_high", exit_code::data, Stream::err, kT1High},
        {"t1_low", {}, "t1_low", exit_code::data, Stream::err, kT1Low},
        {"t1_double_plus", {}, "t1_double_plus", exit_code::data, Stream::err, kT1DoublePlus},
        {"t1_plus_abc", {}, "t1_plus_abc", exit_code::data, Stream::err, kT1PlusAbc},
        {"t1_plus_minus", {}, "t1_plus_minus", exit_code::data, Stream::err, kT1PlusMinus},
        {"t1_plus_only", {}, "t1_plus_only", exit_code::data, Stream::err, kT1PlusOnly},
        {"t1_overflow", {}, "t1_overflow", exit_code::data, Stream::err, kT1Overflow},
        {"t1_overflow_big", {}, "t1_overflow_big", exit_code::data, Stream::err, kT1OverflowBig},
        {"t1_overflow_neg", {}, "t1_overflow_neg", exit_code::data, Stream::err, kT1OverflowNeg},
        {"t1_int32_max", {}, "t1_int32_max", exit_code::data, Stream::err, kT1Int32Max},
        {"t1_hex", {}, "t1_hex", exit_code::data, Stream::err, kT1Hex},

        // успех
        {"t1_norm", {}, "t1_test1", exit_code::success, Stream::out, kT1Test1},
        {"t1_plus", {}, "t1_plus", exit_code::success, Stream::out, kT1Test1},
        {"t1_leading_spaces", {}, "t1_leading_spaces", exit_code::success, Stream::out, kT1Test1},
        {"t1_test2", {}, "t1_test2", exit_code::success, Stream::out, kT1Test2},
        {"t1_space_plus", {}, "t1_space_plus", exit_code::success, Stream::out, kT1Test1},
        {"t1_leading_zero", {}, "t1_leading_zero", exit_code::success, Stream::out, kT1LeadingZero},

        // вывод справки
        {"t1_help", {"--help"}, kEmptyInput, exit_code::success, Stream::out, kT1Help},
        {"t1_help_short", {"-h"}, kEmptyInput, exit_code::success, Stream::out, kT1Help},
        {"t1_help_extra", {"--help", "extra"}, kEmptyInput, exit_code::success, Stream::out, kT1Help},
        {"t1_help_short_extra", {"-h", "extra"}, kEmptyInput, exit_code::success, Stream::out, kT1Help},
    };
    v.insert(v.end(), kVersionCases.begin(), kVersionCases.end());
    v.insert(v.end(), kUnknownArgCases.begin(), kUnknownArgCases.end());
    return v;
}
inline const std::vector<Case> t1_cases = make_t1_cases();
} // namespace e2e
