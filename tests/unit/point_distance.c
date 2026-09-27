#include <math.h>
#include <stdio.h>

#include "../../common/geometry.h"

// счётчик ошибок
static int failures = 0;

// сверка с ожидаемым расстоянием с допуском (вещественная арифметика)
static void check(struct Point a, struct Point b, double want) {
    double got = point_distance(a, b);
    if (fabs(got - want) > 1e-9) {
        ++failures;
        fprintf(stderr, "FAIL: (%.3f,%.3f,%.3f)-(%.3f,%.3f,%.3f) -> %.3f, want %.3f\n", a.x, a.y, a.z, b.x, b.y, b.z,
                got, want);
    }
}

int main(void) {
    struct Point o = {0, 0, 0};

    // нулевое расстояние (совпадающие точки)
    check(o, o, 0.0);
    check((struct Point){1, 2, 3}, (struct Point){1, 2, 3}, 0.0);

    // вырождение по одной оси
    check(o, (struct Point){3, 0, 0}, 3.0);
    check(o, (struct Point){0, 4, 0}, 4.0);
    check(o, (struct Point){0, 0, 7}, 7.0);

    // 3-4-5 в плоскости
    check(o, (struct Point){3, 4, 0}, 5.0);
    check((struct Point){1, 2, 0}, (struct Point){4, 6, 0}, 5.0);

    // отрицательные координаты: sqrt(1+4+4) = 3
    check((struct Point){-1, -2, -2}, o, 3.0);

    // трёхмерный случай: sqrt(1+4+4) = 3
    check((struct Point){1, 2, 2}, o, 3.0);

    fprintf(stderr, "point_distance_c: %d failures\n", failures);
    return failures == 0 ? 0 : 1;
}
