#include <gtest/gtest.h>

#include <ostream>
#include <vector>

#include "../../common/geometry.h"
#include "printers_c.hpp"

namespace {
// тип кейса: облако точек и ожидаемый центроид
struct Case {
    std::vector<struct Point> pts;
    struct Point want;
};

// класс-фикстура (suite), связанный с типом параметра Case
class Centroid : public ::testing::TestWithParam<Case> {};

// TEST_P - параметризованный вариант TEST. имя теста Computes.
TEST_P(Centroid, Computes) {
    const Case& c = GetParam();
    struct Point got = centroid(c.pts.data(), static_cast<int>(c.pts.size()));
    EXPECT_NEAR(got.x, c.want.x, 1e-9);
    EXPECT_NEAR(got.y, c.want.y, 1e-9);
    EXPECT_NEAR(got.z, c.want.z, 1e-9);
}

// набор данных для suite Centroid
INSTANTIATE_TEST_SUITE_P(Cases, Centroid,
                         ::testing::Values(Case{{{5, -2, 3}}, {5, -2, 3}}, Case{{{1, 2, 3}, {-1, -2, -3}}, {0, 0, 0}},
                                           Case{{{0, 0, 0}, {3, 6, 9}, {0, 0, 0}}, {1, 2, 3}},
                                           Case{{{-4, -2, 0}, {-2, -4, 0}}, {-3, -3, 0}}));

// для gtest - "как напечатать данные типа Case" в диагностике
std::ostream& operator<<(std::ostream& os, const Case& c) {
    os << "pts=" << ::testing::PrintToString(c.pts) << ", want=" << ::testing::PrintToString(c.want);
    return os;
}
} // namespace
