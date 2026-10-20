// Generated from tests/expect/t2_*. Do not edit manually.
// CP1251 bytes must be preserved.
#pragma once

#include <string_view>

namespace e2e {

// t2_eof_name
inline constexpr std::string_view kT2EofName = R"E2E(EOF вместо имени фигуры)E2E";

// t2_empty_name
inline constexpr std::string_view kT2EmptyName = R"E2E(Пустой ввод вместо имени фигуры)E2E";

// t2_eof_vertexes
inline constexpr std::string_view kT2EofVertexes = R"E2E(EOF вместо числа вершин)E2E";

// t2_empty_vertexes
inline constexpr std::string_view kT2EmptyVertexes = R"E2E(Пустой ввод вместо кол-ва вершин)E2E";

// t2_fractional_vertexes
inline constexpr std::string_view kT2FractionalVertexes =
    R"E2E(Кол-во вершин должно быть целым числом. Получено: 3.3)E2E";

// t2_nan_vertexes
inline constexpr std::string_view kT2NanVertexes = R"E2E(Кол-во вершин должно быть целым числом. Получено: abc)E2E";

// t2_negative_vertexes
inline constexpr std::string_view kT2NegativeVertexes =
    R"E2E(Кол-во вершин должно быть положительным. Получено: -5)E2E";

// t2_double_plus
inline constexpr std::string_view kT2DoublePlus = R"E2E(Кол-во вершин должно быть целым числом. Получено: ++4)E2E";

// t2_plus_abc
inline constexpr std::string_view kT2PlusAbc = R"E2E(Кол-во вершин должно быть целым числом. Получено: +abc)E2E";

// t2_plus_minus
inline constexpr std::string_view kT2PlusMinus = R"E2E(Кол-во вершин должно быть целым числом. Получено: +-5)E2E";

// t2_plus_only
inline constexpr std::string_view kT2PlusOnly = R"E2E(Кол-во вершин должно быть целым числом. Получено: +)E2E";

// t2_overflow
inline constexpr std::string_view kT2Overflow =
    R"E2E(Кол-во вершин не помещается в 32-битное целое. Получено: 2147483648)E2E";

// t2_overflow_big
inline constexpr std::string_view kT2OverflowBig =
    R"E2E(Кол-во вершин не помещается в 32-битное целое. Получено: 3000000000)E2E";

// t2_overflow_neg
inline constexpr std::string_view kT2OverflowNeg =
    R"E2E(Кол-во вершин не помещается в 32-битное целое. Получено: -2147483649)E2E";

// t2_hex
inline constexpr std::string_view kT2Hex = R"E2E(Кол-во вершин должно быть целым числом. Получено: 0x10)E2E";

// t2_zero_vertexes
inline constexpr std::string_view kT2ZeroVertexes = R"E2E(Кол-во вершин должно быть положительным. Получено: 0)E2E";

// t2_norm
inline constexpr std::string_view kT2Norm = R"E2E(Фигура «звезда»: 6 вершин.)E2E";

// t2_leading_zero
inline constexpr std::string_view kT2LeadingZero = R"E2E(Фигура «звезда»: 9 вершин.)E2E";

// t2_name_with_spaces
inline constexpr std::string_view kT2NameWithSpaces = R"E2E(Фигура «правильный пятиугольник»: 5 вершин.)E2E";

// t2_one_vertex
inline constexpr std::string_view kT2OneVertex = R"E2E(Фигура «угол»: 1 вершина.)E2E";

// t2_help
inline constexpr std::string_view kT2Help = R"E2E(t2_passport: паспорт фигуры
Использование: [--help] [--version]
Вход (stdin): название фигуры (строка, может быть из нескольких слов), затем
целое число вершин V > 0.
Выводит: Фигура «<название>»: <V> <вершина|вершины|вершин>.
Коды возврата: 0 — успех; 65 — некорректные данные; 66 — нет входных данных.)E2E";

} // namespace e2e
