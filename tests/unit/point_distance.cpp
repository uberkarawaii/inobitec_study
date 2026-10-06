#include <gtest/gtest.h>

#include <ostream>

#include "../../common/geometry.hpp"
#include "printers.hpp"

namespace {
// тип кейса: две точки и ожидаемое расстояние между ними
struct Case {
    Point a;
    Point b;
    double want;
};

// класс-фикстура (suite), связанный с типом параметра Case
class PointDistance : public ::testing::TestWithParam<Case> {};

// TEST_P - параметризованный вариант TEST. имя теста Distance.
TEST_P(PointDistance, Distance) {
    const Case& c = GetParam();
    double got = point_distance(c.a, c.b);
    EXPECT_NEAR(got, c.want, 1e-9);
}

// набор данных для suite PointDistance
INSTANTIATE_TEST_SUITE_P(
    Cases, PointDistance,
    ::testing::Values(Case{Point{0, 0, 0}, Point{0, 0, 0}, 0.0}, Case{Point{1, 2, 3}, Point{1, 2, 3}, 0.0},
                      Case{Point{0, 0, 0}, Point{3, 0, 0}, 3.0}, Case{Point{0, 0, 0}, Point{0, 4, 0}, 4.0},
                      Case{Point{0, 0, 0}, Point{0, 0, 7}, 7.0}, Case{Point{0, 0, 0}, Point{3, 4, 0}, 5.0},
                      Case{Point{1, 2, 0}, Point{4, 6, 0}, 5.0}, Case{Point{-1, -2, -2}, Point{0, 0, 0}, 3.0},
                      Case{Point{1, 2, 2}, Point{0, 0, 0}, 3.0}));

// для gtest - "как напечатать данные типа Case" в диагностике
std::ostream& operator<<(std::ostream& os, const Case& c) {
    os << "a=" << ::testing::PrintToString(c.a) << ", b=" << ::testing::PrintToString(c.b) << ", want=" << c.want;
    return os;
}
} // namespace
