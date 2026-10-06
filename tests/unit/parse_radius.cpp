#include <gtest/gtest.h>

#include <ostream>
#include <string_view>

#include "../../common/parse_codes.hpp"
#include "../../common/string_utils.hpp"

namespace {
// тип кейса для ok-случая: вход - строка, ожидание - распознанный радиус
struct OkCase {
    std::string_view in;
    double want;
};

// класс-фикстура (suite) для ok-кейсов, связанный с типом параметра OkCase
class ParseRadiusOk : public ::testing::TestWithParam<OkCase> {};

// TEST_P - параметризованный вариант TEST. имя теста Parses.
TEST_P(ParseRadiusOk, Parses) {
    const OkCase& c = GetParam();
    auto got = parse_radius(c.in);
    ASSERT_TRUE(got.has_value());
    EXPECT_NEAR(*got, c.want, 1e-9);
}

// набор данных для suite ParseRadiusOk
INSTANTIATE_TEST_SUITE_P(OkCases, ParseRadiusOk,
                         ::testing::Values(OkCase{"5", 5.0}, OkCase{"5.5", 5.5}, OkCase{" 5 ", 5.0}, OkCase{"+5", 5.0},
                                           OkCase{"3e2", 300.0}, OkCase{"0.001", 0.001}));

// тип кейса для err-случая: вход - строка, ожидание - код ошибки
struct ErrCase {
    std::string_view in;
    number_error want;
};

// класс-фикстура (suite) для err-кейсов, связанный с типом параметра ErrCase
class ParseRadiusErr : public ::testing::TestWithParam<ErrCase> {};

// TEST_P - параметризованный вариант TEST. имя теста Rejects.
TEST_P(ParseRadiusErr, Rejects) {
    const ErrCase& c = GetParam();
    auto got = parse_radius(c.in);
    ASSERT_FALSE(got.has_value());
    EXPECT_EQ(got.error(), c.want);
}

// набор данных для suite ParseRadiusErr
INSTANTIATE_TEST_SUITE_P(ErrCases, ParseRadiusErr,
                         ::testing::Values(
                             // пустой ввод
                             ErrCase{"", number_error::empty}, ErrCase{"   ", number_error::empty},

                             // не число
                             ErrCase{"abc", number_error::not_number}, ErrCase{"0x10", number_error::not_number},
                             ErrCase{"5x", number_error::not_number}, ErrCase{"5 5", number_error::not_number},

                             // вне диапазона double
                             ErrCase{"1e400", number_error::out_of_range},
                             ErrCase{"1e-400", number_error::out_of_range},

                             // не конечно
                             ErrCase{"inf", number_error::not_finite}, ErrCase{"nan", number_error::not_finite},

                             // не положительно
                             ErrCase{"0", number_error::not_positive}, ErrCase{"-5", number_error::not_positive}));

// для gtest - "как напечатать данные типа OkCase" в диагностике
std::ostream& operator<<(std::ostream& os, const OkCase& c) {
    os << "in=\"" << c.in << "\", want=" << c.want;
    return os;
}

// для gtest - "как напечатать данные типа ErrCase" в диагностике
std::ostream& operator<<(std::ostream& os, const ErrCase& c) {
    os << "in=\"" << c.in << "\", want=code " << static_cast<int>(c.want);
    return os;
}
} // namespace
