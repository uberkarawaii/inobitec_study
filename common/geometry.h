#ifndef IRINA_GEOMETRY_H
#define IRINA_GEOMETRY_H

struct Point {
    double x;
    double y;
    double z;
};

// распознавание точки. возвращает код:
// 0 - точка распознана
// 1 - мало координат
// 2 - нечисловые данные
int parse_point(char* str, struct Point* p);

#endif