#include "geometry.hpp"

#include <array>
#include <cctype>
#include <charconv>
#include <expected>
#include <string>
#include <string_view>
#include <system_error>

// распознавание x y z
std::expected<Point, int> parse_point(std::string_view s) {
    // чтение double чисел из строки
    std::array<double, 3> dots{};
    const char* ptr_start = s.data();
    const char* ptr_end = s.data() + s.size();
    int j = 0;
    while (j < 3) {
        // пропуск пробелов, т.к. между числами может быть и не по одному пробелу
        while (ptr_start != ptr_end && std::isspace(static_cast<unsigned char>(*ptr_start)))
            ++ptr_start;

        // если указатель начала указывает на конец - а при чтении 3 точек этого не должно произойти
        // т.к. их 3 штуки, то значит нет полного X Y Z
        if (ptr_start == ptr_end)
            return std::unexpected(1);

        const auto [ptr, ec] = std::from_chars(ptr_start, ptr_end, dots[j]);
        // указатель на конце, хотя 3 шт не было прочитано
        if (ptr == ptr_end && j < 2)
            return std::unexpected(1);

        // если ec с ошибкой или распознавание слетело не на пробеле и не на /0 в конце, то это ошибка в данных
        if (ec != std::errc{} || (!std::isspace(static_cast<unsigned char>(*ptr)) && *ptr != '\0')) {
            return std::unexpected(2);
        }
        // начальный указатель для след.итерац. становится туда, где остановилось распознавание в этой итерации
        ptr_start = ptr;

        ++j;
    }

    return Point{dots[0], dots[1], dots[2]};
}