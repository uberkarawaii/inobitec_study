#include <stdio.h>
#include <string.h>

#include "../../common/string_utils.h"

// общий счётчик падений
static int failures = 0;

// сравнение очищенной строки с тем, что должно получиться после очистки
static void check(const char* in, const char* want) {
    char buf[64];
    int len = (int)strlen(in);
    memcpy(buf, in, (size_t)len + 1);
    const char* res = trim_string(buf, &len);
    if (strcmp(res, want) != 0) {
        ++failures;
        fprintf(stderr, "FAIL: \"%s\" -> \"%s\", want \"%s\"\n", in, res, want);
    }
}

// набор тест-кейсов
int main(void) {
    check("  abc  ", "abc");
    check("abc", "abc");
    check("   ", "");
    check("", "");
    check(" a ", "a");
    check("  a b  ", "a b");
    check("\tabc\n", "abc");

    fprintf(stderr, "trim_string_c: %d failures\n", failures);
    return failures == 0 ? 0 : 1;
}
