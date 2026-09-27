#include <cmath>
#include <cstdio>
#include <print>

#include "../../common/string_utils.hpp"

// счётчик ошибок
static int failures = 0;

// успешный разбор: значение есть и совпало (допуск)
static void check_ok(std::string_view in, double want) {
    auto got = parse_radius(in);
    if (!got || std::abs(*got - want) > 1e-9) {
        ++failures;
        std::println(stderr, "FAIL ok: \"{}\"", in);
    }
}

// ошибочный разбор: ожидаем конкретную ошибку
static void check_err(std::string_view in, number_error want) {
    auto got = parse_radius(in);
    if (got || got.error() != want) {
        ++failures;
        std::println(stderr, "FAIL err: \"{}\"", in);
    }
}

int main() {
    // ok
    check_ok("5", 5.0);
    check_ok("5.5", 5.5);
    check_ok(" 5 ", 5.0);
    check_ok("+5", 5.0);
    check_ok("3e2", 300.0);
    check_ok("0.001", 0.001);

    // пустой ввод
    check_err("", number_error::empty);
    check_err("   ", number_error::empty);

    // не число
    check_err("abc", number_error::not_number);
    check_err("0x10", number_error::not_number);
    check_err("5x", number_error::not_number);
    check_err("5 5", number_error::not_number);

    // вне диапазона double
    check_err("1e400", number_error::out_of_range);
    check_err("1e-400", number_error::out_of_range);

    // не конечное
    check_err("inf", number_error::not_finite);
    check_err("nan", number_error::not_finite);

    // не положительное
    check_err("0", number_error::not_positive);
    check_err("-5", number_error::not_positive);

    std::println(stderr, "parse_radius_cpp: {} failures", failures);
    return failures == 0 ? 0 : 1;
}
