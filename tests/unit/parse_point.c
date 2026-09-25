#include <stdio.h>

#include "../../common/geometry.h"

// общий счётчик ошибок
static int failures = 0;
// строка с x y z, готовая точка из тех же x y z - сравниваем что получился одинак. объект
static void check_ok(const char* in, double x, double y, double z) {
    struct Point p = {0};
    int code = parse_point(in, &p);
    if (code != NUMBER_OK || p.x != x || p.y != y || p.z != z) {
        ++failures;
        fprintf(stderr, "FAIL ok: \"%s\"\n", in);
    }
}

// точка с ошибочными данными. смотрим чтобы код ошибки был определённый
static void check_err(const char* in, int want_code) {
    struct Point p = {0};
    int code = parse_point(in, &p);
    if (code != want_code) {
        ++failures;
        fprintf(stderr, "FAIL err: \"%s\"\n", in);
    }
}

// все кейсы этого юнит=теста
int main(void) {
    // success
    check_ok("1 2 3", 1, 2, 3);
    check_ok("1.5 2.5 3.5", 1.5, 2.5, 3.5);
    check_ok(" 1.5 2.5 3.5 ", 1.5, 2.5, 3.5);
    check_ok("+1 2 3", 1, 2, 3);
    check_ok("3e+2 0 0", 300, 0, 0);
    check_ok("1E2 0 0", 100, 0, 0);
    check_ok("1e-2 0 0", 0.01, 0, 0);
    check_ok(".5 2 3", 0.5, 2, 3);
    check_ok("5. 2 3", 5, 2, 3);
    check_ok("0 0 0", 0, 0, 0);
    check_ok("1e-320 0 0", 1e-320, 0, 0);

    // too few coordinates
    check_err("1 2", NUMBER_TOO_FEW);
    check_err("1", NUMBER_TOO_FEW);
    check_err("", NUMBER_TOO_FEW);
    check_err("   ", NUMBER_TOO_FEW);

    // too many coordinates
    check_err("1 2 3 4", NUMBER_TOO_MUCH);
    check_err("1 2 3 nan", NUMBER_TOO_MUCH);

    // non-number
    check_err("1 2 x", NUMBER_NOT_NUMBER);
    check_err("abc 2 3", NUMBER_NOT_NUMBER);
    check_err("0x10 2 3", NUMBER_NOT_NUMBER);
    check_err("0X10 2 3", NUMBER_NOT_NUMBER);
    check_err("1,5 2 3", NUMBER_NOT_NUMBER);
    check_err("+-3 2 3", NUMBER_NOT_NUMBER);

    // out of double range
    check_err("1e400 0 0", NUMBER_OUT_OF_RANGE);
    check_err("1e-400 0 0", NUMBER_OUT_OF_RANGE);

    // non-finite
    check_err("inf 0 0", NUMBER_NOT_FINITE);
    check_err("nan 0 0", NUMBER_NOT_FINITE);
    check_err("infinity 0 0", NUMBER_NOT_FINITE);
    check_err("NAN(2) 0 0", NUMBER_NOT_FINITE);

    // regression: vertical tab / form feed before hex prefix
    check_err("\v0x10 2 3", NUMBER_NOT_NUMBER);
    check_err("\f0X10 2 3", NUMBER_NOT_NUMBER);

    fprintf(stderr, "parse_point_c: %d failures\n", failures);
    return failures == 0 ? 0 : 1;
}
