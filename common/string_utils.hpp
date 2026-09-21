#pragma once

#include <string>

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

#include <cstdint>
#include <expected>
#include <string_view>

#include "parse_codes.hpp"

// убрать пробелы слева и справа
COMMON_API void trim_str(std::string& s);

// пуста€ ли строка
COMMON_API int is_empty(const std::string& s);

// разбор целого (дес€тичное); строка уже обрезана от пробелов
COMMON_API std::expected<std::int32_t, number_error> parse_int32(std::string_view s);
