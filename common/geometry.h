#ifndef IRINA_GEOMETRY_H
#define IRINA_GEOMETRY_H

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

#include "parse_codes.h"

struct Point {
    double x;
    double y;
    double z;
};

// распознавание точки. возвращает код из enum-ы
// срабатывает первая ошибка при движении справа налево
// parse_point использует is_hex_prefix из string_utils
COMMON_API int parse_point(char* str, struct Point* p);

#endif
