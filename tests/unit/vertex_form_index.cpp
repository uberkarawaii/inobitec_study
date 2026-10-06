#include <gtest/gtest.h>

#include <ostream>

#include "../../common/string_utils.hpp"

namespace {
// тип кейса: вход - число вершин, ожидание - индекс формы слова
struct Case {
    int in;
    int want;
};

// класс-фикстура (suite), связанный с типом параметра Case
class VertexFormIndex : public ::testing::TestWithParam<Case> {};

// TEST_P - параметризованный вариант TEST. имя теста Returns.
TEST_P(VertexFormIndex, Returns) {
    const Case& c = GetParam();
    int got = vertex_form_index(c.in);
    EXPECT_EQ(got, c.want);
}

// набор данных для suite VertexFormIndex
INSTANTIATE_TEST_SUITE_P(Cases, VertexFormIndex,
                         ::testing::Values(Case{1, 0}, Case{21, 0}, Case{101, 0}, Case{2, 1}, Case{3, 1}, Case{4, 1},
                                           Case{22, 1}, Case{24, 1}, Case{11, 2}, Case{12, 2}, Case{14, 2}, Case{5, 2},
                                           Case{10, 2}, Case{15, 2}, Case{20, 2}, Case{0, 2}, Case{100, 2},
                                           Case{111, 2}));

// для gtest - "как напечатать данные типа Case" в диагностике
std::ostream& operator<<(std::ostream& os, const Case& c) {
    os << "in=" << c.in << ", want=" << c.want;
    return os;
}
} // namespace
