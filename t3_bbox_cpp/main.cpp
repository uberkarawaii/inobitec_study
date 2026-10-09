#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <print>
#include <ranges>
#include <string>
#include <vector>

#include "../common/cli.hpp"
#include "../common/exit_codes.hpp"
#include "../common/geometry.hpp"
#include "../common/parse_codes.hpp"
#include "../common/string_utils.hpp"

constexpr std::string_view kHelpText = R"(t3_bbox: bounding box и центроид облака точек
Использование: [--help] [--version]
Вход (stdin): точки "x y z" (по одной в строке) до EOF.
Выводит количество точек, min/max по осям, центроид и среднее расстояние до
центроида (3 знака).
Коды возврата: 0 — успех; 65 — некорректные данные; 66 — нет входных данных; 74 — сбой ввода-вывода.)";

int main(int argc, char* argv[]) {
    // если первый аргумент --version - отдаём версию и завершаем
    if (handle_version_flag(argc, argv))
        return 0;
    // если первый арг. --help/-h - выводим справку и завершаем
    if (handle_help_flag(argc, argv, kHelpText))
        return 0;

    // откл. синхронизации, иначе EOF и io-fail станут неразличимы из-за чтения через fgetc
    std::ios::sync_with_stdio(false);
    // считывание начальных x y z в массив; сначала строка, потом число
    // при возникновении ошибки вывод с номером битой строки
    std::string temp;
    std::vector<Point> points;
    int i = 0;
    // пока не EOF - читаем
    while (std::getline(std::cin, temp)) {
        // увеличение сразу, чтобы был именно номер строки, а не элем. в массиве
        ++i;

        // абсолютно пустые строки просто пропускаются
        if (is_empty(temp))
            continue;

        // чтение точки через parse_point
        auto result = parse_point(temp);
        if (!result) {
            // мало аргументов
            if (result.error() == number_error::too_few)
                std::cerr << "Строка " << i << " - недостаточно координат. Ожидалось X Y Z, получено: " << temp << "\n";
            // нечисловые данные
            else if (result.error() == number_error::not_number)
                std::cerr << "Строка " << i << ". Нечисловые данные: " << temp << "\n";
            // слишком много координат
            else if (result.error() == number_error::too_much)
                std::cerr << "Строка " << i << " - слишком много координат. Ожидалось X Y Z, получено: " << temp
                          << "\n";
            // одна из координат - не конечное число
            else if (result.error() == number_error::not_finite)
                std::cerr << "Строка " << i << ". Среди X Y Z обнаружена не конечная координата: " << temp << "\n";
            // в точке есть число выходящее за диапазон допустимого
            else if (result.error() == number_error::out_of_range)
                std::cerr << "Строка " << i
                          << ". Среди X Y Z обнаружена координата, выходящая за допустимый диапазон: " << temp << "\n";

            return exit_code::data;
        }

        // если дошли до этого момента, значит ex_code == 0 и можно сложить точку в массив
        points.push_back(*result);
    }
    // проверка сбоя после цикла, т.к. при сбое - выход из цикла. i+1 т.к. в цикл при ошибке не зашли
    if (std::cin.bad()) {
        std::cerr << "Строка " << i + 1 << " - сбой ввода-вывода\n";
        return exit_code::io_fail;
    }

    // если по итогу чтения массив точек пустой, то входных данных не было
    if (points.empty()) {
        std::cerr << "Входные данные отсутствуют\n";
        return exit_code::no_in;
    }

    // векторы <double> отдельно со всеми x, y, z для дальнейших вычислений
    auto xs = points | std::views::transform([](const Point& p) { return p.x; });
    auto ys = points | std::views::transform([](const Point& p) { return p.y; });
    auto zs = points | std::views::transform([](const Point& p) { return p.z; });

    // поиск минимумов и максимумов по осям
    Point max_border{std::ranges::max(xs), std::ranges::max(ys), std::ranges::max(zs)};
    Point min_border{std::ranges::min(xs), std::ranges::min(ys), std::ranges::min(zs)};

    // центр - сумма по всем осям делённая на кол-во точек
    Point center = centroid(points);

    // вычисление среднего расстояния от точек до центроида
    double dist = 0;
    for (const auto& p : points) {
        dist += point_distance(p, center);
    }
    dist = dist / points.size();

    // вывод кол-ва точек, границ паралеллепипеда, коорд. центроида и сред. расст. до него
    std::print("Количество точек: {}\n", points.size());

    std::print(
        "Координаты огранич. паралеллепипеда x: [ {:.3f}; {:.3f} ] y: [ {:.3f}; {:.3f} ] z: [ {:.3f}; {:.3f} ]\n",
        min_border.x, max_border.x, min_border.y, max_border.y, min_border.z, max_border.z);

    std::print("Координаты центроида x: {:.3f}, y: {:.3f}, z: {:.3f}\n", center.x, center.y, center.z);

    std::print("Среднее расстояние от точек до центроида: {:.3f}\n", dist);

    return 0;
}