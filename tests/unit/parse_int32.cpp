#include <cstdio>
#include <print>

#include "../../common/string_utils.hpp"

// общий счётчик падений
static int failures = 0;

// проверка - распарсилось ли число именно в нужное или нет
static void check_ok(std::string_view in, std::int32_t want) {
    auto got = parse_int32(in);
    if (!got || *got != want) {
        ++failures;
        std::println(stderr, "FAIL ok: \"{}\"", in);
    }
}

// когда строка числа с ошибкой - проверка, что упало с нужным кодом
static void check_err(std::string_view in, number_error want) {
    auto got = parse_int32(in);
    if (got || got.error() != want) {
        ++failures;
        std::println(stderr, "FAIL err: \"{}\"", in);
    }
}

// набор тест-кейсов
int main() {
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
    check_err("", number_error::empty);
    check_err("   ", number_error::empty);
    check_err("\t\n", number_error::empty);

    // not_number
    check_err("abc", number_error::not_number);
    check_err("5x", number_error::not_number);
    check_err("5.5", number_error::not_number);
    check_err("0x10", number_error::not_number);
    check_err("5 5", number_error::not_number);
    check_err("+-5", number_error::not_number);
    check_err("++5", number_error::not_number);
    check_err("+", number_error::not_number);
    check_err("+abc", number_error::not_number);

    // out_of_range
    check_err("2147483648", number_error::out_of_range);
    check_err("-2147483649", number_error::out_of_range);
    check_err("3000000000", number_error::out_of_range);

    std::println(stderr, "parse_int32_cpp: {} failures", failures);
    return failures == 0 ? 0 : 1;
}
