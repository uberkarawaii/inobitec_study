#include <array>
#include <charconv>
#include <cmath>
#include <iostream>
#include <print>
#include <string>
#include <system_error>
#include <vector>

#include "..\common\exit_codes.hpp"
#include "..\common\string_utils.hpp"

struct Point {
    double x;
    double y;
    double z;
};

int main(int argc, char* argv[]) {
    // проверки: что был введЄн один аргумент - радиус
    if (argc < 2) {
        std::cerr << "ќжидалс€ радиус; его значение не было введено\n";
        return exit_code::usage;
    }

    if (argc > 2) {
        std::cerr << "ќжидалс€ радиус; были введены лишние аргументы\n";
        return exit_code::usage;
    }

    std::string_view r_line{argv[1]};

    // распознавание числа и проверка, что это число
    double R;
    auto [ptr, ec] = std::from_chars(r_line.data(), r_line.data() + r_line.size(), R);
    if (ec != std::errc() || ptr != r_line.data() + r_line.size()) {
        std::cerr << "–адиус должен быть числом. ѕолучено: " << r_line << "\n";
        return exit_code::usage;
    }

    // deepseek посоветовал сделать провеку на конечность числа
    // т.к. fromchars читает nan и бесконечность без проблем, как число
    if (!std::isfinite(R)) {
        std::cerr << "–адиус должен быть конечным числом. ѕолучено: " << r_line << "\n";
        return exit_code::usage;
    }

    // проверка на положительность R
    if (R <= 0) {
        std::cerr << "–адиус должен быть положительным. ѕолучено: " << R << "\n";
        return exit_code::usage;
    }

    // массив точек
    std::vector<Point> points;
    // строка дл€ считывани€ и промежуточный массив
    std::string temp;
    std::array<double, 3> point{};
    // счЄтчик строк
    int i = 0;

    while (std::getline(std::cin, temp)) {
        ++i;
        // error IO проверка
        if (std::cin.bad()) {
            std::cerr << "ќшибка IO при чтении строки " << i << "\n";
            return exit_code::io_fail;
        }
        // обрезка боковых пробелов
        trim_str(temp);
        // пропуск пустых строк
        if (temp.empty())
            continue;

        // распознавание чисел и проверка на то, что они числовые, и что их достаточное кол-во
        const char* start_ptr = temp.data();
        const char* end_ptr = start_ptr + temp.size();
        int j = 0;

        while (j < 3) {

            const auto [ptr1, ec1] = std::from_chars(start_ptr, end_ptr, point[j]);
            // ошибка или остановка не на прблеле и не на конце строки в строке точно есть нечисловой символ
            if (ec1 != std::errc() || (*ptr1 != ' ' && *ptr1 != '\0')) {
                std::cerr << "—трока " << i << ". Ќечисловое значение: " << temp << "\n";
                return exit_code::data;
            }
            // если обрабатываетс€ не 3-й аргумент, а указатель при распозн. остановилс€ на последнем символе
            // это означает что аргументов не 3 шт.
            if (j != 2 && ptr1 == end_ptr) {
                std::cerr << "—трока " << i << ". ќжидалось X Y Z, получено: " << temp << "\n";
                return exit_code::data;
            }

            // сдвиг переднего указател€ пока между числами пробелы
            // предварительно он переставл€етс€ туда, где закончилось распознавание предыдущего числа
            start_ptr = ptr1;
            while (start_ptr != end_ptr && *start_ptr == ' ')
                ++start_ptr;
            ++j;
        }

        // если всЄ было распознано без ошибок, можно положить эти числа в массив точек
        points.push_back({point[0], point[1], point[2]});
    }

    // если после всех считываний массив пуст, значит в него ничего и не было заисано
    if (points.empty()) {
        std::cerr << "“очки отсутствуют\n";
        return exit_code::no_in;
    }

    // проход по всем точкам и вывод только тех, чьЄ рассто€ние от начала коорд. меньше R
    for (const auto& p : points) {
        if (std::hypot(p.x, p.y, p.z) < R)
            std::print("{:.3f} {:.3f} {:.3f}\n", p.x, p.y, p.z);
    }

    return 0;
}