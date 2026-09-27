#include <math.h>
#include <stdio.h>

#include "../../common/geometry.h"

// счётчик ошибок
static int failures = 0;

// сверка с ожидаемым центроидом с допуском (вещественная арифметика)
static void check(const struct Point* pts, int n, struct Point want) {
    struct Point got = centroid(pts, n);
    if (fabs(got.x - want.x) > 1e-9 || fabs(got.y - want.y) > 1e-9 || fabs(got.z - want.z) > 1e-9) {
        ++failures;
        fprintf(stderr, "FAIL: got (%.3f,%.3f,%.3f), want (%.3f,%.3f,%.3f)\n", got.x, got.y, got.z, want.x, want.y,
                want.z);
    }
}

int main(void) {
    // одна точка
    struct Point one[] = {{5, -2, 3}};
    check(one, 1, (struct Point){5, -2, 3});

    // симметричный набор -> центр в нуле
    struct Point sym[] = {{1, 2, 3}, {-1, -2, -3}};
    check(sym, 2, (struct Point){0, 0, 0});

    // среднее по трём точкам
    struct Point three[] = {{0, 0, 0}, {3, 6, 9}, {0, 0, 0}};
    check(three, 3, (struct Point){1, 2, 3});

    // отрицательные координаты
    struct Point neg[] = {{-4, -2, 0}, {-2, -4, 0}};
    check(neg, 2, (struct Point){-3, -3, 0});

    fprintf(stderr, "centroid_c: %d failures\n", failures);
    return failures == 0 ? 0 : 1;
}
