#include "geometry.h"

#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

int is_hex_prefix(const char* ptr) {
    // пропуск пробелов руками, т.к. надо проверить префикс
    ptr += strspn(ptr, " \t\n\r");
    if (*ptr == '\0')
        return 0;
    // если только префикс типа 0x, то падение и так будет. здесь проверка на 0xчисло
    int pad = 0;
    // +/- перед hex числом - допустимо
    if (*ptr == '+' || *ptr == '-')
        ++pad;
    // если есть \0, то остановка будет точно на нём. и после - выход из if
    if (*(ptr + pad) == '0' && (*(ptr + pad + 1) == 'x' || *(ptr + pad + 1) == 'X'))
        return 1;
    return 0;
}

// распознавание точки. только для непустых строк
// коды возможных ошибок:
// PARSE_TOO_FEW - 1 - мало координат
// PARSE_NOT_NUMBER - 2 - нечисловые данные
// PARSE_EXTRA - 3 - много координат
// PARSE_NOT_FINITE - 4 - не конечное число
// PARSE_OUT_OF_RANGE - 5 - число за границами допустимого диапазона
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
            return PARSE_NOT_NUMBER;
        // ошибка в ноль до попытки распознать
        errno = 0;
        // попытка распознать число
        d[i] = strtod(pointer, &end);
        // если при распознавании underflow/overflow
        if (errno == ERANGE && (d[i] == 0.0 || !isfinite(d[i])))
            return PARSE_OUT_OF_RANGE;
        // проверка именно того, что это не nan / inf
        if (!isfinite(d[i]))
            return PARSE_NOT_FINITE;
        // проверить - м.б. нечисловые данные;
        // это будет если остановка не на пробеле/конце или в самом начале строки уже нечисловые данные,
        //  тогда end == pointer
        if (end == pointer || (*end != '\0' && strchr(" \t\n\r", *end) == NULL))
            return PARSE_NOT_NUMBER;
        // продвижение указателя в место, где окончилось распознавание числа
        pointer = end;
        // сдвиг на к-во пробелов после числа. нужно, т.к. при проверке на 4й арг. пробелы мешаются
        pointer += strspn(pointer, " \t\n\r");
        ++i;
    }

    // если прошли меньше раз, а строка закончилась - координат меньше нужного
    if (i != 3)
        return PARSE_TOO_FEW;

    // если прошли достаточно раз, а дальше что-то ещё (пробелы после 3 числа уже срезаны) - или лишнее число, или
    // символ
    if (i == 3 && *pointer != '\0') {
        // если это hex число, то воспринимаем его как не-число. а не как "лишний арг после x y z"
        if (is_hex_prefix(pointer))
            return PARSE_NOT_NUMBER;
        // парс того что за X Y Z. хранить это нет смысла, так что без переменной
        errno = 0;
        double extra = strtod(pointer, &end);
        // если при распознавании underflow/overflow
        if (errno == ERANGE && (extra == 0.0 || !isfinite(extra)))
            return PARSE_OUT_OF_RANGE;
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