#define _USE_MATH_DEFINES
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/exit_codes.h"
#include "../common/geometry.h"
#include "../common/string_utils.h"

#define MAX_SIZE 20
#define MIN_SIZE 3

int main(void) {
    char lineN[32];
    if (fgets(lineN, sizeof(lineN), stdin) == NULL) {
        fprintf(stderr, "Получен пустой ввод вместо целого N");
        return no_input;
    }
    // очистка строки от пробельных символов по краям
    int len = (int)strlen(lineN);
    char* clean_n = trim_string(lineN, &len);

    // проверка строки на пустоту после очистки от пробельных символов
    if (is_empty(clean_n)) {
        fprintf(stderr, "Получен пустой ввод вместо целого N");
        return no_input;
    }

    // распознавание - через библиотеч. ф-цию из string_utils
    int32_t N = 0;
    int code = parse_int32(clean_n, &N);
    if (code == INT_PARSE_OUT_OF_RANGE) {
        fprintf(stderr, "N не помещается в 32-битное целое. Получено: %s", clean_n);
        return data;
    }
    if (code == INT_PARSE_NOT_NUMBER) {
        fprintf(stderr, "N должно быть целым числом. Получено: %s", clean_n);
        return data;
    }
    if (N < MIN_SIZE || N > MAX_SIZE) {
        fprintf(stderr, "N должно быть в диапазоне [3;20]. Получено: %s", clean_n);
        return usage;
    }

    // вершины. Х и У это косинусы и синусы от углов
    struct Point points[MAX_SIZE];
    for (int i = 0; i < N; ++i) {
        points[i].x = cos(i * 2 * M_PI / N);
        points[i].y = sin(i * 2 * M_PI / N);
    }

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            if (i == j)
                printf("%8.3f", 0.0);
            else
                printf("%8.3f", sqrt(pow(points[i].x - points[j].x, 2) + pow(points[i].y - points[j].y, 2)));
        }
        printf("\n");
    }

    return 0;
}
