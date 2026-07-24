#include "geometry.h"

#include <stdlib.h>
#include <string.h>

// распознавание точки
// 0 - точка распознана
// 1 - мало координат
// 2 - нечисловые данные
int parse_point(char* str, struct Point* p) {
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