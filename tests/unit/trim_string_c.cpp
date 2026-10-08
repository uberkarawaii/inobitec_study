#include <gtest/gtest.h>

#include <ostream>
#include <string>

#include "../../common/string_utils.h"

namespace {
// тип данных. вход - строка, want - ожидаемая обрезанная строка
struct Case {
    std::string in;
    std::string want;
};

// класс-фикстура (suite) для trim_string
class TrimStr : public ::testing::TestWithParam<Case> {};

// trim_string меняет буфер на месте, поэтому подаём изменяемую копию
TEST_P(TrimStr, Trims) {
    const Case& c = GetParam();
    // C-конвенция: строка-аргумент мутабельная + out-длина
    std::string buf = c.in;
    int len = static_cast<int>(buf.size());
    // возвращается указатель на начало обрезанного (мог сдвинуться вправо)
    char* got = trim_string(buf.data(), &len);
    EXPECT_STREQ(got, c.want.c_str());
}

INSTANTIATE_TEST_SUITE_P(Cases, TrimStr,
                         ::testing::Values(Case{"  abc  ", "abc"}, Case{"abc", "abc"}, Case{"   ", ""}, Case{"", ""},
                                           Case{" a ", "a"}, Case{"  a b  ", "a b"}, Case{"\tabc\n", "abc"}));

// для gtest - "как напечатать значение типа Case"
std::ostream& operator<<(std::ostream& os, const Case& c) {
    os << "in=\"" << c.in << "\", want=\"" << c.want << "\"";
    return os;
}
} // namespace
