#include <charconv>
#include <iostream>
#include <string>
#include <string_view>

#include "..\common\string_utils.hpp"

// функция от ии чтобы не было проверки на одном и том же коде
std::string_view get_vertex_name(int N) {
    int r10 = N % 10;
    int r100 = N % 100;
    if (r100 >= 11 && r100 <= 14)
        return "вершин";
    switch (r10) {
    case 1:
        return "вершина";
    case 2:
    case 3:
    case 4:
        return "вершины";
    default:
        return "вершин";
    }
}

int main() {
    setlocale(LC_ALL, "Russian_Russia.1251");

    std::string name;
    std::getline(std::cin, name);
    trim_str(name);

    std::string vertexes;
    std::getline(std::cin, vertexes);
    trim_str(vertexes);
    int N;
    std::from_chars(vertexes.data(), vertexes.data() + vertexes.size(), N);

    std::cout << "Фигура «" << name << "»: " << N << " " << get_vertex_name(N) << ".\n";

    return 0;
}