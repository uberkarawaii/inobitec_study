#include "geometry.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

// распознавание точки. коды возможных ошибок:
// PARSE_TOO_FEW - 1 - мало координат
// PARSE_NOT_NUMBER - 2 - нечисловые данные
// PARSE_EXTRA - 3 - много координат
// PARSE_NOT_FINITE - 4 - не конечное число
int parse_point(char* str, struct Point* p) {
    // указатель на начало строки
    char* pointer = str;
    // пустой указатель, туда будет попадать конец распознавания числа
    char* end = NULL;

    int i = 0;
    double d[3];
    while (i < 3 && *pointer != '\0') {
        // попытка распознать число
        d[i] = strtod(pointer, &end);
        // проверка что это не nan / inf
        if (!isfinite(d[i]))
            return PARSE_NOT_FINITE;
        // проверить - м.б. нечисловые данные;
        // это будет если остановка не на пробеле/конце или в самом начале строки уже нечисловые данные,
        //  тогда end == pointer
        if (end == pointer || (*end != '\0' && strchr(" \t\n\r", *end) == NULL))
            return PARSE_NOT_NUMBER;
        // продвижение указателя в место, где окончилось распознавание числа
        pointer = end;
        // сдвигаем указатель на кол-во пробелов после числа
        pointer += strspn(pointer, " \t\n\r");
        ++i;
    }

    // если прошли меньше раз, а строка закончилась - координат меньше нужного
    if (i != 3)
        return PARSE_TOO_FEW;

    // если прошли достаточно раз, а дальше что-то ещё (пробелы после 3 числа уже срезаны) - или лишнее число, или
    // символ
    if (i == 3 && *pointer != '\0') {
        // парс того что за X Y Z. хранить это нет смысла, так что без переменной
        (void)strtod(pointer, &end);
        // если с самого начала символ (end == ptr) или end остановился не на пробельном - значит проблема в не-числе
        if (end == pointer || (*end != '\0' && strchr(" \t\n\r", *end) == NULL))
            return PARSE_NOT_NUMBER;
        // иначе - это лишнее число
        else
            return PARSE_EXTRA;
    }

    *p = (struct Point){.x = d[0], .y = d[1], .z = d[2]};

    return 0;
}