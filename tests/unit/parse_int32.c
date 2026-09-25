#include <stdint.h>
#include <stdio.h>

#include "../../common/string_utils.h"

// общий счётчик падений
static int failures = 0;

// проверка - распарсилось ли число именно в нужное или нет
static void check_ok(const char* in, int32_t want) {
    int32_t v = 0;
    int code = parse_int32(in, &v);
    if (code != NUMBER_OK || v != want) {
        ++failures;
        fprintf(stderr, "FAIL ok: \"%s\"\n", in);
    }
}

// когда строка числа с ошибкой - проверка, что упало с нужным кодом
static void check_err(const char* in, int want_code) {
    int32_t v = 0;
    int code = parse_int32(in, &v);
    if (code != want_code) {
        ++failures;
        fprintf(stderr, "FAIL err: \"%s\"\n", in);
    }
}

// набор тест-кейсов
int main(void) {
    // ok
    check_ok("5", 5);
    check_ok("+7", 7);
    check_ok("-3", -3);
    check_ok("0", 0);
    check_ok("007", 7);
    check_ok("2147483647", 2147483647);
    check_ok("-2147483648", -2147483647 - 1);
    check_ok(" 42 ", 42);
    check_ok(" +7 ", 7);

    // empty
    check_err("", NUMBER_EMPTY);
    check_err("   ", NUMBER_EMPTY);
    check_err("\t\n", NUMBER_EMPTY);

    // not_number
    check_err("abc", NUMBER_NOT_NUMBER);
    check_err("5x", NUMBER_NOT_NUMBER);
    check_err("5.5", NUMBER_NOT_NUMBER);
    check_err("0x10", NUMBER_NOT_NUMBER);
    check_err("5 5", NUMBER_NOT_NUMBER);
    check_err("+-5", NUMBER_NOT_NUMBER);
    check_err("++5", NUMBER_NOT_NUMBER);
    check_err("+", NUMBER_NOT_NUMBER);
    check_err("+abc", NUMBER_NOT_NUMBER);

    // out_of_range
    check_err("2147483648", NUMBER_OUT_OF_RANGE);
    check_err("-2147483649", NUMBER_OUT_OF_RANGE);
    check_err("3000000000", NUMBER_OUT_OF_RANGE);

    fprintf(stderr, "parse_int32_c: %d failures\n", failures);
    return failures == 0 ? 0 : 1;
}
