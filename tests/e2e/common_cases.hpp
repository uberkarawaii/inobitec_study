#pragma once

#include <string_view>
#include <vector>

#include "../../common/exit_codes.hpp"
#include "e2e_case.hpp"
#include "expected_common.hpp"
#include "version_expected.h" // содержит эталон версии в виде макроса E2E_EXPECTED_VERSION

namespace e2e {

// Общий пустой вход: для version/help/unknown stdin не читается.
inline constexpr std::string_view kEmptyInput = "t1_nul";

// общий сценарий тестов по версионированию - имя, args, пустой stdin, excode=0, осн.поток out, эталон(нагенерн CMake)
inline const std::vector<Case> kVersionCases = {
    {"version", {"--version"}, kEmptyInput, exit_code::success, Stream::out, E2E_EXPECTED_VERSION},
    {"version_extra", {"--version", "extra"}, kEmptyInput, exit_code::success, Stream::out, E2E_EXPECTED_VERSION},
};

// общ. сценарий тестов запуска программы с неизвестными аргументами
inline const std::vector<Case> kUnknownArgCases = {
    {"arg_unknown", {"--foo"}, kEmptyInput, exit_code::usage, Stream::err, kArgUnknown},
    {"arg_unknown_multi", {"--foo", "bar"}, kEmptyInput, exit_code::usage, Stream::err, kArgUnknownMulti},
};

} // namespace e2e
