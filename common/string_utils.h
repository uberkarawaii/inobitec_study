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

    char* start_ptr = str;
    char* end_ptr;
    double d[3];
    int j = 0;
    while (j < 3) {
        // срез пробелов в начале
        while (isspace((unsigned char)*start_ptr))
            ++start_ptr;
        // если уже конец на указателе начала - хотя start_ptr должен остновиться на числовом символе минимум 3 раза
        // - то это значит, что чисел меньше трёх
        if (*start_ptr == '\0')
            return 1;
        d[j] = strtod(start_ptr, &end_ptr);
        // распознавание не началось или остановилось на не пробельном / не конечном символе
        if (!isspace((unsigned char)*end_ptr) && *end_ptr != '\0')
            return 2;
        start_ptr = end_ptr;
        ++j;
    }

    *p = (struct Point){.x = d[0], .y = d[1], .z = d[2]};

    return 0;
}

// срез пробелов по бокам
static inline char* trim_string(char* s, int* len) {
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

#endif