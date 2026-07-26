#pragma once

#include <expected>
#include <string_view>

// geom.hpp включаетс€ и в geom.cpp, и в main.cpp
// из geom.cpp будет сделана dll, так что при сборке geom.cpp его ф-ции будут помечены дл€ экспорта
// внесени€ в таблицу адресов (.lib)
// при сборке main же надо указать, что эта ф-ци€ будет вз€та из .dll, будет импортирована - линкер
// поставит ссылку на неЄ в .dll
// т.е. дл€ управлени€ поведением заголовочника - условна€ компил€ци€
#ifdef COMMON_EXPORTS
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
