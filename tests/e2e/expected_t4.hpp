// Generated from tests/expect/t4_*. Do not edit manually.
// CP1251 bytes must be preserved.
#pragma once

#include <string_view>

namespace e2e {

// t4_radius_empty
inline constexpr std::string_view kT4RadiusEmpty = R"E2E(Ожидался радиус; его значение не было введено
Для справки вызовите программу с параметром --help или -h)E2E";

// t4_radius_too_much
inline constexpr std::string_view kT4RadiusTooMuch = R"E2E(Ожидался радиус; были введены лишние аргументы
Для справки вызовите программу с параметром --help или -h)E2E";

// t4_radius_symbol
inline constexpr std::string_view kT4RadiusSymbol = R"E2E(Радиус должен быть числом. Получено: 4ch
Для справки вызовите программу с параметром --help или -h)E2E";

// t4_radius_inf
inline constexpr std::string_view kT4RadiusInf = R"E2E(Радиус должен быть конечным числом. Получено: inf
Для справки вызовите программу с параметром --help или -h)E2E";

// t4_radius_negative
inline constexpr std::string_view kT4RadiusNegative = R"E2E(Радиус должен быть положительным. Получено: -10
Для справки вызовите программу с параметром --help или -h)E2E";

// t4_radius_plus_plus
inline constexpr std::string_view kT4RadiusPlusPlus = R"E2E(Радиус должен быть числом. Получено: ++10
Для справки вызовите программу с параметром --help или -h)E2E";

// t4_radius_plus_abc
inline constexpr std::string_view kT4RadiusPlusAbc = R"E2E(Радиус должен быть числом. Получено: +abc
Для справки вызовите программу с параметром --help или -h)E2E";

// t4_radius_plus_minus
inline constexpr std::string_view kT4RadiusPlusMinus = R"E2E(Радиус должен быть числом. Получено: +-3
Для справки вызовите программу с параметром --help или -h)E2E";

// t4_radius_hex
inline constexpr std::string_view kT4RadiusHex = R"E2E(Радиус должен быть числом. Получено: 0x10
Для справки вызовите программу с параметром --help или -h)E2E";

// t4_radius_hex_upper
inline constexpr std::string_view kT4RadiusHexUpper = R"E2E(Радиус должен быть числом. Получено: 0X10
Для справки вызовите программу с параметром --help или -h)E2E";

// t4_radius_overflow
inline constexpr std::string_view kT4RadiusOverflow = R"E2E(Радиус выходит за допустимый диапазон: 1e400
Для справки вызовите программу с параметром --help или -h)E2E";

// t4_radius_underflow
inline constexpr std::string_view kT4RadiusUnderflow = R"E2E(Радиус выходит за допустимый диапазон: 1e-400
Для справки вызовите программу с параметром --help или -h)E2E";

// t4_radius_space_hex
inline constexpr std::string_view kT4RadiusSpaceHex = R"E2E(Радиус должен быть числом. Получено:    0x10
Для справки вызовите программу с параметром --help или -h)E2E";

// t4_radius_spaces
inline constexpr std::string_view kT4RadiusSpaces = R"E2E(Ожидался радиус; получена строка без значения
Для справки вызовите программу с параметром --help или -h)E2E";

// t4_radius_inner_space
inline constexpr std::string_view kT4RadiusInnerSpace = R"E2E(Радиус должен быть числом. Получено: 2 3
Для справки вызовите программу с параметром --help или -h)E2E";

// t4_radius_inner_quote
inline constexpr std::string_view kT4RadiusInnerQuote = R"E2E(Радиус должен быть числом. Получено: 2" 3
Для справки вызовите программу с параметром --help или -h)E2E";

// t4_radius_trailing_backslash
inline constexpr std::string_view kT4RadiusTrailingBackslash = R"E2E(Радиус должен быть числом. Получено: 10\
Для справки вызовите программу с параметром --help или -h)E2E";

// t4_radius_multi_quote
inline constexpr std::string_view kT4RadiusMultiQuote = R"E2E(Радиус должен быть числом. Получено: 2"3"
Для справки вызовите программу с параметром --help или -h)E2E";

// t4_symbol_coords
inline constexpr std::string_view kT4SymbolCoords = R"E2E(Строка 1. Нечисловые данные: 2 wow 3)E2E";

// t4_empty_coords
inline constexpr std::string_view kT4EmptyCoords = R"E2E(Точки отсутствуют)E2E";

// t4_plus_plus_coord
inline constexpr std::string_view kT4PlusPlusCoord = R"E2E(Строка 1. Нечисловые данные: 2 ++3 1)E2E";

// t4_plus_abc_coord
inline constexpr std::string_view kT4PlusAbcCoord = R"E2E(Строка 1. Нечисловые данные: 5 6 +abc)E2E";

// t4_test1
inline constexpr std::string_view kT4Test1 = R"E2E(3.000 4.000 5.000)E2E";

// t4_test2
inline constexpr std::string_view kT4Test2 = R"E2E(4.000 5.000 5.000)E2E";

// t4_test3
inline constexpr std::string_view kT4Test3 = R"E2E()E2E";

// t4_leading_spaces
inline constexpr std::string_view kT4LeadingSpaces = R"E2E(1.500 2.500 3.500)E2E";

// t4_scientific
inline constexpr std::string_view kT4Scientific = R"E2E(30.000 1.000 0.100)E2E";

// t4_subnormal_coord
inline constexpr std::string_view kT4SubnormalCoord = R"E2E(0.000 0.000 0.000)E2E";

// t4_exp_plus
inline constexpr std::string_view kT4ExpPlus = R"E2E(300.000 0.000 0.000)E2E";

// t4_exp_upper
inline constexpr std::string_view kT4ExpUpper = R"E2E(100.000 0.000 0.000)E2E";

// t4_exp_negative
inline constexpr std::string_view kT4ExpNegative = R"E2E(0.010 0.000 0.000)E2E";

// t4_dot_trailing_coord
inline constexpr std::string_view kT4DotTrailingCoord = R"E2E(5.000 2.000 3.000)E2E";

// t4_dot_leading_coord
inline constexpr std::string_view kT4DotLeadingCoord = R"E2E(0.500 2.000 3.000)E2E";

// t4_help
inline constexpr std::string_view kT4Help = R"E2E(t4_filter: фильтр точек по радиусу
Использование: <R> [--help] [--version]
R — радиус: вещественное, R > 0.
Вход (stdin): точки "x y z" (по одной в строке) до EOF.
Выводит в stdout только точки, расстояние от начала координат до которых меньше R.
Коды возврата: 0 — успех; 64 — неверные аргументы; 65 — некорректные данные; 66 — нет входных данных; 74 — сбой ввода-вывода.)E2E";

} // namespace e2e
