#pragma once

#include <string_view>

// условна€ компил€ци€ дл€ разных моментов:
// dllexport - дл€ компил. dll, dllimport - дл€ компил. main
// пусто - дл€ статической компил€ции
// в условную компил€цию добавлен случай linux - там этих флагов вообще не будет
#ifdef _WIN32

#ifdef COMMON_STATIC
#define COMMON_API

#elif defined(COMMON_EXPORTS)
#define COMMON_API __declspec(dllexport)

#else
#define COMMON_API __declspec(dllimport)
#endif

#else
#define COMMON_API
#endif

// при --version первым аргументом вернЄт версию программы
COMMON_API bool handle_version_flag(int argc, char* argv[]);

// при вызове программы с --help / -h будет выводитьс€ справка help_text
// принимает help_text без \n в конце и печатает уже с концевым переносом
COMMON_API bool handle_help_flag(int argc, char* argv[], std::string_view help_text);

// при падении с кодом 64 usage - печать хинта на help
COMMON_API void print_help_hint();
