#include <charconv>
#include <cmath>
#include <iostream>
#include <numbers>
#include <print>
#include <string>
#include <vector>

#include "../common/exit_codes.hpp"

int main() {
    std::string lineN;

    if (!std::getline(std::cin, lineN) || lineN.empty()) {
        std::cerr << "Пустой ввод вместо целого числа\n";
        return exit_code::no_in;
    }

    while (!lineN.empty() && (lineN.back() == '\r' || lineN.back() == ' '))
        lineN.pop_back();

    if (lineN.empty()) {
        std::cerr << "Пустой ввод вместо целого числа\n";
        return exit_code::no_in;
    }

    int N;
    const std::from_chars_result res = std::from_chars(lineN.data(), lineN.data() + lineN.size(), N);
    if (res.ec != std::errc{} || res.ptr != lineN.data() + lineN.size()) {
        std::cerr << "N должно быть целым числом. Получено: " << lineN << '\n';
        return exit_code::data;
    }

    if (N < 3 || N > 20) {
        std::cerr << "N должно быть в диапазоне [3; 20]. Получено: " << lineN << '\n';
        return exit_code::usage;
    }

    std::vector<double> x(N);
    std::vector<double> y(N);
    double perm_angle;
    for (int i = 0; i < N; ++i) {
        perm_angle = 2 * std::numbers::pi * i / N;
        x[i] = std::cos(perm_angle);
        y[i] = std::sin(perm_angle);
    }

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            if (i == j)
                std::print("{:8.3f}", 0.0);
            else
                std::print("{:8.3f}", std::hypot(x[i] - x[j], y[i] - y[j]));
        }
        std::print("\n");
    }
    return 0;
}
