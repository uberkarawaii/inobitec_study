#pragma once
#include <vector>

#include "../../common/exit_codes.hpp"
#include "common_cases.hpp"
#include "e2e_case.hpp"
#include "expected_t2.hpp"

namespace e2e {
inline std::vector<Case> make_t2_cases() {
    std::vector<Case> v = {
        // входные данные с ошибками
        {"t2_eof_name", {}, "t2_eof_name", exit_code::no_in, Stream::err, kT2EofName},
        {"t2_empty_name", {}, "t2_empty_name", exit_code::no_in, Stream::err, kT2EmptyName},
        {"t2_eof_vertexes", {}, "t2_eof_vertexes", exit_code::no_in, Stream::err, kT2EofVertexes},
        {"t2_empty_vertexes", {}, "t2_empty_vertexes", exit_code::no_in, Stream::err, kT2EmptyVertexes},
        {"t2_fractional_vertexes", {}, "t2_fractional_vertexes", exit_code::data, Stream::err, kT2FractionalVertexes},
        {"t2_nan_vertexes", {}, "t2_nan_vertexes", exit_code::data, Stream::err, kT2NanVertexes},
        {"t2_negative_vertexes", {}, "t2_negative_vertexes", exit_code::data, Stream::err, kT2NegativeVertexes},
        {"t2_double_plus", {}, "t2_double_plus", exit_code::data, Stream::err, kT2DoublePlus},
        {"t2_plus_abc", {}, "t2_plus_abc", exit_code::data, Stream::err, kT2PlusAbc},
        {"t2_plus_minus", {}, "t2_plus_minus", exit_code::data, Stream::err, kT2PlusMinus},
        {"t2_plus_only", {}, "t2_plus_only", exit_code::data, Stream::err, kT2PlusOnly},
        {"t2_overflow", {}, "t2_overflow", exit_code::data, Stream::err, kT2Overflow},
        {"t2_overflow_big", {}, "t2_overflow_big", exit_code::data, Stream::err, kT2OverflowBig},
        {"t2_overflow_neg", {}, "t2_overflow_neg", exit_code::data, Stream::err, kT2OverflowNeg},
        {"t2_hex", {}, "t2_hex", exit_code::data, Stream::err, kT2Hex},
        {"t2_zero_vertexes", {}, "t2_zero_vertexes", exit_code::data, Stream::err, kT2ZeroVertexes},

        // нормальные входные данные
        {"t2_norm", {}, "t2_input", exit_code::success, Stream::out, kT2Norm},
        {"t2_plus", {}, "t2_plus", exit_code::success, Stream::out, kT2Norm},
        {"t2_leading_spaces", {}, "t2_leading_spaces", exit_code::success, Stream::out, kT2Norm},
        {"t2_space_plus", {}, "t2_space_plus", exit_code::success, Stream::out, kT2Norm},
        {"t2_leading_zero", {}, "t2_leading_zero", exit_code::success, Stream::out, kT2LeadingZero},
        {"t2_name_with_spaces", {}, "t2_name_with_spaces", exit_code::success, Stream::out, kT2NameWithSpaces},
        {"t2_one_vertex", {}, "t2_one_vertex", exit_code::success, Stream::out, kT2OneVertex},

        // справка
        {"t2_help", {"--help"}, kEmptyInput, exit_code::success, Stream::out, kT2Help},
        {"t2_help_short", {"-h"}, kEmptyInput, exit_code::success, Stream::out, kT2Help},
        {"t2_help_extra", {"--help", "extra"}, kEmptyInput, exit_code::success, Stream::out, kT2Help},
        {"t2_help_short_extra", {"-h", "extra"}, kEmptyInput, exit_code::success, Stream::out, kT2Help},
    };
    v.insert(v.end(), kVersionCases.begin(), kVersionCases.end());
    v.insert(v.end(), kUnknownArgCases.begin(), kUnknownArgCases.end());
    return v;
}
inline const std::vector<Case> t2_cases = make_t2_cases();
} // namespace e2e
