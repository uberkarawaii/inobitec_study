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
COMMON_API int parse_point(const char* str, struct Point* p);

// вычисляет расст. м-у двумя точками в 3д
// предполагается, что обе точки валидные
COMMON_API double point_distance(struct Point a, struct Point b);

// среднее арифметическое набора точек
// по контракту предполагается, что набор точек будет непустым и N - положительным
COMMON_API struct Point centroid(const struct Point* pts, int n);

// для получения вершин правильного N-угольника
COMMON_API struct Point polygon_vertex(int i, int n);

#endif
