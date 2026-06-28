#include <charconv>
#include <iostream>
#include <string>
#include <string_view>
#include <system_error>

#include "..\common\exit_codes.hpp"
#include "..\common\string_utils.hpp"

// форма слова "вершина" со склонением.
// string_view т.к. это уже готовый объект, можно отдаль просто указатель + длину, а не копию
std::string_view get_vertex_name(int N) {
    if (N % 10 == 1 && N % 100 / 10 != 1)
        return "вершина";
    else if (N % 10 >= 2 && N % 10 <= 4 && N % 100 / 10 != 1)
        return "вершины";
    else
        return "вершин";
}

int main() {
    setlocale(LC_ALL, "Russian_Russia.1251");

    std::string name;
    if (!std::getline(std::cin, name)) {
        std::cerr << "EOF вместо имени фигуры\n";
        return exit_code::no_in;
    }

    trim_str(name);
    if (name.empty()) {
        std::cerr << "Пустой ввод вместо имени фигуры\n";
        return exit_code::data;
    }

    std::string vertexes;
    if (!std::getline(std::cin, vertexes)) {
        std::cerr << "EOF вместо кол-ва вершин\n";
        return exit_code::no_in;
    }

    trim_str(vertexes);
    if (vertexes.empty()) {
        std::cerr << "Пустой ввод вместо кол-ва вершин\n";
        return exit_code::data;
    }

    int V;
    // ptr - указат., где остановлен парсинг. ec - error code.
    // в случае успешного чтения ec проинциализирована пустым инициализатором. это и есть std::errc()
    const auto [ptr, ec] = std::from_chars(vertexes.data(), vertexes.data() + vertexes.size(), V);
    if (ec != std::errc{} || ptr != vertexes.data() + vertexes.size()) {
        std::cerr << "Кол-во вершин должно быть целым числом. Получено: " << vertexes << "\n";
        return exit_code::data;
    }

    if (V < 1) {
        std::cerr << "Кол-во вершин должно быть положительным. Получено: " << V << "\n";
        return exit_code::usage;
    }

    std::string_view vertex_name = get_vertex_name(V);

    std::cout << "Фигура «" << name << "»: " << V << " " << vertex_name << ".\n";

    return 0;
}
