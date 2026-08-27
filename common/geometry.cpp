#include "geometry.hpp"

#include <array>
#include <cctype>
#include <charconv>
#include <cmath>
#include <expected>
#include <string>
#include <string_view>
#include <system_error>

// распознавание x y z
std::expected<Point, constexpr int> parse_point(std::string_view s) {
    // чтение double чисел из строки
    std::array<double, 3> dots{};
    // указатель на (начало строки)
    const char* ptr_start = s.data();
    // указатель на (начало строки) + (размер строки)
    const char* ptr_end = s.data() + s.size();
    int j = 0;
    while (j < 3) {
        // пропуск пробелов, т.к. между числами может быть и не по одному пробелу
        while (ptr_start != ptr_end && std::isspace(static_cast<unsigned char>(*ptr_start)))
            ++ptr_start;

        // если указатель начала указывает на конец - а при чтении 3 точек этого не должно произойти
        // т.к. их 3 штуки, то значит нет полного X Y Z
        if (ptr_start == ptr_end)
            return std::unexpected(parse_too_few);

        const auto [ptr, ec] = std::from_chars(ptr_start, ptr_end, dots[j]);
        // указатель на конце, хотя 3 шт не было прочитано
        if (ptr == ptr_end && j < 2)
            return std::unexpected(parse_too_few);

        // если ec с ошибкой или распознавание слетело не на пробеле и не на конце, то это ошибка в данных
        if (ec != std::errc{} || (ptr != ptr_end && !std::isspace(static_cast<unsigned char>(*ptr)))) {
            return std::unexpected(parse_not_number);
        }

        // проверка, что это не nan/inf
        if (!std::isfinite(dots[j]))
            return std::unexpected(parse_not_finite);
        // начальный указатель для след.итерац. становится туда, где остановилось распознавание в этой итерации
        ptr_start = ptr;

        ++j;
    }

    // если дошли сюда и за X Y Z есть что-то ещё
    if (ptr_start != ptr_end) {
        // пропуск пробелов перед этим объектом
        while (ptr_start != ptr_end && std::isspace(static_cast<unsigned char>(*ptr_start)))
            ++ptr_start;

        // если после пропуска начало == конец, то там были одни пробелы. иначе - осталось что-то ещё
        if (ptr_start != ptr_end) {
            double d{};
            const auto [ptr1, ec1] = std::from_chars(ptr_start, ptr_end, d);
            // если ec1 выдаёт ошибку и указатель остановился не на пробельном и не на конечном символе, то дело в
            // лишнем символе
            if (ec1 != std::errc{} || (ptr1 != ptr_end && !std::isspace(static_cast<unsigned char>(*ptr1))))
                return std::unexpected(parse_not_number);
            // иначе - там лишнее число
            else
                return std::unexpected(parse_too_much);
        }
    }

    return Point{dots[0], dots[1], dots[2]};
}