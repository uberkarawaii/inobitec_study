#include <stdio.h>

#include "../../common/string_utils.h"

// счётчик ошибок
static int failures = 0;

// сверка индекса формы
static void check(int n, int want) {
    int got = vertex_form_index(n);
    if (got != want) {
        ++failures;
        fprintf(stderr, "FAIL: %d -> %d, want %d\n", n, got, want);
    }
}

int main(void) {
    // одна вершина
    check(1, 0);
    check(21, 0);
    check(101, 0);

    // две-четыре вершины
    check(2, 1);
    check(3, 1);
    check(4, 1);
    check(22, 1);
    check(24, 1);

    // исключение: 11-14
    check(11, 2);
    check(12, 2);
    check(14, 2);

    // пять и больше / ноль
    check(5, 2);
    check(10, 2);
    check(15, 2);
    check(20, 2);
    check(0, 2);
    check(100, 2);
    check(111, 2);

    fprintf(stderr, "vertex_form_index_c: %d failures\n", failures);
    return failures == 0 ? 0 : 1;
}
