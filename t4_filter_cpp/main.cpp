#include <cmath>
#include <expected>
#include <iostream>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include "..\common\exit_codes.hpp"
#include "..\common\geometry.hpp"
#include "..\common\string_utils.hpp"

// получение радиуса в виде числа
// 1 - несчисловой символ
// 2 - не конечное число
// 3 - не положительный радиус
std::expected<double, int> get_radius(std::string_view r_line) {
    double R;
    // если парс остановился не на конце или возникла ошибка - там нечисловой символ
    auto [ptr, ec] = std::from_chars(r_line.data(), r_line.data() + r_line.size(), R);
    if (ec != std::errc() || ptr != r_line.data() + r_line.size())
        return std::unexpected(1);
    // deepseek посоветовал сделать провеку на конечность числа
    // т.к. fromchars читает nan и бесконечность без проблем, как число
    if (!std::isfinite(R))
        return std::unexpected(2);
    // проверка положительности радиуса
    if (R <= 0)
        return std::unexpected(3);

    return R;
}

// отдаёт вектор с точками
// 1 - мало аргументов
// 2 - нечисловое значение
// 3 - сбой в потоке IO
// 4 - массив точек пустой
std::expected<std::vector<Point>, int> get_points() {
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
            return std::unexpected(exit_code::io_fail);
        }

        // пропуск пустых строк
        if (is_empty(temp))
            continue;

        // распознавание чисел и проверка на то, что они числовые, и что их достаточное кол-во
        // если мало аргументов вернётся 1, если нечисловой символ - 2
        auto result = parse_point(temp);
        if (!result) {
            if (result.error() == 1)
                std::cerr << "Строка " << i << ". Ожидалось X Y Z, получено: " << temp << "\n";
            else
                std::cerr << "Строка " << i << ". Нечисловое значение: " << temp << "\n";
            return std::unexpected(exit_code::data);
        }

        // если дошли сюда, то ex_code == 0 можно положить точку в массив
        points.push_back(*result);
    }

    if (points.empty()) {
        std::cerr << "Точки отсутствуют\n";
        return std::unexpected(exit_code::no_in);
    }

    return points;
}

int main(int argc, char* argv[]) {
    // проверки ввода радиуса; при ошибке аргументов либо 0, либо больше чем 1
    if (argc != 2) {
        if (argc < 2)
            std::cerr << "Ожидался радиус; его значение не было введено\n";
        else
            std::cerr << "Ожидался радиус; были введены лишние аргументы\n";
        return exit_code::usage;
    }

    std::string_view r_line{argv[1]};

    // распознавание числа и проверка, что это число
    auto parsedR = get_radius(r_line);
    if (!parsedR) {
        if (parsedR.error() == 1)
            std::cerr << "Радиус должен быть числом. Получено: " << r_line << "\n";
        else if (parsedR.error() == 2)
            std::cerr << "Радиус должен быть конечным числом. Получено: " << r_line << "\n";
        else
            std::cerr << "Радиус должен быть положительным. Получено: " << r_line << "\n";
        return exit_code::usage;
    }
    double R = *parsedR;

    // результат обработки входных точек
    auto result = get_points();

    // Обработка ошибок - get_points уже напечатал лог и отдаст верный код ошибки
    if (!result)
        return result.error();

    // если без ошибок, можно дальше обращаться к *result
    // что оставляем
    auto is_point_suitable = [R](const Point& p) { return std::hypot(p.x, p.y, p.z) < R; };
    // что делаем
    auto show_format = [](const Point& p) { return std::format("{:.3f} {:.3f} {:.3f}\n", p.x, p.y, p.z); };
    // перед выводом в result складывается всё что проходит по фильтру, в виде форматированной строки
    auto suitable_points = *result | std::views::filter(is_point_suitable) | std::views::transform(show_format);
    for (const auto& pt : suitable_points)
        std::print("{}", pt);

    return 0;
}