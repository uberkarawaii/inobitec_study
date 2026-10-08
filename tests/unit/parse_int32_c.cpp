#include <gtest/gtest.h>

#include <cstdint>
#include <ostream>
#include <string>

#include "../../common/parse_codes.h"
#include "../../common/string_utils.h"

namespace {
// тип кейса для ok-случая: вход - строка, ожидание - распознанное число
struct OkCase {
    std::string in;
    std::int32_t want;
};

// класс-фикстура (suite) для ok-кейсов, связанный с типом параметра OkCase
class ParseInt32Ok : public ::testing::TestWithParam<OkCase> {};

// TEST_P - параметризованный вариант TEST. имя теста Parses.
TEST_P(ParseInt32Ok, Parses) {
    const OkCase& c = GetParam();
    std::int32_t res = 0;
    int code = parse_int32(c.in.c_str(), &res);
    ASSERT_EQ(code, NUMBER_OK);
    EXPECT_EQ(res, c.want);
}

// набор данных для suite ParseInt32Ok
INSTANTIATE_TEST_SUITE_P(OkCases, ParseInt32Ok,
                         ::testing::Values(OkCase{"5", 5}, OkCase{"+7", 7}, OkCase{"-3", -3}, OkCase{"0", 0},
                                           OkCase{"007", 7}, OkCase{"2147483647", 2147483647},
                                           OkCase{"-2147483648", -2147483647 - 1}, OkCase{" 42 ", 42},
                                           OkCase{" +7 ", 7}));

// тип кейса для err-случая: вход - строка, ожидание - код ошибки
struct ErrCase {
    std::string in;
    int want;
};

// класс-фикстура (suite) для err-кейсов, связанный с типом параметра ErrCase
class ParseInt32Err : public ::testing::TestWithParam<ErrCase> {};

// TEST_P - параметризованный вариант TEST. имя теста Rejects.
TEST_P(ParseInt32Err, Rejects) {
    const ErrCase& c = GetParam();
    std::int32_t res = 0;
    int code = parse_int32(c.in.c_str(), &res);
    EXPECT_EQ(code, c.want);
}

// набор данных для suite ParseInt32Err
INSTANTIATE_TEST_SUITE_P(ErrCases, ParseInt32Err,
                         ::testing::Values(
                             // пустой ввод
                             ErrCase{"", NUMBER_EMPTY}, ErrCase{"   ", NUMBER_EMPTY}, ErrCase{"\t\n", NUMBER_EMPTY},

                             // не число
                             ErrCase{"abc", NUMBER_NOT_NUMBER}, ErrCase{"5x", NUMBER_NOT_NUMBER},
                             ErrCase{"5.5", NUMBER_NOT_NUMBER}, ErrCase{"0x10", NUMBER_NOT_NUMBER},
                             ErrCase{"5 5", NUMBER_NOT_NUMBER}, ErrCase{"+-5", NUMBER_NOT_NUMBER},
                             ErrCase{"++5", NUMBER_NOT_NUMBER}, ErrCase{"+", NUMBER_NOT_NUMBER},
                             ErrCase{"+abc", NUMBER_NOT_NUMBER},

                             // вне диапазона int32
                             ErrCase{"2147483648", NUMBER_OUT_OF_RANGE}, ErrCase{"-2147483649", NUMBER_OUT_OF_RANGE},
                             ErrCase{"3000000000", NUMBER_OUT_OF_RANGE}));

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
