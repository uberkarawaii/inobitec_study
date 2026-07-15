#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "..\common\exit_codes.h"
#include "..\common\string_utils.h"

struct Point {
    double x;
    double y;
    double z;
};

int main(int argc, char* argv[]) {
    // проверки радиуса - кол-во аргументов и сам радиус (число ли, конечен ли, неотрицателен ли)

    if (argc < 2) {
        fprintf(stderr, "Ожидался радиус; его значение не было введено\n");
        return usage;
    }

    if (argc > 2) {
        fprintf(stderr, "Ожидался радиус; были введены лишние аргументы\n");
        return usage;
    }

    char* end_r;
    double r = strtod(argv[1], &end_r);
    if (*end_r != '\0') {
        fprintf(stderr, "Радиус должен быть числом. Получено: %s\n", argv[1]);
        return usage;
    }

    if (!isfinite(r)) {
        fprintf(stderr, "Радиус должен быть конечным числом. Получено: %s\n", argv[1]);
        return usage;
    }

    if (r <= 0) {
        fprintf(stderr, "Радиус должен быть положительным. Получено: %.3f\n", r);
        return usage;
    }

    // счётчик для вывода ошибок и длина текущей строки
    int i = 0, len = 0;
    // указатель на динамич. массив в последующем
    char* s;
    // размер и вместимость динамич. массива
    int sz = 0, capacity = 1;
    // выделение памяти под массив и проверка того, что удалось её выделить
    struct Point* points = malloc(sizeof(*points));
    if (!points) {
        fprintf(stderr, "Не удалось выделить память\n");
        return io_fail;
    }

    // считывание строк с точками
    while (1) {
        s = get_string(&len);
        ++i;
        // проверка ошибки в IO
        if (len == -1 && ferror(stdin)) {
            fprintf(stderr, "Ошибка в IO\n");
            free(s);
            s = NULL;
            return io_fail;
        }

        // проверка конца ввода
        if (len == -1) {
            free(s);
            break;
        }

        // очистка от пробелов по краям, проверка на пустоту
        char* clean_s = trim_string(s, &len);
        if (len == 0) {
            free(s);
            s = NULL;
            continue;
        }

        // распознвание чисел в строке
        int j = 0;
        double point[3];
        char* start = clean_s;
        char* end;

        while (j < 3) {

            point[j] = strtod(start, &end);

            // если остановка не на пробеле и не в конце, то в строке есть нечисловой символ
            if ((*end != ' ' && *end != '\0')) {
                fprintf(stderr, "Строка %d. Нечисловое значение: %s\n", i, clean_s);
                free(s);
                s = NULL;
                free(points);
                return data;
            }

            // если указатель остановки уже на конце, а при этом аргументов не 3шт, то их меньше нужного
            if (*end == '\0' && j != 2) {
                fprintf(stderr, "Строка %d. Ожидалось X Y Z, получено: %s\n", i, clean_s);
                free(s);
                s = NULL;
                free(points);
                return data;
            }

            // пропуск пустых символов
            start = end;
            while (start != clean_s + len && *start == ' ')
                ++start;
            ++j;
        }

        // после удачного распознавания можно положить значения в массив точек
        points[sz] = (struct Point){.x = point[0], .y = point[1], .z = point[2]};
        ++sz;

        // как только размер == вместимость, надо расширить вместимость
        if (sz >= capacity) {
            capacity *= 2;
            struct Point* p = realloc(points, capacity * sizeof(*p));
            if (!p) {
                fprintf(stderr, "Ошибка при выделении памяти\n");
                free(s);
                free(points);
                exit(io_fail);
            }
            points = p;
        }

        // освободить s т.к. в неё положится новая строка
        free(s);
        s = NULL;
    }

    // если после чтения массив точек пуст, то значения точек не были введены
    if (sz == 0) {
        fprintf(stderr, "Точки отсутствуют\n");
        return no_input;
    }

    // вывод точек, у которых расст. до центра < r
    for (int k = 0; k < sz; ++k) {
        if (sqrt(points[k].x * points[k].x + points[k].y * points[k].y + points[k].z * points[k].z) < r)
            printf("%.3f %.3f %.3f\n", points[k].x, points[k].y, points[k].z);
    }

    free(points);
    return 0;
}