#include "geometry.h"

#include <stdlib.h>
#include <string.h>

// распознавание точки
// 0 - точка распознана
// 1 - мало координат
// 2 - нечисловые данные
int parse_point(char* str, struct Point* p) {

    char* pointer = str;
    char* end = NULL;

    int i = 0;
    double d[3];
    while (i < 3 && *pointer != '\0') {
        // попытка распознать число
        d[i] = strtod(pointer, &end);
        // нечисловые данные; плохо если остановка не на пробеле/конце или в самом начале строки уже нечисловые данные,
        // тогда end == pointer
        if (end == pointer || (*end != '\0' && strchr(" \t\n\r", *end) == NULL))
            return 2;
        // продвижение указателя в место, где окончилось распознавание числа
        pointer = end;
        // сдвигаем указатель на кол-во пробелов после числа
        pointer += strspn(pointer, " \t\n\r");
        ++i;
    }

    // если прошли меньше раз или дальше ещё что-то было для распознвания - есть лишние символы
    if (i != 3 || *pointer != '\0')
        return 1;

    *p = (struct Point){.x = d[0], .y = d[1], .z = d[2]};

    return 0;
}