#include <cmath>
#include <iostream>
#include <limits>
#include <print>
#include <string>
#include <vector>

#include "..\common\exit_codes.hpp"
#include "..\common\geometry.hpp"
#include "..\common\string_utils.hpp"

int main() {
    setlocale(LC_ALL, "Russian_Russia.1251");
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

        // чтение double чисел из строки функцией parse_point
        Point p{};
        int ex_code = parse_point(temp, p);

        // мало аргументов
        if (ex_code == 1) {
            std::cerr << "Строка " << i << ". Ожидались координаты X Y Z. Получено: " << temp << "\n";
            return exit_code::data;
        }

        // нечисловые данные в строке
        if (ex_code == 2) {
            std::cerr << "Строка " << i << ". Нечисловые данные: " << temp << "\n";
            return exit_code::data;
        }

        // если дошли до этого момента, значит ex_code == 0 и можно сложить точку в массив
        points.push_back(p);
    }

    // если по итогу чтения массив точек пустой, то входных данных не было
    if (points.empty()) {
        std::cerr << "Входные данные отсутствуют\n";
        return exit_code::no_in;
    }

    // поиск минимальных и максимальных x y z. и суммы точек для среднего арифм. - центроида
    // точки min и max, проинициализированные на + и - границы double.
    Point max_border{std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(),
                     std::numeric_limits<double>::lowest()};

    Point min_border{std::numeric_limits<double>::max(), std::numeric_limits<double>::max(),
                     std::numeric_limits<double>::max()};

    // и точка центроида
    Point center{};

    for (const auto& p : points) {

        if (p.x > max_border.x)
            max_border.x = p.x;
        if (p.y > max_border.y)
            max_border.y = p.y;
        if (p.z > max_border.z)
            max_border.z = p.z;

        if (p.x < min_border.x)
            min_border.x = p.x;
        if (p.y < min_border.y)
            min_border.y = p.y;
        if (p.z < min_border.z)
            min_border.z = p.z;

        center.x += p.x;
        center.y += p.y;
        center.z += p.z;
    }

    // деление суммы координат на кол-во для центроида и вывод
    center.x = center.x / points.size();
    center.y = center.y / points.size();
    center.z = center.z / points.size();

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