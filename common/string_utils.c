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
    // пропуск пробелов руками, т.к. надо проверить префикс. через isspace,
    // не ручной набор пробел. символов. иначе ассиметрия с с++
    while (isspace((unsigned char)*ptr))
        ++ptr;
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
    // указат на начало и конец
    const char *start = s, *end = s + (int)strlen(s);
    while (isspace((unsigned char)*start) && *start != '\0')
        ++start;
    while (end > start && isspace((unsigned char)*(end - 1)))
        --end;
    // теперь указатели по краям содержимого. если его нет и они в одном месте - строка пустая
    if (start == end)
        return NUMBER_EMPTY;

    // ведущий + допустим только если сразу за ним цифра: strtol принял бы и "+-5",
    // сдвинуть + руками надо (from_chars не умеет плюс), но тогда "+-5" надо отсечь
    if (*start == '+') {
        ++start;
        if (!isdigit((unsigned char)*start))
            return NUMBER_NOT_NUMBER;
    }

    errno = 0;
    char* end_recogn = NULL;
    long v = strtol(start, &end_recogn, 10);

    // Windows: long == int32, спасёт только ERANGE (значение уже clamp'нуто)
    // Linux:   long 64-битный, ERANGE нет, спасёт сравнение с INT32_*
    if (errno == ERANGE || v < INT32_MIN || v > INT32_MAX)
        return NUMBER_OUT_OF_RANGE;

    // если конец распозн. от strtol не там же, где найденный конец данных end, то есть лишние символы
    if (end_recogn != end)
        return NUMBER_NOT_NUMBER;

    *out = (int32_t)v;
    return 0;
}
