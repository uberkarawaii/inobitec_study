#include "cli.h"

#include "version.h"
// version.h Будет найден благодаря include_directories от CMake - добавит каталог generated в пути поиска

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// при --version первым аргументом вернёт версию программы
bool handle_version_flag(int argc, char* argv[]) {
    if (argc == 2 && strcmp(argv[1], "--version") == 0) {
        printf("%s\n", INOBITEC_STUD_VERSION);
        return 1;
    } else {
        return 0;
    }
}

// при вызове программы с --help / -h будет выводиться справка help_text
// принимает help_text без \n в конце и печатает уже с концевым переносом
bool handle_help_flag(int argc, char* argv[], const char* help_text) {
    if (argc == 2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        printf("%s\n", help_text);
        return 1;
    } else {
        return 0;
    }
}

void print_help_hint(void) {
    fprintf(stderr, "Для справки вызовите программу с параметром --help или -h\n");
    return;
}
