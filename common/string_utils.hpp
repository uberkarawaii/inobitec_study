#pragma once

#include <string>

// условная компиляция для разных моментов:
// dllexport - для компил. dll, dllimport - для компил. main
// пусто - для статической компиляции
#ifdef COMMON_STATIC
#define COMMON_API

#elif defined(COMMON_EXPORTS)
#define COMMON_API __declspec(dllexport)

#else
#define COMMON_API __declspec(dllimport)
#endif

// убрать пробелы слева и справа
COMMON_API void trim_str(std::string& s);

// пустая ли строка
COMMON_API int is_empty(const std::string& s);
