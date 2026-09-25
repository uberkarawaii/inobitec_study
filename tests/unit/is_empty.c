#include <stdio.h>

#include "../../common/string_utils.h"

// общий счётчик падений
static int failures = 0;
// сравнивает код, который получился от is_empty на строке с тем, который должен был получиться
static void check(const char* in, int want) {
    int got = is_empty(in);
    if (got != want) {
        ++failures;
        fprintf(stderr, "FAIL: \"%s\"\n", in);
    }
}
// набор тест-кейсов
int main(void) {
    check("", 1);
    check("   ", 1);
    check("\t\n", 1);
    check("abc", 0);
    check("  a", 0);
    check("a  ", 0);
    check(" a ", 0);
    check("0", 0);

    fprintf(stderr, "is_empty_c: %d failures\n", failures);
    return failures == 0 ? 0 : 1;
}
