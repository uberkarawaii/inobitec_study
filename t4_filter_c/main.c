#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "..\common\exit_codes.h"
#include "..\common\geometry.h"
#include "..\common\string_utils.h"

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

        // сразу после прочтения проверка на проблему с IO или конец вход. потока
        if (len == -1) {
            if (ferror(stdin)) {
                fprintf(stderr, "Ошибка в IO\n");
                free(s);
                s = NULL;
                return io_fail;
            }
            // если len == -1 без проблем с потоком ошибок, то был достигнут EOF
            else {
                free(s);
                s = NULL;
                break;
            }
        }

        // проверка на пустоту; если пусто, пропускаем
        if (is_empty(s)) {
            free(s);
            s = NULL;
            continue;
        }

        // распознвание чисел в строке через parse_point
        struct Point p;
        int ex_code = parse_point(s, &p);

        // если мало аргументов
        if (ex_code == 1) {
            fprintf(stderr, "Строка %d. Ожидалось X Y Z, получено: %s\n", i, s);
            free(s);
            s = NULL;
            free(points);
            return data;
        }
        // если значение нечисловое
        if (ex_code == 2) {
            fprintf(stderr, "Строка %d. Нечисловое значение: %s\n", i, s);
            free(s);
            s = NULL;
            free(points);
            return data;
        }

        // если дошли сюда, то код возврата из parse_points == 0 - распознавание удачно
        points[sz] = p;
        ++sz;

        // как только размер == вместимость, надо расширить вместимость
        if (sz >= capacity) {
            capacity *= 2;
            struct Point* temp_points = realloc(points, capacity * sizeof(*temp_points));
            if (!temp_points) {
                fprintf(stderr, "Ошибка при выделении памяти\n");
                free(s);
                free(points);
                exit(io_fail);
            }
            points = temp_points;
        }

        // освободить s т.к. в неё положится новая строка
        free(s);
        s = NULL;
    }

    // если после чтения массив точек пуст, то значения точек не были введены
    if (sz == 0) {
        fprintf(stderr, "Точки отсутствуют\n");
        free(s);
        free(points);
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