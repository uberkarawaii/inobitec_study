#include "string_utils.hpp"

#include <cctype>
#include <charconv>
#include <cstdint>
#include <expected>
#include <string>
#include <string_view>
#include <system_error>

// убрать пробелы слева и справа
void trim_str(std::string& s) {
    // первый слева не пробельный или служебный символ
    auto left = s.find_first_not_of(" \n\t\r");
    // если он указывает на конец строки
    if (left == std::string::npos) {
        s.clear();
        return;
    }
    s.erase(0, left);
    auto right = s.find_last_not_of(" \n\t\r");
    // если справа не было пробелов, то обрежется часть с индексом s.len(),
    // а её не существует т.к. это за строкой. т.е. ничего не обрежется
    s.erase(right + 1);
}

int is_empty(const std::string& s) {
    // первый слева не пробельный или служебный символ
    auto left = s.find_first_not_of(" \n\t\r");
    // указывает ли он на конец строки
    return left == std::string::npos;
}

// распознавание целого числа
std::expected<std::int32_t, int> parse_int32(std::string_view s) {
    const char* first = s.data();
    const char* last = s.data() + s.size();

    // ведущий + допустим только если сразу за ним цифра:
    // иначе from_chars после сдвига принял бы "+-5" как -5
    if (first != last && *first == '+') {
        ++first;
        if (first == last || !std::isdigit(static_cast<unsigned char>(*first)))
            return std::unexpected(int_parse_not_number);
    }

    std::int32_t v = 0;
    const auto [ptr, ec] = std::from_chars(first, last, v);

    if (ec == std::errc::result_out_of_range)
        return std::unexpected(int_parse_out_of_range);

    // неполное потребление ("5x", "5.5", "0x10") или нет преобразования ("", "x5")
    if (ec != std::errc{} || ptr != last)
        return std::unexpected(int_parse_not_number);

    return v;
}
