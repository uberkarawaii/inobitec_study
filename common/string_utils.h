#ifndef IRINA_STRING_UTILS_H
#define IRINA_STRING_UTILS_H

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "geometry.h"

// static - локальная копия для файлов, где будет ф-ция
// inline - прописать функцию в файле
static inline char* get_string(int* len) {
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
    if (ch == EOF)
        *len = -1;

    return s;
}

// проверка на пустоту
static inline int is_empty(const char* s) {
    while (isspace((unsigned char)*s))
        ++s;
    return *s == '\0';
}

// распознавание точки
// 0 - точка распознана
// 1 - мало координат
// 2 - нечисловые данные
static inline int parse_point(char* str, struct Point* p) {
    // копия т.к. strtok_s будет делить через \0 и исход. строка разрушится
    // выделение места и копирование в него
    char* copy = malloc(strlen(str) + 1);
    strcpy_s(copy, strlen(str) + 1, str);
    // указатель на начала строк для strtok_s
    char* next_token = NULL;
    // то что будет отделено от след. последовательности символами " \t\n\r"
    char* pch = strtok_s(copy, " \t\n\r", &next_token);
    int i = 0;
    double d[3];
    while (pch != NULL && i < 3) {
        char* end_ptr = NULL;
        d[i] = strtod(pch, &end_ptr);

        // если парс числа остановился не не-числовом символе, значит он есть в строке
        if (*end_ptr != '\0') {
            free(copy);
            return 2;
        }

        // если указать null как входной парам., сканирование продолжится
        // с того места, где останов. в прошлый раз
        pch = strtok_s(NULL, " \t\n\r", &next_token);
        ++i;
    }

    // если прошли меньше раз или дальше ещё что-то было для распознвания - есть лишние символы
    if (i != 3 || pch != NULL) {
        free(copy);
        return 1;
    }

    *p = (struct Point){.x = d[0], .y = d[1], .z = d[2]};
    // т.к. strdup использует malloc
    free(copy);
    return 0;
}

// срез пробелов по бокам
static inline char* trim_string(char* s, int* len) {
    // пропуск начальных пробелов
    size_t gap = strspn(s, " \t\n\r");
    *len -= (int)gap;
    s += gap;
    // конец строки
    while (*len > 0 && isspace((unsigned char)s[*len - 1])) {
        --(*len);
        s[*len] = '\0';
    }

    return s;
}

#endif