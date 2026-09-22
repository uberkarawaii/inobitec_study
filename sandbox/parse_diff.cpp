// parse_diff.cpp - one-off probe: strtod vs std::from_chars on the same input
// not part of the repo; built by hand
#include <cerrno>
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <format>
#include <iostream>
#include <system_error>

namespace {

constexpr double kSentinel = -1.23456789012345e+200;  // to see if from_chars touched value

const char* ec_name(std::errc e) {
    if (e == std::errc{}) return "ok";
    if (e == std::errc::invalid_argument) return "invalid_argument";
    if (e == std::errc::result_out_of_range) return "result_out_of_range";
    return "other";
}

const char* fp_class(double v) {
    switch (std::fpclassify(v)) {
        case FP_ZERO:      return "zero";
        case FP_SUBNORMAL: return "subnormal";
        case FP_NORMAL:    return "normal";
        case FP_INFINITE:  return "inf";
        case FP_NAN:       return "nan";
        default:           return "?";
    }
}

void fc_line(const char* input, std::chars_format fmt, const char* name) {
    double v = kSentinel;
    const char* last = input + std::strlen(input);
    auto [ptr, ec] = std::from_chars(input, last, v, fmt);
    std::cout << std::format(
        "  from_chars {:<10} ec={:<19} value={:<24.17g} [{:a}] class={:<9} value_changed={:<3} ptr->'{}'\n",
        name, ec_name(ec), v, v, fp_class(v), (v != kSentinel) ? "yes" : "no", ptr);
}

void show(const char* input) {
    std::cout << std::format("input = '{}'\n", input);

    errno = 0;
    char* end = nullptr;
    const double sv = std::strtod(input, &end);
    std::cout << std::format(
        "  strtod               value={:<24.17g} [{:a}] class={:<9} consumed={:<3} rest='{}' errno={}\n",
        sv, sv, fp_class(sv), static_cast<long long>(end - input), end,
        errno == ERANGE ? "ERANGE" : "0");

    fc_line(input, std::chars_format::general,    "general");
    fc_line(input, std::chars_format::scientific, "scientific");
    fc_line(input, std::chars_format::fixed,      "fixed");
    fc_line(input, std::chars_format::hex,        "hex");
    std::cout << '\n';
}

}  // namespace

int main() {
    const char* cases[] = {
        "42", " +5", "+5",
        "5e2", "5e+2", "5e",
        "1e400", "1e-400", "1e-320",
        "0x10", "0x1p1", "1.8", "1.8p1",
        "inf", "nan", "abc", "infinity", "INFINITY",
        "NAN", "NAN(2)", "nan(2)", "5.", ".5", "1,5", "3e+2", "1E2",
    };
    for (const char* c : cases)
        show(c);
}