#include <cmath>
#include <cstdio>
#include <print>

#include "../../common/geometry.hpp"

// счётчик ошибок
static int failures = 0;

// проверка вершины: ожидаемые x/y (допуск) и z == 0
static void check(int i, int n, double want_x, double want_y) {
    Point p = polygon_vertex(i, n);
    if (std::abs(p.x - want_x) > 1e-9 || std::abs(p.y - want_y) > 1e-9 || p.z != 0.0) {
        ++failures;
        std::println(stderr, "FAIL: i={} n={} -> ({:.6f},{:.6f},{:.6f}), want ({:.6f},{:.6f},0)", i, n, p.x, p.y, p.z,
                     want_x, want_y);
    }
}

int main() {
    // квадрат: вершины на осях
    check(0, 4, 1, 0);
    check(1, 4, 0, 1);
    check(2, 4, -1, 0);
    check(3, 4, 0, -1);

    // треугольник: 120-градусные шаги
    check(0, 3, 1, 0);
    check(1, 3, -0.5, 0.8660254037844386);
    check(2, 3, -0.5, -0.8660254037844386);

    std::println(stderr, "polygon_vertex_cpp: {} failures", failures);
    return failures == 0 ? 0 : 1;
}
