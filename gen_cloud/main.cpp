#include <charconv>
#include <cstring>
#include <iostream>
#include <print>
#include <random>
#include <system_error>

#include "..\common\exit_codes.hpp"

int main(int argc, char* argv[]) {

    // если аргументов не 5шт, что-то не так со входными данными
    if (argc != 5) {
        std::cerr << "‘ормат входных данных не соответствует ожидаемому (gen_cloud --size N --seed S)\n";
        return exit_code::usage;
    }

    // 2 аргумент argv - число точек, 4 аргумент - €дро
    // число точек:
    int N;
    const char* start_2 = argv[2];
    const char* end_2 = start_2 + std::strlen(start_2);
    auto [ptr2, ec2] = std::from_chars(start_2, end_2, N);
    if (ec2 != std::errc{} || ptr2 != end_2) {
        std::cerr << " ол-во точек должно быть целым числом. ѕолучено: " << argv[2] << "\n";
        return exit_code::data;
    }

    // €дро
    int s;
    const char* start_4 = argv[4];
    const char* end_4 = start_4 + std::strlen(start_4);
    auto [ptr4, ec4] = std::from_chars(start_4, end_4, s);
    if (ec4 != std::errc{} || ptr4 != end_4) {
        std::cerr << "ядро должно быть целым числом. ѕолучено: " << argv[4] << "\n";
        return exit_code::data;
    }
    // проверка на неотрицательное число, т.к. генератор будет принимать unsigned int
    if (s < 0) {
        std::cerr << "ядро должно быть положительным числом. ѕолучено: " << argv[4] << "\n";
        return exit_code::data;
    }
    unsigned int S = s;

    // генератор случаных величин с €дром S и обЄртка дл€ вещественных чисел над ним
    std::mt19937 rand{S};
    std::uniform_real_distribution<double> range(-100.0, 100.0);
    // точки, N штук
    for (int i = 0; i < N; ++i) {
        std::print("{} {} {}\n", range(rand), range(rand), range(rand));
    }

    return 0;
}