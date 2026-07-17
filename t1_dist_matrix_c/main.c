#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/exit_codes.h"
#include "../common/geometry.h"

#define MAX_SIZE 20
#define MIN_SIZE 3

int main(void) {
    char lineN[32];
    if (fgets(lineN, sizeof(lineN), stdin) == NULL || lineN[0] == '\n' || lineN[0] == '\r') {
        fprintf(stderr, "Получен пустой ввод вместо целого N");
        return no_input;
    }
    // очистка строки от служебных символов на конце
    size_t sz = strlen(lineN) - 1;
    while (sz > 0 && (lineN[sz] == '\n' || lineN[sz] == '\r' || lineN[sz] == ' ')) {
        lineN[sz] = '\0';
        --sz;
    }
    // если после очистки единств. символ - симв. конца строки, то это пустая строка
    if (lineN[0] == '\0') {
        fprintf(stderr, "Получен пустой ввод вместо целого N");
        return no_input;
    }

    // указат на указат на 1й символ, который strtol не сможет расшифровать
    char* endptr;
    long int N = strtol(lineN, &endptr, 10);
    if (*endptr != '\n' && *endptr != '\0') {
        fprintf(stderr, "N должно быть целым числом. Получено: %s", lineN);
        return data;
    }

    if (N < MIN_SIZE || N > MAX_SIZE) {
        fprintf(stderr, "N должно быть в диапазоне [3;20]. Получено: %s", lineN);
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