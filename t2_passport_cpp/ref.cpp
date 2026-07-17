#include <charconv>
#include <iostream>
#include <string>
#include <array>
#include <string_view>

#include "..\common\string_utils.hpp"

// функция от ии чтобы не было проверки на одном и том же коде
std::string_view get_vertex_name(int N) {
    int r10 = N % 10;
    int r100 = N % 100;
    if (r100 >= 11 && r100 <= 14)
        return 2;
    switch (r10) {
    case 1:
        return 0;
    case 2:
    case 3:
    case 4:
        return 1;
    default:
        return 2;
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
    
    std::array words {"вершина", "вершины", "вершин"};

    std::cout << "Фигура «" << name << "»: " << N << " " << words[get_vertex_name(N)] << ".\n";

    return 0;
}