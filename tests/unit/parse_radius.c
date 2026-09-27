#include <math.h>
#include <stdio.h>

#include "../../common/string_utils.h"

// счётчик ошибок
static int failures = 0;

// успешный разбор: код == OK и значение совпало (допуск)
static void check_ok(const char* in, double want) {
    double out = 0;
    int code = parse_radius(in, &out);
    if (code != NUMBER_OK || fabs(out - want) > 1e-9) {
        ++failures;
        fprintf(stderr, "FAIL ok: \"%s\" -> %d, %.3f\n", in, code, out);
    }
}

// ошибочный разбор: ожидаем конкретный код
static void check_err(const char* in, int want_code) {
    double out = 0;
    int code = parse_radius(in, &out);
    if (code != want_code) {
        ++failures;
        fprintf(stderr, "FAIL err: \"%s\" -> %d, want %d\n", in, code, want_code);
    }
}

int main(void) {
    // ok
    check_ok("5", 5.0);
    check_ok("5.5", 5.5);
    check_ok(" 5 ", 5.0);
    check_ok("+5", 5.0);
    check_ok("3e2", 300.0);
    check_ok("0.001", 0.001);

    // пустой ввод
    check_err("", NUMBER_EMPTY);
    check_err("   ", NUMBER_EMPTY);

    // не число
    check_err("abc", NUMBER_NOT_NUMBER);
    check_err("0x10", NUMBER_NOT_NUMBER);
    check_err("5x", NUMBER_NOT_NUMBER);
    check_err("5 5", NUMBER_NOT_NUMBER);

    // вне диапазона double
    check_err("1e400", NUMBER_OUT_OF_RANGE);
    check_err("1e-400", NUMBER_OUT_OF_RANGE);

    // не конечное
    check_err("inf", NUMBER_NOT_FINITE);
    check_err("nan", NUMBER_NOT_FINITE);

    // не положительное
    check_err("0", NUMBER_NOT_POSITIVE);
    check_err("-5", NUMBER_NOT_POSITIVE);

    fprintf(stderr, "parse_radius_c: %d failures\n", failures);
    return failures == 0 ? 0 : 1;
}
