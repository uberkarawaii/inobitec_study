// Generated from tests/expect/t_arg_unknown and t_arg_unknown_multi.
// Do not edit manually: CP1251 bytes must be preserved.
#pragma once

#include <string_view>

namespace e2e {

inline constexpr std::string_view kArgUnknown = R"E2E(Ожидался вызов без аргументов. Получено: --foo
Для справки вызовите программу с параметром --help или -h
)E2E";

inline constexpr std::string_view kArgUnknownMulti = R"E2E(Ожидался вызов без аргументов. Получено: --foo bar
Для справки вызовите программу с параметром --help или -h
)E2E";

} // namespace e2e
