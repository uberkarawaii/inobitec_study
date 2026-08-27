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

struct Point {
    double x;
    double y;
    double z;
};

// распознавание точки. возвращает код из enum-ы
// срабатывает первая ошибка при движении справа налево
COMMON_API int parse_point(char* str, struct Point* p);

// 1 - мало координат
// 2 - нечисловые данные
// 3 - много координат
// 4 - не конечное число
enum { PARSE_TOO_FEW = 1, PARSE_NOT_NUMBER = 2, PARSE_EXTRA = 3, PARSE_NOT_FINITE = 4 };

#endif