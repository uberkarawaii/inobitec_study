#include <array>
#include <iostream>
#include <string>

#include "../common/cli.hpp"
#include "../common/exit_codes.hpp"
#include "../common/parse_codes.hpp"
#include "../common/string_utils.hpp"

constexpr std::string_view kHelpText = R"(t2_passport: паспорт фигуры
Использование: [--help] [--version]
Вход (stdin): название фигуры (строка, может быть из нескольких слов), затем
целое число вершин V > 0.
Выводит: Фигура «<название>»: <V> <вершина|вершины|вершин>.
Коды возврата: 0 — успех; 65 — некорректные данные; 66 — нет входных данных.)";

int main(int argc, char* argv[]) {
    // если первый аргумент --version - отдаём версию и завершаем
    if (handle_version_flag(argc, argv))
        return 0;
    // если первый арг. --help/-h - выводим справку и завершаем
    if (handle_help_flag(argc, argv, kHelpText))
        return 0;

    // проверка что имя фигуры - не пустое
    std::string name;
    if (!std::getline(std::cin, name)) {
        std::cerr << "EOF вместо имени фигуры\n";
        return exit_code::no_in;
    }

    trim_str(name);
    if (name.empty()) {
        std::cerr << "Пустой ввод вместо имени фигуры\n";
        return exit_code::no_in;
    }
    // так же и для к-ва вершин. далее, распознавание этого числа
    std::string vertexes;
    if (!std::getline(std::cin, vertexes)) {
        std::cerr << "EOF вместо числа вершин\n";
        return exit_code::no_in;
    }

    auto parsed = parse_int32(vertexes);
    if (!parsed) {
        if (parsed.error() == number_error::out_of_range) {
            std::cerr << "Кол-во вершин не помещается в 32-битное целое. Получено: " << vertexes << "\n";
            return exit_code::data;
        } else if (parsed.error() == number_error::empty) {
            std::cerr << "Пустой ввод вместо кол-ва вершин\n";
            return exit_code::no_in;
        }
        // при любой др. ошибке - эта ветка (ожидаемо - плохой символ. но для страховки, чтобы исп. не ушло дальше)
        std::cerr << "Кол-во вершин должно быть целым числом. Получено: " << vertexes << "\n";
        return exit_code::data;
    }
    // к-во вершин
    const int V = *parsed;
    if (V < 1) {
        std::cerr << "Кол-во вершин должно быть положительным. Получено: " << V << "\n";
        return exit_code::data;
    }

    // массив словоформ . через vertex_form_index(V) будет индекс для верной формы из этого массива
    std::array words{"вершина", "вершины", "вершин"};

    std::cout << "Фигура «" << name << "»: " << V << " " << words[vertex_form_index(V)] << ".\n";

    return 0;
}
