// parse_diff_int.cpp � ��������� strtol(base=10) � std::from_chars<int>
// ������ ��� ����; Windows: cl /std:c++latest /W4 /EHsc parse_diff_int.cpp
//                        Linux:  g++ -std=c++23 -Wall -Wextra -O0 parse_diff_int.cpp -o parse_diff_int

#include <cerrno>
#include <charconv>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace {

std::string quote(std::string_view s) {
    std::string out = "\"";
    for (char c : s) {
        if (c == '\t')
            out += "\\t";
        else if (c == '\n')
            out += "\\n";
        else if (c == '\r')
            out += "\\r";
        else
            out += c;
    }
    out += "\"";
    return out;
}

const char* ec_name(std::errc e) {
    if (e == std::errc{})
        return "ok";
    if (e == std::errc::invalid_argument)
        return "invalid_argument";
    if (e == std::errc::result_out_of_range)
        return "out_of_range";
    return "other";
}

constexpr int kSentinel = -123456789; // to see if from_chars touches value

} // namespace

int main() {
    const std::vector<std::string> cases = {
        "",
        " ",
        "5",
        " 5",
        "5 ",
        "+5",
        "+ 5",
        "-5",
        "- 5",
        "5.5",
        ".5",
        "5x",
        "x5",
        "010",
        "0",
        "0x10",
        "0X10",
        "2147483647",
        "2147483648",
        "2147483649",
        "3000000000",
        "9223372036854775807",
        "9223372036854775808",
        "-2147483648",
        "-2147483649",
        "-3000000000",
        "-9223372036854775808",
        "-9223372036854775809",
        "99999999999999999999999",
        "+5 ",
        "  +5",
        "  -5",
        "++5",
        "+-5",
        "-+5",
        "+abc",
        "-abc",
        "+",
        "-",
        "+.5",
    };

    std::printf("sizeof(int)=%zu sizeof(long)=%zu sizeof(int32_t)=%zu\n\n", sizeof(int), sizeof(long),
                sizeof(std::int32_t));

    for (const std::string& s : cases) {
        // 1) C-�����: strtol, base 10
        errno = 0;
        char* end = nullptr;
        const long v = std::strtol(s.c_str(), &end, 10);
        const bool st_conv = (end != s.c_str());
        const bool st_full = st_conv && (*end == '\0');
        const bool st_erange = (errno == ERANGE);

        const char* first = s.data();
        const char* last = s.data() + s.size();

        // 2) from_chars ����� (��� ������� ������ �����)
        int raw = kSentinel;
        const auto [p_raw, ec_raw] = std::from_chars(first, last, raw);
        const bool raw_full = (ec_raw == std::errc{}) && (p_raw == last);
        const bool raw_changed = (raw != kSentinel);

        // 3) from_chars � ������ ������� '+' (��� ������ � t1/t2)
        int skip = kSentinel;
        const char* first_skip = first;
        if (first_skip != last && *first_skip == '+')
            ++first_skip;
        const auto [p_skip, ec_skip] = std::from_chars(first_skip, last, skip);
        const bool skip_full = (ec_skip == std::errc{}) && (p_skip == last);
        const bool skip_changed = (skip != kSentinel);

        std::printf("in=%-26s | strtol: v=%-20ld conv=%d full=%d ERANGE=%d rem=%-10s "
                    "| fc-raw: ec=%-16s v=%-11d full=%d chg=%d | fc+skip: ec=%-16s v=%-11d full=%d chg=%d\n",
                    quote(s).c_str(), v, st_conv, st_full, st_erange, quote(end).c_str(), ec_name(ec_raw), raw,
                    raw_full, raw_changed, ec_name(ec_skip), skip, skip_full, skip_changed);
    }

    return 0;
}