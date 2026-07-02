#include <charconv>
#include <cmath>
#include <iostream>
#include <limits>
#include <print>
#include <string>
#include <system_error>
#include <vector>

#include "..\common\exit_codes.hpp"
#include "..\common\string_utils.hpp"

// структура для точек
struct Point {
    double x;
    double y;
    double z;
};

int main(void) {
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
        trim_str(temp);
        if (temp.empty())
            continue;

        // чтение double чисел из строки
        std::vector<double> dots(3);
        const char* ptr_start = temp.data();
        const char* ptr_end = temp.data() + temp.size();

        for (int j = 0; j < 3; ++j) {
            const auto [ptr, ec] = std::from_chars(ptr_start, ptr_end, dots[j]);
            // если указатель на конец, а 3 числа не было прочитано, то это неверный ввод какой-то точки, нет полного X
            // Y Z
            if (ptr == ptr_end && j != 2) {
                std::cerr << "Строка " << i << ". Ожидались координаты X Y Z. Получено:" << temp << "\n";
                return exit_code::data;
            }
            // если ec с ошибкой или распознавание слетело не на пробеле и не на /0 в конце, то это ошибка в данных
            if (ec != std::errc{} || (*ptr != ' ' && *ptr != '\0')) {
                std::cerr << "Строка " << i << ". Нечисловые данные: " << temp << "\n";
                return exit_code::data;
            }
            // сдвиг начального указателя на следующий не пробельный символ
            ptr_start = ptr;
            while (ptr_start != ptr_end && *ptr_start == ' ')
                ++ptr_start;
        }

        // если всё распознано нормально, можно сложить в общий массив
        points.push_back({dots[0], dots[1], dots[2]});
    }

    // если по итогу чтения массив точек пустой, то входных данных не было
    if (points.empty()) {
        std::cerr << "Входные данные отсутствуют\n";
        return exit_code::no_in;
    }

    // поиск минимальных и максимальных x y z. и суммы точек для среднего арифм. - центроида
    // точки min и max, проинициализированные на + и - границы double.
    double minimum = std::numeric_limits<double>::lowest();
    double maximum = std::numeric_limits<double>::max();
    Point max_border{minimum, minimum, minimum};
    Point min_border{maximum, maximum, maximum};
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