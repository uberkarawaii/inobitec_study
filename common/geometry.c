#include "geometry.h"

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>

#include "string_utils.h"

// распознавание точки. только для непустых строк
// коды возможных ошибок:
int parse_point(char* str, struct Point* p) {
    // указатель на начало строки
    char* pointer = str;
    // пустой указатель, туда будет попадать конец распознавания числа
    char* end = NULL;

    int i = 0;
    double d[3];
    while (i < 3 && *pointer != '\0') {
        // проверка префикса 16сс. если есть - воспринимаем как некорректное
        if (is_hex_prefix(pointer))
            return NUMBER_NOT_NUMBER;
        // ошибка в ноль до попытки распознать
        errno = 0;
        // попытка распознать число
        d[i] = strtod(pointer, &end);
        // если при распознавании underflow/overflow
        if (errno == ERANGE && (d[i] == 0.0 || !isfinite(d[i])))
            return NUMBER_OUT_OF_RANGE;
        // проверка именно того, что это не nan / inf
        if (!isfinite(d[i]))
            return NUMBER_NOT_FINITE;
        // проверить - м.б. нечисловые данные;
        // это будет если остановка не на пробеле/конце или в самом начале строки уже нечисловые данные,
        //  тогда end == pointer
        if (end == pointer || (*end != '\0' && !isspace((unsigned char)*end)))
            return NUMBER_NOT_NUMBER;
        // продвижение указателя в место, где окончилось распознавание числа
        pointer = end;
        // сдвиг на к-во пробелов после числа. нужно, т.к. при проверке на 4й арг. пробелы мешаются
        while (isspace((unsigned char)*pointer))
            ++pointer;
        ++i;
    }

    // если прошли меньше раз, а строка закончилась - координат меньше нужного
    if (i != 3)
        return NUMBER_TOO_FEW;

    // если прошли достаточно раз, а дальше что-то ещё (пробелы после 3 числа уже срезаны) - или лишнее число, или
    // символ
    if (i == 3 && *pointer != '\0') {
        // если это hex число, то воспринимаем его как не-число. а не как "лишний арг после x y z"
        if (is_hex_prefix(pointer))
            return NUMBER_NOT_NUMBER;
        // парс того что за X Y Z. хранить это нет смысла, так что без переменной
        errno = 0;
        double extra = strtod(pointer, &end);
        // если при распознавании underflow/overflow
        if (errno == ERANGE && (extra == 0.0 || !isfinite(extra)))
            return NUMBER_OUT_OF_RANGE;
        // если с самого начала символ (end == ptr) или end остановился не на пробельном - значит проблема в не-числе
        if (end == pointer || (*end != '\0' && !isspace((unsigned char)*end)))
            return NUMBER_NOT_NUMBER;
        // иначе - это лишнее число
        else
            return NUMBER_TOO_MUCH;
    }

    *p = (struct Point){.x = d[0], .y = d[1], .z = d[2]};

    return 0;
}
