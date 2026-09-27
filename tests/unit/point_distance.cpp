#include <cmath>
#include <cstdio>
#include <print>

#include "../../common/geometry.hpp"

// счётчик ошибок
static int failures = 0;

// сверка с ожидаемым расстоянием с допуском (вещественная арифметика)
static void check(const Point& a, const Point& b, double want) {
    double got = point_distance(a, b);
    if (std::abs(got - want) > 1e-9) {
        ++failures;
        std::println(stderr, "FAIL: ({:.3f},{:.3f},{:.3f})-({:.3f},{:.3f},{:.3f}) -> {:.3f}, want {:.3f}", a.x, a.y,
                     a.z, b.x, b.y, b.z, got, want);
    }
}

int main() {
    const Point o{0, 0, 0};

    // нулевое расстояние (совпадающие точки)
    check(o, o, 0.0);
    check(Point{1, 2, 3}, Point{1, 2, 3}, 0.0);

    // вырождение по одной оси
    check(o, Point{3, 0, 0}, 3.0);
    check(o, Point{0, 4, 0}, 4.0);
    check(o, Point{0, 0, 7}, 7.0);

    // 3-4-5 в плоскости
    check(o, Point{3, 4, 0}, 5.0);
    check(Point{1, 2, 0}, Point{4, 6, 0}, 5.0);

    // отрицательные координаты: sqrt(1+4+4) = 3
    check(Point{-1, -2, -2}, o, 3.0);

    // трёхмерный случай: sqrt(1+4+4) = 3
    check(Point{1, 2, 2}, o, 3.0);

    std::println(stderr, "point_distance_cpp: {} failures", failures);
    return failures == 0 ? 0 : 1;
}
