#include <cmath>
#include <iostream>
#include <print>
#include <string>
#include <vector>

#include "..\common\exit_codes.hpp"
#include "..\common\geometry.hpp"
#include "..\common\string_utils.hpp"

int main(int argc, char* argv[]) {
    // проверки: что был введён один аргумент - радиус
    if (argc < 2) {
        std::cerr << "Ожидался радиус; его значение не было введено\n";
        return exit_code::usage;
    }

    if (argc > 2) {
        std::cerr << "Ожидался радиус; были введены лишние аргументы\n";
        return exit_code::usage;
    }

    std::string_view r_line{argv[1]};

    // распознавание числа и проверка, что это число
    double R;
    auto [ptr, ec] = std::from_chars(r_line.data(), r_line.data() + r_line.size(), R);
    if (ec != std::errc() || ptr != r_line.data() + r_line.size()) {
        std::cerr << "Радиус должен быть числом. Получено: " << r_line << "\n";
        return exit_code::usage;
    }

    // deepseek посоветовал сделать провеку на конечность числа
    // т.к. fromchars читает nan и бесконечность без проблем, как число
    if (!std::isfinite(R)) {
        std::cerr << "Радиус должен быть конечным числом. Получено: " << r_line << "\n";
        return exit_code::usage;
    }

    // проверка на положительность R
    if (R <= 0) {
        std::cerr << "Радиус должен быть положительным. Получено: " << R << "\n";
        return exit_code::usage;
    }

    // массив точек
    std::vector<Point> points;
    // строка для считывания
    std::string temp;
    // счётчик строк
    int i = 0;

    while (std::getline(std::cin, temp)) {
        ++i;
        // error IO проверка
        if (std::cin.bad()) {
            std::cerr << "Ошибка IO при чтении строки " << i << "\n";
            return exit_code::io_fail;
        }

        // пропуск пустых строк
        if (is_empty(temp))
            continue;

        // распознавание чисел и проверка на то, что они числовые, и что их достаточное кол-во
        Point p{};
        int ex_code = parse_point(temp, p);

        // мало аргументов
        if (ex_code == 1) {
            std::cerr << "Строка " << i << ". Ожидалось X Y Z, получено: " << temp << "\n";
            return exit_code::data;
        }

        // нечисловое значение
        if (ex_code == 2) {
            std::cerr << "Строка " << i << ". Нечисловое значение: " << temp << "\n";
            return exit_code::data;
        }

        // если дошли сюда, то ex_code == 0 И можно сложить точку в массив
        points.push_back(p);
    }

    // если после всех считываний массив пуст, значит в него ничего и не было заисано
    if (points.empty()) {
        std::cerr << "Точки отсутствуют\n";
        return exit_code::no_in;
    }

    // проход по всем точкам и вывод только тех, чьё расстояние от начала коорд. меньше R
    for (const auto& p : points) {
        if (std::hypot(p.x, p.y, p.z) < R)
            std::print("{:.3f} {:.3f} {:.3f}\n", p.x, p.y, p.z);
    }

    return 0;
}