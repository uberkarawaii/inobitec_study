#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/exit_codes.h"
#include "../common/geometry.h"
#include "../common/parse_codes.h"
#include "../common/string_utils.h"

#define MAX_SIZE 20
#define MIN_SIZE 3

int main(int argc, char* argv[]) {
    // если первый аргумент --version - отдаём версию и завершаем
    if (handle_version_flag(argc, argv))
        return 0;

    // получение числа N
    char lineN[32];
    if (fgets(lineN, sizeof(lineN), stdin) == NULL) {
        fprintf(stderr, "Получен пустой ввод вместо целого N");
        return no_input;
    }

    // распознавание - через библиотеч. ф-цию из string_utils
    int32_t N = 0;
    int code = parse_int32(lineN, &N);

    if (code == NUMBER_EMPTY) {
        fprintf(stderr, "Получен пустой ввод вместо целого N");
        return no_input;
    }

    if (code == NUMBER_OUT_OF_RANGE) {
        fprintf(stderr, "N не помещается в 32-битное целое. Получено: %s", lineN);
        return data;
    }
    if (code == NUMBER_NOT_NUMBER) {
        fprintf(stderr, "N должно быть целым числом. Получено: %s", lineN);
        return data;
    }
    if (N < MIN_SIZE || N > MAX_SIZE) {
        fprintf(stderr, "N должно быть в диапазоне [3;20]. Получено: %s", lineN);
        return data;
    }

    // вершины. Х и У это косинусы и синусы от углов
    struct Point points[MAX_SIZE];
    for (int i = 0; i < N; ++i) {
        points[i] = polygon_vertex(i, N);
    }

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            if (i == j)
                printf("%8.3f", 0.0);
            else
                printf("%8.3f", point_distance(points[i], points[j]));
        }
        printf("\n");
    }

    return 0;
}
