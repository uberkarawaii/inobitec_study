#include <gtest/gtest.h>

#include <ostream>
#include <string>

#include "../../common/geometry.h"
#include "../../common/parse_codes.h"
#include "printers_c.hpp"

namespace {
// тип данных. вход в виде строки и ожидаемая точка. нормальный случай
struct OkCase {
    std::string in;
    struct Point want;
};

// класс-фикстура (suite). от него TEST_P строит наследника,
// параметризованный типом OkCase
class ParsePointOk : public ::testing::TestWithParam<OkCase> {};

// TEST_P - параметризованный TEST. второе имя (Parses) - имя теста
TEST_P(ParsePointOk, Parses) {
    const OkCase& c = GetParam();
    // буфер-приёмник, который заполнит C-функция
    struct Point p{};
    // C-конвенция: int-код + out-параметр через указатель
    int code = parse_point(c.in.c_str(), &p);
    // код верный -> p заполнена, дальше по полям (operator== у C-структуры нет)
    ASSERT_EQ(code, NUMBER_OK);
    EXPECT_EQ(p.x, c.want.x);
    EXPECT_EQ(p.y, c.want.y);
    EXPECT_EQ(p.z, c.want.z);
}

// набор данных для ParsePointOk; сколько наборов - столько и инстансов
INSTANTIATE_TEST_SUITE_P(OkCases, ParsePointOk,
                         ::testing::Values(OkCase{"1 2 3", {1, 2, 3}}, OkCase{"1.5 2.5 3.5", {1.5, 2.5, 3.5}},
                                           OkCase{" 1.5 2.5 3.5 ", {1.5, 2.5, 3.5}}, OkCase{"+1 2 3", {1, 2, 3}},
                                           OkCase{"3e+2 0 0", {300, 0, 0}}, OkCase{"1E2 0 0", {100, 0, 0}},
                                           OkCase{"1e-2 0 0", {0.01, 0, 0}}, OkCase{".5 2 3", {0.5, 2, 3}},
                                           OkCase{"5. 2 3", {5, 2, 3}}, OkCase{"0 0 0", {0, 0, 0}},
                                           OkCase{"1e-320 0 0", {1e-320, 0, 0}}));

// тип данных. вход в виде строки и ожидаемый код ошибки. ошибочный случай.
// want - int-код NUMBER_* (на C++-стороне был enum class number_error)
struct ErrCase {
    std::string in;
    int want;
};

class ParsePointErr : public ::testing::TestWithParam<ErrCase> {};

TEST_P(ParsePointErr, Rejects) {
    const ErrCase& c = GetParam();
    struct Point p{};
    int code = parse_point(c.in.c_str(), &p);
    EXPECT_EQ(code, c.want);
}

INSTANTIATE_TEST_SUITE_P(ErrCases, ParsePointErr,
                         ::testing::Values(
                             // мало координат
                             ErrCase{"1 2", NUMBER_TOO_FEW}, ErrCase{"1", NUMBER_TOO_FEW}, ErrCase{"", NUMBER_TOO_FEW},
                             ErrCase{"   ", NUMBER_TOO_FEW},

                             // лишние после координат
                             ErrCase{"1 2 3 4", NUMBER_TOO_MUCH}, ErrCase{"1 2 3 nan", NUMBER_TOO_MUCH},

                             // не число
                             ErrCase{"1 2 x", NUMBER_NOT_NUMBER}, ErrCase{"abc 2 3", NUMBER_NOT_NUMBER},
                             ErrCase{"0x10 2 3", NUMBER_NOT_NUMBER}, ErrCase{"0X10 2 3", NUMBER_NOT_NUMBER},
                             ErrCase{"1,5 2 3", NUMBER_NOT_NUMBER}, ErrCase{"+-3 2 3", NUMBER_NOT_NUMBER},

                             // вне диапазона double
                             ErrCase{"1e400 0 0", NUMBER_OUT_OF_RANGE}, ErrCase{"1e-400 0 0", NUMBER_OUT_OF_RANGE},

                             // не-конечное
                             ErrCase{"inf 0 0", NUMBER_NOT_FINITE}, ErrCase{"nan 0 0", NUMBER_NOT_FINITE},
                             ErrCase{"infinity 0 0", NUMBER_NOT_FINITE}, ErrCase{"NAN(2) 0 0", NUMBER_NOT_FINITE},

                             // регресс: \v / \f перед hex-префиксом
                             ErrCase{"\v0x10 2 3", NUMBER_NOT_NUMBER}, ErrCase{"\f0X10 2 3", NUMBER_NOT_NUMBER}));

// для gtest - "как напечатать значение типа OkCase"
// PrintToString(c.want) подхватит PrintTo(struct Point) из printers_c.hpp
std::ostream& operator<<(std::ostream& os, const OkCase& c) {
    os << "in=\"" << c.in << "\", want=" << ::testing::PrintToString(c.want);
    return os;
}

// для gtest - "как напечатать значение типа ErrCase"
std::ostream& operator<<(std::ostream& os, const ErrCase& c) {
    os << "in=\"" << c.in << "\", want=code " << c.want;
    return os;
}
} // namespace