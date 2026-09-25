#include <cstdio>
#include <print>

#include "../../common/geometry.hpp"

// общий счётчик ошибок
static int failures = 0;
// строка с x y z, готовая точка из тех же x y z - сравниваем что получился одинак. объект
static void check_ok(std::string_view in, Point want) {
    auto got = parse_point(in);
    // если точки не образовалось или она не такова, как желаемая - несходство
    if (!got || *got != want) {
        ++failures;
        std::println(stderr, "FAIL ok: \"{}\"", in);
    }
}
// точка с ошибочными данными. смотрим чтобы код ошибки был определённый
static void check_err(std::string_view in, number_error want) {
    auto got = parse_point(in);
    // тут ошибка, потому точки получиться не должно. если она всё же получилась или код ошибки другой - несходство
    // если точка есть, то выполнение не дойдёт до got.error() - для существующей точки это вызвало бы ошибку
    // но при её существовании выполнение до туда не доходит
    if (got || got.error() != want) {
        ++failures;
        std::println(stderr, "FAIL err: \"{}\"", in);
    }
}
// все кейсы этого юнит=теста
int main() {
    // success
    check_ok("1 2 3", Point{1, 2, 3});
    check_ok("1.5 2.5 3.5", Point{1.5, 2.5, 3.5});
    check_ok(" 1.5 2.5 3.5 ", Point{1.5, 2.5, 3.5});
    check_ok("+1 2 3", Point{1, 2, 3});
    check_ok("3e+2 0 0", Point{300, 0, 0});
    check_ok("1E2 0 0", Point{100, 0, 0});
    check_ok("1e-2 0 0", Point{0.01, 0, 0});
    check_ok(".5 2 3", Point{0.5, 2, 3});
    check_ok("5. 2 3", Point{5, 2, 3});
    check_ok("0 0 0", Point{0, 0, 0});
    check_ok("1e-320 0 0", Point{1e-320, 0, 0});

    // too few coordinates
    check_err("1 2", number_error::too_few);
    check_err("1", number_error::too_few);
    check_err("", number_error::too_few);
    check_err("   ", number_error::too_few);

    // too many coordinates
    check_err("1 2 3 4", number_error::too_much);
    check_err("1 2 3 nan", number_error::too_much);

    // non-number
    check_err("1 2 x", number_error::not_number);
    check_err("abc 2 3", number_error::not_number);
    check_err("0x10 2 3", number_error::not_number);
    check_err("0X10 2 3", number_error::not_number);
    check_err("1,5 2 3", number_error::not_number);
    check_err("+-3 2 3", number_error::not_number);

    // out of double range
    check_err("1e400 0 0", number_error::out_of_range);
    check_err("1e-400 0 0", number_error::out_of_range);

    // non-finite
    check_err("inf 0 0", number_error::not_finite);
    check_err("nan 0 0", number_error::not_finite);
    check_err("infinity 0 0", number_error::not_finite);
    check_err("NAN(2) 0 0", number_error::not_finite);

    // regression: vertical tab / form feed before hex prefix
    check_err("\v0x10 2 3", number_error::not_number);
    check_err("\f0X10 2 3", number_error::not_number);

    std::println(stderr, "parse_point_cpp: {} failures", failures);
    return failures == 0 ? 0 : 1;
}
