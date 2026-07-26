#ifndef IRINA_GEOMETRY_H
#define IRINA_GEOMETRY_H

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

// распознавание точки. возвращает код:
// 0 - точка распознана
// 1 - мало координат
// 2 - нечисловые данные
COMMON_API int parse_point(char* str, struct Point* p);

#endif