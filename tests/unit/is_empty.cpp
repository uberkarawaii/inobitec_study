#include <cstdio>
#include <print>
#include <string>

#include "../../common/string_utils.hpp"

// общий счётчик падений
static int failures = 0;

// сравнивает код, который получился от is_empty на строке с тем, который должен был получиться
static void check(const std::string& in, int want) {
    int got = is_empty(in);
    if (got != want) {
        ++failures;
        std::println(stderr, "FAIL: \"{}\"", in);
    }
}

// набор тест-кейсов
int main() {
    check("", 1);
    check("   ", 1);
    check("\t\n", 1);
    check("abc", 0);
    check("  a", 0);
    check("a  ", 0);
    check(" a ", 0);
    check("0", 0);

    std::println(stderr, "is_empty_cpp: {} failures", failures);
    return failures == 0 ? 0 : 1;
}
