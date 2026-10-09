#include "cli.hpp"

#include "version.h"
// version.h Будет найден благодаря include_directories от CMake - добавит каталог generated в пути поиска

#include <iostream>
#include <print>
#include <string_view>

// при --version первым аргументом вернёт версию программы
bool handle_version_flag(int argc, char* argv[]) {
    if (argc == 2 && std::string_view{argv[1]} == "--version") {
        std::println(INOBITEC_STUD_VERSION);
        return 1;
    } else {
        return 0;
    }
}

// при вызове программы с --help / -h будет выводиться справка help_text
// принимает help_text без \n в конце и печатает уже с концевым переносом
bool handle_help_flag(int argc, char* argv[], std::string_view help_text) {
    if (argc == 2 && (std::string_view{argv[1]} == "--help" || std::string_view{argv[1]} == "-h")) {
        std::println("{}", help_text);
        return 1;
    } else {
        return 0;
    }
}

void print_help_hint() {
    std::cerr << "Для справки вызовите программу с параметром --help или -h\n";
    return;
}
