#pragma once

#include <string>

// условна€ компил€ци€ дл€ разных моментов: dllexport - дл€ компил. dll, dllimport - дл€ компил. main
#ifdef COMMON_EXPORTS
#define COMMON_API __declspec(dllexport)
#else
#define COMMON_API __declspec(dllimport)
#endif

// убрать пробелы слева и справа
COMMON_API void trim_str(std::string& s);

// пуста€ ли строка
COMMON_API int is_empty(const std::string& s);
