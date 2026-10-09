#include <iostream>
#include <print>
#include <string>
#include <vector>

#include "../common/cli.hpp"
#include "../common/exit_codes.hpp"
#include "../common/geometry.hpp"
#include "../common/parse_codes.hpp"
#include "../common/string_utils.hpp"

constexpr std::string_view kHelpText = R"(t1_dist_matrix: матрица попарных расстояний вершин N-угольника
Использование: [--help] [--version]
Вход (stdin): целое N, 3 <= N <= 20.
Коды возврата: 0 — успех; 65 — некорректные данные; 66 — нет входных данных.)";

int main(int argc, char* argv[]) {
    // если первый аргумент --version - отдаём версию и завершаем
    if (handle_version_flag(argc, argv))
        return 0;
    // если первый арг. --help/-h - выводим справку и завершаем
    if (handle_help_flag(argc, argv, kHelpText))
        return 0;

    // получение N
    std::string lineN;
    // считывание строки, проверка на пустоту
    if (!std::getline(std::cin, lineN)) {
        std::cerr << "Получен пустой ввод вместо целого N\n";
        return exit_code::no_in;
    }

    // распознавание N и проверка на ошибки распознавания
    auto parsed = parse_int32(lineN);
    if (!parsed) {
        if (parsed.error() == number_error::out_of_range) {
            std::cerr << "N не помещается в 32-битное целое. Получено: " << lineN << '\n';
            return exit_code::data;
        } else if (parsed.error() == number_error::empty) {
            std::cerr << "Получен пустой ввод вместо целого N\n";
            return exit_code::no_in;
        }
        // при любой другой ошибке (ожидаемо при плохом токене, но для страховки, чтобы вдруг не пошло дальше)
        std::cerr << "N должно быть целым числом. Получено: " << lineN << '\n';
        return exit_code::data;
    }

    // полученное число
    const int N = *parsed;

    // проверяем диапазон
    if (N < 3 || N > 20) {
        std::cerr << "N должно быть в диапазоне [3;20]. Получено: " << lineN << '\n';
        return exit_code::data;
    }

    std::vector<Point> points(N);
    for (int i = 0; i < N; ++i) {
        points[i] = polygon_vertex(i, N);
    }

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            if (i == j)
                std::print("{:8.3f}", 0.0);
            else
                std::print("{:8.3f}", point_distance(points[i], points[j]));
        }
        std::print("\n");
    }
    return 0;
}
