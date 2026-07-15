#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "..\common\exit_codes.h"
#include "..\common\geometry.h"
#include "..\common\string_utils.h"

int main() {

    int len;
    char* s = NULL;

    // массив точек и проверка выделени€ пам€ти
    int points_size = 0, points_capacity = 1;
    struct Point* points = malloc(sizeof(struct Point));
    if (!points) {
        fprintf(stderr, "Ќе удалось выделить пам€ть\n");
        return io_fail;
    }

    // центроид
    struct Point center = {.x = 0, .y = 0, .z = 0};

    // счЄтчик дл€ вывода ошибок
    int i = 0;
    while (1) {
        // текуща€ строка
        s = get_string(&len);
        ++i;

        // сразу после прочтени€ проверка на проблему с IO
        if (len == -1) {
            if (ferror(stdin)) {
                fprintf(stderr, "ќшибка в IO\n");
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

        // проверка на пустоту. если пусто, пропуск
        if (is_empty(s)) {
            free(s);
            s = NULL;
            continue;
        }

        // распознавание X Y Z через parse_point
        struct Point p;
        int ex_code = parse_point(s, &p);
        // при ненулевом коде ошибки - его обработка
        // недостаточно данных
        if (ex_code == 1) {
            fprintf(stderr, "—трока %d. ќжидались координаты X Y Z. ѕолучено: %s\n", i, s);
            free(s);
            s = NULL;
            return data;
        }
        // нечисловые данные
        if (ex_code == 2) {
            fprintf(stderr, "—трока %d. Ќечисловые данные: %s\n", i, s);
            free(s);
            s = NULL;
            return data;
        }
        // если дошли до сюда, код выхода == 0 и распознавание произошло
        points[points_size] = p;
        points_size++;

        // если размер >= вместимость то надо увеличить вместимость
        if (points_size >= points_capacity) {
            points_capacity *= 2;
            struct Point* tmp = realloc(points, points_capacity * sizeof(*tmp));
            // если не удалось выделить пам€ть
            if (!tmp) {
                fprintf(stderr, "ќшибка при выделении пам€ти\n");
                free(s);
                free(points);
                exit(io_fail);
            }
            points = tmp;
        }

        // суммирование точек в центроид
        center.x += p.x;
        center.y += p.y;
        center.z += p.z;

        // освобождение s т.к. в неЄ положитс€ нова€ строка
        free(s);
        s = NULL;
    }

    // если после прохода длина массива == 0 то входных данных не было
    if (points_size == 0) {
        fprintf(stderr, "¬ходные данные отсутствуют\n");
        free(s);
        free(points);
        return no_input;
    }

    // деление накопленных в центроиде чисел на кол-во точек
    center.x /= points_size;
    center.y /= points_size;
    center.z /= points_size;

    // границы - минимумы и максимумы
    double max[3] = {points[0].x, points[0].y, points[0].z};
    double min[3] = {points[0].x, points[0].y, points[0].z};
    // среднее рассто€ние до центроида
    double d = 0;

    i = 0;
    for (; i < points_size; ++i) {
        if (max[0] < points[i].x)
            max[0] = points[i].x;
        if (max[1] < points[i].y)
            max[1] = points[i].y;
        if (max[2] < points[i].z)
            max[2] = points[i].z;

        if (min[0] > points[i].x)
            min[0] = points[i].x;
        if (min[1] > points[i].y)
            min[1] = points[i].y;
        if (min[2] > points[i].z)
            min[2] = points[i].z;

        d += sqrt((points[i].x - center.x) * (points[i].x - center.x) +
                  (points[i].y - center.y) * (points[i].y - center.y) +
                  (points[i].z - center.z) * (points[i].z - center.z));
    }

    // деление накопленного рассто€ни€ на кол-во точек
    d /= points_size;

    // вывод параметров
    printf(" оличество точек: %d\n", points_size);
    printf(" оординаты огранич. паралеллепипеда x: [ %.3f; %.3f ] y: [ %.3f; %.3f ] z: [ %.3f; %.3f ]\n", min[0],
           max[0], min[1], max[1], min[2], max[2]);
    printf(" оординаты центроида x: %.3f, y: %.3f, z: %.3f\n", center.x, center.y, center.z);
    printf("—реднее рассто€ние от точек до центроида: %.3f\n", d);

    free(s);
    free(points);
    return 0;
}