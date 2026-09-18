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
// 5 - число за границами допустимого диапазона
enum { PARSE_TOO_FEW = 1, PARSE_NOT_NUMBER = 2, PARSE_EXTRA = 3, PARSE_NOT_FINITE = 4, PARSE_OUT_OF_RANGE = 5 };

// пропускает ведущ. пробелы и смотрит по префиксу - это 16ричное число?
COMMON_API int is_hex_prefix(const char* ptr);

#endif