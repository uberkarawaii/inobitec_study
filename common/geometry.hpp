#pragma once

#include <expected>
#include <string_view>

// geom.hpp включается и в geom.cpp, и в main.cpp
// из geom.cpp будет сделана dll, так что при сборке geom.cpp его ф-ции будут помечены для экспорта
// внесения в таблицу адресов (.lib)
// при сборке main же надо указать, что эта ф-ция будет взята из .dll, будет импортирована - линкер
// поставит ссылку на неё в .dll
// т.е. для управления поведением заголовочника - условная компиляция
// и чтобы не было проблем при статической линковке - пустой COMMON_API для соотв. константы

// в условную компиляцию был добавлен случай linux - там этих флагов вообще не будет
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

struct Point {
    double x;
    double y;
    double z;
};

// распознавание x y z
// возвращает либо Point, либо код ошибки (произойдёт первая ошибка при движении справа налево)
COMMON_API std::expected<Point, constexpr int> parse_point(std::string_view s);

// 1 - мало координат
// 2 - нечисловые данные
// 3 - много координат
// 4 - координата равна inf или nan
inline constexpr int parse_too_few = 1;
inline constexpr int parse_not_number = 2;
inline constexpr int parse_too_much = 3;
inline constexpr int parse_not_finite = 4;
