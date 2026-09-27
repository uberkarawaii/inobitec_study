#include "string_utils.hpp"

#include <cctype>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <expected>
#include <string>
#include <string_view>
#include <system_error>

#include "parse_codes.hpp"

// при переводе на isspace - новые вспомогат. методы
namespace {
std::size_t find_first_not_space(const std::string& s) {
    std::size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i])))
        ++i;
    return i == s.size() ? std::string::npos : i;
}
std::size_t find_last_not_space(const std::string& s) {
    std::size_t i = s.size();
    while (i > 0 && std::isspace(static_cast<unsigned char>(s[i - 1])))
        --i;
    return i == 0 ? std::string::npos : i - 1;
}
} // namespace

// убрать пробелы слева и справа
void trim_str(std::string& s) {
    // первый слева не пробельный или служебный символ
    auto left = find_first_not_space(s);
    // если он указывает на конец строки
    if (left == std::string::npos) {
        s.clear();
        return;
    }
    s.erase(0, left);
    auto right = find_last_not_space(s);
    // если справа не было пробелов, то обрежется часть с индексом s.len(),
    // а её не существует т.к. это за строкой. т.е. ничего не обрежется
    s.erase(right + 1);
}

int is_empty(const std::string& s) {
    // первый слева не пробельный или служебный символ
    auto left = find_first_not_space(s);
    // указывает ли он на конец строки
    return left == std::string::npos;
}

// распознавание целого числа
std::expected<std::int32_t, number_error> parse_int32(std::string_view s) {
    // пропуск краевых пробелов
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front())))
        s.remove_prefix(1);
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back())))
        s.remove_suffix(1);
    // если после очистки символов больше нет, строка пустая
    if (s.empty())
        return std::unexpected(number_error::empty);

    const char* first = s.data();
    const char* last = s.data() + s.size();

    // ведущий + допустим только если сразу за ним цифра:
    // иначе from_chars после сдвига принял бы "+-5" как -5
    if (first != last && *first == '+') {
        ++first;
        if (first == last || !std::isdigit(static_cast<unsigned char>(*first)))
            return std::unexpected(number_error::not_number);
    }

    std::int32_t v = 0;
    const auto [ptr, ec] = std::from_chars(first, last, v);

    if (ec == std::errc::result_out_of_range)
        return std::unexpected(number_error::out_of_range);

    // неполное потребление ("5x", "5.5", "0x10") или нет преобразования ("", "x5")
    if (ec != std::errc{} || ptr != last)
        return std::unexpected(number_error::not_number);

    return v;
}

// получение радиуса в виде числа
std::expected<double, number_error> parse_radius(std::string_view r_line) {
    // указат. на начало и конец
    const char* first = r_line.data();
    const char* last = r_line.data() + r_line.size();

    // пропуск ведущих и концевых пробелов
    while (first != last && std::isspace(static_cast<unsigned char>(*first)))
        ++first;
    while (last != first && std::isspace(static_cast<unsigned char>(last[-1])))
        --last;

    if (first == last)
        return std::unexpected(number_error::empty);

    // если после плюса идёт минус - выходим, т.к. это плохой симол
    // остальные плохие символы и так выхзовут падение далее
    if (first != last && *first == '+') {
        ++first;
        if (first != last && *first == '-')
            return std::unexpected(number_error::not_number);
    }

    double R = 0;
    auto [ptr, ec] = std::from_chars(first, last, R);

    // число за границами диапазона типа
    if (ec == std::errc::result_out_of_range)
        return std::unexpected(number_error::out_of_range);

    // если парс остановился не на конце или возникла ошибка - там нечисловой символ
    if (ec != std::errc() || ptr != last)
        return std::unexpected(number_error::not_number);

    // deepseek посоветовал сделать провеку на конечность числа
    // т.к. fromchars читает nan и бесконечность без проблем, как число
    if (!std::isfinite(R))
        return std::unexpected(number_error::not_finite);
    // проверка положительности радиуса
    if (R <= 0)
        return std::unexpected(number_error::not_positive);

    return R;
}

// форма слова "вершина" со склонением.
// функция отдаёт индекс, по которому в массиве лежит нужная форма слова
int vertex_form_index(int N) {
    if (N % 10 == 1 && N % 100 / 10 != 1)
        return 0;
    else if (N % 10 >= 2 && N % 10 <= 4 && N % 100 / 10 != 1)
        return 1;
    else
        return 2;
}
