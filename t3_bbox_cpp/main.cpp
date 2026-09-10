#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <print>
#include <ranges>
#include <string>
#include <vector>

#include "../common/exit_codes.hpp"
#include "../common/geometry.hpp"
#include "../common/string_utils.hpp"

int main() {
    // откл. синхронизации, иначе EOF и io-fail станут неразличимы из-за чтени€ через fgetc
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

        // абсолютно пустые строки просто пропускаютс€
        if (is_empty(temp))
            continue;

        // чтение точки через parse_point
        auto result = parse_point(temp);
        if (!result) {
            // мало аргументов
            if (result.error() == parse_too_few)
                std::cerr << "—трока " << i << " - недостаточно координат. ќжидалось X Y Z, получено: " << temp << "\n";
            // нечисловые данные
            else if (result.error() == parse_not_number)
                std::cerr << "—трока " << i << ". Ќечисловые данные: " << temp << "\n";
            // слишком много координат
            else if (result.error() == parse_too_much)
                std::cerr << "—трока " << i << " - слишком много координат. ќжидалось X Y Z, получено: " << temp
                          << "\n";
            // одна из координат - не конечное число
            else if (result.error() == parse_not_finite)
                std::cerr << "—трока " << i << ". —реди X Y Z обнаружена не конечна€ координата: " << temp << "\n";

            return exit_code::data;
        }

        // если дошли до этого момента, значит ex_code == 0 и можно сложить точку в массив
        points.push_back(*result);
    }
    // проверка сбо€ после цикла, т.к. при сбое - выход из цикла. i+1 т.к. в цикл при ошибке не зашли
    if (std::cin.bad()) {
        std::cerr << "—трока " << i + 1 << " - сбой ввода-вывода\n";
        return exit_code::io_fail;
    }

    // если по итогу чтени€ массив точек пустой, то входных данных не было
    if (points.empty()) {
        std::cerr << "¬ходные данные отсутствуют\n";
        return exit_code::no_in;
    }

    // векторы <double> отдельно со всеми x, y, z дл€ дальнейших вычислений
    auto xs = points | std::views::transform([](const Point& p) { return p.x; });
    auto ys = points | std::views::transform([](const Point& p) { return p.y; });
    auto zs = points | std::views::transform([](const Point& p) { return p.z; });

    // поиск минимумов и максимумов по ос€м
    Point max_border{std::ranges::max(xs), std::ranges::max(ys), std::ranges::max(zs)};
    Point min_border{std::ranges::min(xs), std::ranges::min(ys), std::ranges::min(zs)};

    // центр - сумма по всем ос€м делЄнна€ на кол-во точек
    Point center{std::reduce(xs.begin(), xs.end()) / points.size(), std::reduce(ys.begin(), ys.end()) / points.size(),
                 std::reduce(zs.begin(), zs.end()) / points.size()};

    // вычисление среднего рассто€ни€ от точек до центроида
    double dist = 0;
    for (const auto& p : points) {
        dist += std::hypot(p.x - center.x, p.y - center.y, p.z - center.z);
    }
    dist = dist / points.size();

    // вывод кол-ва точек, границ паралеллепипеда, коорд. центроида и сред. расст. до него
    std::print(" оличество точек: {}\n", points.size());

    std::print(
        " оординаты огранич. паралеллепипеда x: [ {:.3f}; {:.3f} ] y: [ {:.3f}; {:.3f} ] z: [ {:.3f}; {:.3f} ]\n",
        min_border.x, max_border.x, min_border.y, max_border.y, min_border.z, max_border.z);

    std::print(" оординаты центроида x: {:.3f}, y: {:.3f}, z: {:.3f}\n", center.x, center.y, center.z);

    std::print("—реднее рассто€ние от точек до центроида: {:.3f}\n", dist);

    return 0;
}