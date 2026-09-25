#include <stdio.h>

#include "../../common/string_utils.h"

// общий счётчик падений
static int failures = 0;

// сравнение - замечен ли hex-префикс, если он есть?
static void check(const char* in, int want) {
    int got = is_hex_prefix(in);
    if (got != want) {
        ++failures;
        fprintf(stderr, "FAIL: \"%s\"\n", in);
    }
}
// набор тест-кейсов
int main(void) {
    check("0x10", 1);
    check("0X10", 1);
    check("  0x10", 1);
    check("+0x10", 1);
    check("-0X10", 1);
    check("0x", 1);
    check("10", 0);
    check("0", 0);
    check("abc", 0);
    check("", 0);
    check("   ", 0);

    fprintf(stderr, "is_hex_prefix_c: %d failures\n", failures);
    return failures == 0 ? 0 : 1;
}
