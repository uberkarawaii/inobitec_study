#include <gtest/gtest.h>

#include <ostream>
#include <string>

#include "../../common/string_utils.h"

namespace {
// тип кейса: вход - строка, ожидание - 1 для пустой строки (только пробельные), иначе 0
struct Case {
    std::string in;
    int want;
};

// класс-фикстура (suite) - от него TEST_P создаст наследника,
// связанного с типом параметра - Case
class IsEmpty : public ::testing::TestWithParam<Case> {};

// макрос генератор шаблонного кода. TEST_P - с параметрами.
// имя класса-теста Detects
// TestBody возьмёт данные из GetParam()
TEST_P(IsEmpty, Detects) {
    const Case& c = GetParam();
    int got = is_empty(c.in.c_str());
    EXPECT_EQ(got, c.want);
}

// набор данных для suite IsEmpty: сколько Values - столько запусков Detects
INSTANTIATE_TEST_SUITE_P(Cases, IsEmpty,
                         ::testing::Values(Case{"", 1}, Case{"   ", 1}, Case{"\t\n", 1}, Case{"abc", 0}, Case{"  a", 0},
                                           Case{"a  ", 0}, Case{" a ", 0}, Case{"0", 0}));

// для gtest - "как напечатать данные типа Case" в диагностике
std::ostream& operator<<(std::ostream& os, const Case& c) {
    os << "in=\"" << c.in << "\", want=" << c.want;
    return os;
}
} // namespace
