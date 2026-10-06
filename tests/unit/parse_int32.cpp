#include <gtest/gtest.h>

#include <cstdint>
#include <ostream>
#include <string_view>

#include "../../common/parse_codes.hpp"
#include "../../common/string_utils.hpp"

namespace {
// тип кейса для ok-случая: вход - строка, ожидание - распознанное число
struct OkCase {
    std::string_view in;
    std::int32_t want;
};

// класс-фикстура (suite) для ok-кейсов, связанный с типом параметра OkCase
class ParseInt32Ok : public ::testing::TestWithParam<OkCase> {};

// TEST_P - параметризованный вариант TEST. имя теста Parses.
TEST_P(ParseInt32Ok, Parses) {
    const OkCase& c = GetParam();
    auto got = parse_int32(c.in);
    ASSERT_TRUE(got.has_value());
    EXPECT_EQ(*got, c.want);
}

// набор данных для suite ParseInt32Ok
INSTANTIATE_TEST_SUITE_P(OkCases, ParseInt32Ok,
                         ::testing::Values(OkCase{"5", 5}, OkCase{"+7", 7}, OkCase{"-3", -3}, OkCase{"0", 0},
                                           OkCase{"007", 7}, OkCase{"2147483647", 2147483647},
                                           OkCase{"-2147483648", -2147483647 - 1}, OkCase{" 42 ", 42},
                                           OkCase{" +7 ", 7}));

// тип кейса для err-случая: вход - строка, ожидание - код ошибки
struct ErrCase {
    std::string_view in;
    number_error want;
};

// класс-фикстура (suite) для err-кейсов, связанный с типом параметра ErrCase
class ParseInt32Err : public ::testing::TestWithParam<ErrCase> {};

// TEST_P - параметризованный вариант TEST. имя теста Rejects.
TEST_P(ParseInt32Err, Rejects) {
    const ErrCase& c = GetParam();
    auto got = parse_int32(c.in);
    ASSERT_FALSE(got.has_value());
    EXPECT_EQ(got.error(), c.want);
}

// набор данных для suite ParseInt32Err
INSTANTIATE_TEST_SUITE_P(ErrCases, ParseInt32Err,
                         ::testing::Values(
                             // пустой ввод
                             ErrCase{"", number_error::empty}, ErrCase{"   ", number_error::empty},
                             ErrCase{"\t\n", number_error::empty},

                             // не число
                             ErrCase{"abc", number_error::not_number}, ErrCase{"5x", number_error::not_number},
                             ErrCase{"5.5", number_error::not_number}, ErrCase{"0x10", number_error::not_number},
                             ErrCase{"5 5", number_error::not_number}, ErrCase{"+-5", number_error::not_number},
                             ErrCase{"++5", number_error::not_number}, ErrCase{"+", number_error::not_number},
                             ErrCase{"+abc", number_error::not_number},

                             // вне диапазона int32
                             ErrCase{"2147483648", number_error::out_of_range},
                             ErrCase{"-2147483649", number_error::out_of_range},
                             ErrCase{"3000000000", number_error::out_of_range}));

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
