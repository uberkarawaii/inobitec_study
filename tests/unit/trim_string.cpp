#include <gtest/gtest.h>

#include <ostream>
#include <string>

#include "../../common/string_utils.hpp"

namespace {
// тип кейса: вход - строка, ожидание - результат обрезки пробелов по краям
struct Case {
    std::string in;
    std::string want;
};

// класс-фикстура (suite), связанный с типом параметра Case
class TrimStr : public ::testing::TestWithParam<Case> {};

// trim_str меняет строку на месте, поэтому копируем вход
TEST_P(TrimStr, Trims) {
    const Case& c = GetParam();
    std::string got = c.in;
    trim_str(got);
    EXPECT_EQ(got, c.want);
}

// набор данных для suite TrimStr
INSTANTIATE_TEST_SUITE_P(Cases, TrimStr,
                         ::testing::Values(Case{"  abc  ", "abc"}, Case{"abc", "abc"}, Case{"   ", ""}, Case{"", ""},
                                           Case{" a ", "a"}, Case{"  a b  ", "a b"}, Case{"\tabc\n", "abc"}));

// для gtest - "как напечатать данные типа Case" в диагностике
std::ostream& operator<<(std::ostream& os, const Case& c) {
    os << "in=\"" << c.in << "\", want=\"" << c.want << "\"";
    return os;
}
} // namespace
