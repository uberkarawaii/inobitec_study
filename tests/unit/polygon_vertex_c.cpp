#include <gtest/gtest.h>

#include <ostream>

#include "../../common/geometry.h"

namespace {
// тип кейса: индекс вершины, число вершин и ожидаемые x/y (z всегда 0)
struct Case {
    int i;
    int n;
    double want_x;
    double want_y;
};

// класс-фикстура (suite), связанный с типом параметра Case
class PolygonVertex : public ::testing::TestWithParam<Case> {};

// TEST_P - параметризованный вариант TEST. имя теста Returns.
TEST_P(PolygonVertex, Returns) {
    const Case& c = GetParam();
    struct Point got = polygon_vertex(c.i, c.n);
    EXPECT_NEAR(got.x, c.want_x, 1e-9);
    EXPECT_NEAR(got.y, c.want_y, 1e-9);
    EXPECT_DOUBLE_EQ(got.z, 0.0);
}

// набор данных для suite PolygonVertex
INSTANTIATE_TEST_SUITE_P(Cases, PolygonVertex,
                         ::testing::Values(Case{0, 4, 1, 0}, Case{1, 4, 0, 1}, Case{2, 4, -1, 0}, Case{3, 4, 0, -1},
                                           Case{0, 3, 1, 0}, Case{1, 3, -0.5, 0.8660254037844386},
                                           Case{2, 3, -0.5, -0.8660254037844386}));

// для gtest - "как напечатать данные типа Case" в диагностике
std::ostream& operator<<(std::ostream& os, const Case& c) {
    os << "i=" << c.i << ", n=" << c.n << ", want=(" << c.want_x << ", " << c.want_y << ")";
    return os;
}
} // namespace
