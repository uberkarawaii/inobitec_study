#include <cmath>
#include <cstdio>
#include <print>
#include <span>
#include <vector>

#include "../../common/geometry.hpp"

// счётчик ошибок
static int failures = 0;

// сверка с ожидаемым центроидом с допуском (вещественная арифметика)
static void check(std::span<const Point> pts, const Point& want) {
    Point got = centroid(pts);
    if (std::abs(got.x - want.x) > 1e-9 || std::abs(got.y - want.y) > 1e-9 || std::abs(got.z - want.z) > 1e-9) {
        ++failures;
        std::println(stderr, "FAIL: got ({:.3f},{:.3f},{:.3f}), want ({:.3f},{:.3f},{:.3f})", got.x, got.y, got.z,
                     want.x, want.y, want.z);
    }
}

int main() {
    // одна точка
    check(std::vector<Point>{{5, -2, 3}}, Point{5, -2, 3});

    // симметричный набор -> центр в нуле
    check(std::vector<Point>{{1, 2, 3}, {-1, -2, -3}}, Point{0, 0, 0});

    // среднее по трём точкам
    check(std::vector<Point>{{0, 0, 0}, {3, 6, 9}, {0, 0, 0}}, Point{1, 2, 3});

    // отрицательные координаты
    check(std::vector<Point>{{-4, -2, 0}, {-2, -4, 0}}, Point{-3, -3, 0});

    std::println(stderr, "centroid_cpp: {} failures", failures);
    return failures == 0 ? 0 : 1;
}
