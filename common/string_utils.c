#include "string_utils.h"

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// массив символов из входного потока до \0 через динамич. массив
char* get_string(int* len) {
    *len = 0;
    int capacity = 1;
    char *s = (char*)malloc(sizeof(char)), *temp = NULL;
    int ch;
    while ((ch = getchar()) != EOF && ch != '\n') {
        s[*len] = (char)ch;
        ++(*len);

        if (*len >= capacity) {
            capacity *= 2;
            temp = (char*)realloc(s, capacity * sizeof(char));
            // проверка на null от realloc по совету deepseek
            // + free блока, чтобы он не затёрся при возврате null и не было утечки
            if (!temp) {
                free(s);
                *len = -1;
                return NULL;
            }
            s = temp;
        }
    }
    s[*len] = '\0';
    if (ch == EOF && *len == 0)
        *len = -1;

    return s;
}

// проверка на пустоту
int is_empty(const char* s) {
    while (isspace((unsigned char)*s))
        ++s;
    return *s == '\0';
}

// срез пробелов по бокам
char* trim_string(char* s, int* len) {
    // пропуск начальных пробелов
    while (*len > 0 && isspace((unsigned char)*s)) {
        ++s;
        --(*len);
    }
    // конец строки
    while (*len > 0 && isspace((unsigned char)s[*len - 1])) {
        --(*len);
        s[*len] = '\0';
    }

    return s;
}

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

// распознавание целого числа
int parse_int32(const char* s, int32_t* out) {
    // ведущий + допустим только если сразу за ним цифра: strtol принял бы и "+-5",
    // сдвинуть + руками надо (from_chars не умеет плюс), но тогда "+-5" надо отсечь
    const char* first = s;
    if (*first == '+') {
        ++first;
        if (!isdigit((unsigned char)*first))
            return NUMBER_NOT_NUMBER;
    }

    errno = 0;
    char* end = NULL;
    long v = strtol(first, &end, 10);

    // Windows: long == int32, спасёт только ERANGE (значение уже clamp'нуто)
    // Linux:   long 64-битный, ERANGE нет, спасёт сравнение с INT32_*
    if (errno == ERANGE || v < INT32_MIN || v > INT32_MAX)
        return NUMBER_OUT_OF_RANGE;

    // число обязано занимать всю строку: "5x", "5.5", "0x10", "" - не число
    if (end == first || *end != '\0')
        return NUMBER_NOT_NUMBER;

    *out = (int32_t)v;
    return 0;
}
