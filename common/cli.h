#ifndef IRINA_CLI_H
#define IRINA_CLI_H

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

#include <stdbool.h>

// для с++ потребителя будет внешней си-функцией, с с-abi, без манглинга. для си-потребителя - не влияет
#ifdef __cplusplus
extern "C" {
#endif

// при --version первым аргументом вернёт версию программы
COMMON_API bool handle_version_flag(int argc, char* argv[]);

// при вызове программы с --help / -h будет выводиться справка help_text
// принимает help_text без \n в конце и печатает уже с концевым переносом
COMMON_API bool handle_help_flag(int argc, char* argv[], const char* help_text);

// при падении с кодом 64 usage - печать хинта на help
COMMON_API void print_help_hint(void);

#ifdef __cplusplus
}
#endif

#endif
