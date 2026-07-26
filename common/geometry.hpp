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
#ifdef COMMON_STATIC
#define COMMON_API

#elif defined(COMMON_EXPORTS)
#define COMMON_API __declspec(dllexport)

#else
#define COMMON_API __declspec(dllimport)
#endif

struct Point {
    double x;
    double y;
    double z;
};

// распознавание x y z
// возвращает либо Point, либо код ошибки
COMMON_API std::expected<Point, int> parse_point(std::string_view s);
