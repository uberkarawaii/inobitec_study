#include <gtest/gtest.h>

#include <ostream>
#include <string>

#include "../../common/parse_codes.h"
#include "../../common/string_utils.h"

namespace {
// тип кейса для ok-случая: вход - строка, ожидание - распознанный радиус
struct OkCase {
    std::string in;
    double want;
};

// класс-фикстура (suite) для ok-кейсов, связанный с типом параметра OkCase
class ParseRadiusOk : public ::testing::TestWithParam<OkCase> {};

// TEST_P - параметризованный вариант TEST. имя теста Parses.
TEST_P(ParseRadiusOk, Parses) {
    const OkCase& c = GetParam();
    double res = 0;
    int code = parse_radius(c.in.c_str(), &res);
    ASSERT_EQ(code, NUMBER_OK);
    EXPECT_NEAR(res, c.want, 1e-9);
}

// набор данных для suite ParseRadiusOk
INSTANTIATE_TEST_SUITE_P(OkCases, ParseRadiusOk,
                         ::testing::Values(OkCase{"5", 5.0}, OkCase{"5.5", 5.5}, OkCase{" 5 ", 5.0}, OkCase{"+5", 5.0},
                                           OkCase{"3e2", 300.0}, OkCase{"0.001", 0.001}));

// тип кейса для err-случая: вход - строка, ожидание - код ошибки
struct ErrCase {
    std::string in;
    int want;
};

// класс-фикстура (suite) для err-кейсов, связанный с типом параметра ErrCase
class ParseRadiusErr : public ::testing::TestWithParam<ErrCase> {};

// TEST_P - параметризованный вариант TEST. имя теста Rejects.
TEST_P(ParseRadiusErr, Rejects) {
    const ErrCase& c = GetParam();
    double res = 0;
    int code = parse_radius(c.in.c_str(), &res);
    EXPECT_EQ(code, c.want);
}

// набор данных для suite ParseRadiusErr
INSTANTIATE_TEST_SUITE_P(ErrCases, ParseRadiusErr,
                         ::testing::Values(
                             // пустой ввод
                             ErrCase{"", NUMBER_EMPTY}, ErrCase{"   ", NUMBER_EMPTY},

                             // не число
                             ErrCase{"abc", NUMBER_NOT_NUMBER}, ErrCase{"0x10", NUMBER_NOT_NUMBER},
                             ErrCase{"5x", NUMBER_NOT_NUMBER}, ErrCase{"5 5", NUMBER_NOT_NUMBER},

                             // вне диапазона double
                             ErrCase{"1e400", NUMBER_OUT_OF_RANGE}, ErrCase{"1e-400", NUMBER_OUT_OF_RANGE},

                             // не конечно
                             ErrCase{"inf", NUMBER_NOT_FINITE}, ErrCase{"nan", NUMBER_NOT_FINITE},

                             // не положительно
                             ErrCase{"0", NUMBER_NOT_POSITIVE}, ErrCase{"-5", NUMBER_NOT_POSITIVE}));

// для gtest - "как напечатать данные типа OkCase" в диагностике
std::ostream& operator<<(std::ostream& os, const OkCase& c) {
    os << "in=\"" << c.in << "\", want=" << c.want;
    return os;
}

// для gtest - "как напечатать данные типа ErrCase" в диагностике
std::ostream& operator<<(std::ostream& os, const ErrCase& c) {
    os << "in=\"" << c.in << "\", want=code " << c.want;
    return os;
}
} // namespace
