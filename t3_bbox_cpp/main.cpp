#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <print>
#include <ranges>
#include <string>
#include <vector>

#include "..\common\exit_codes.hpp"
#include "..\common\geometry.hpp"
#include "..\common\string_utils.hpp"

int main() {
    // setlocale(LC_ALL, "Russian_Russia.1251");
    // считывание начальных x y z в массив; сначала строка, потом число
    // при возникновении ошибки вывод с номером битой строки
    std::string temp;
    std::vector<Point> points;
    int i = 0;
    // пока не EOF - читаем
    while (std::getline(std::cin, temp)) {
        // увеличение сразу, чтобы был именно номер строки, а не элем. в массиве
        ++i;

        if (std::cin.bad()) {
            std::cerr << "Сбой IO при чтении данных в строке " << i << "\n";
            return exit_code::io_fail;
        }

        // абсолютно пустые строки просто пропускаются
        if (is_empty(temp))
            continue;

        // чтение точки через parse_point
        auto result = parse_point(temp);
        if (!result) {
            // мало аргументов
            if (result.error() == 1)
                std::cerr << "Строка " << i << ". Ожидались координаты X Y Z. Получено: " << temp << "\n";
            // нечисловые данные
            else
                std::cerr << "Строка " << i << ". Нечисловые данные: " << temp << "\n";
            return exit_code::data;
        }

        // если дошли до этого момента, значит ex_code == 0 и можно сложить точку в массив
        points.push_back(*result);
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
    Point center{std::reduce(xs.begin(), xs.end()) / points.size(), std::reduce(ys.begin(), ys.end()) / points.size(),
                 std::reduce(zs.begin(), zs.end()) / points.size()};

    // вычисление среднего расстояния от точек до центроида
    double dist = 0;
    for (const auto& p : points) {
        dist += std::hypot(p.x - center.x, p.y - center.y, p.z - center.z);
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