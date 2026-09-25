#include <cstdio>
#include <print>
#include <string>

#include "../../common/string_utils.hpp"

// общий счётчик падений
static int failures = 0;

// сравнение очищенной строки с тем, что должно получиться после очистки
static void check(const std::string& in, const std::string& want) {
    std::string got = in;
    trim_str(got);
    if (got != want) {
        ++failures;
        std::println(stderr, "FAIL: \"{}\" -> \"{}\", want \"{}\"", in, got, want);
    }
}

// набор тест-кейсов
int main() {
    check("  abc  ", "abc");
    check("abc", "abc");
    check("   ", "");
    check("", "");
    check(" a ", "a");
    check("  a b  ", "a b");
    check("\tabc\n", "abc");

    std::println(stderr, "trim_string_cpp: {} failures", failures);
    return failures == 0 ? 0 : 1;
}
