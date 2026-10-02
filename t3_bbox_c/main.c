#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "../common/exit_codes.h"
#include "../common/geometry.h"
#include "../common/parse_codes.h"
#include "../common/string_utils.h"

int main(int argc, char* argv[]) {
    // если первый аргумент --version - отдаЄм версию и завершаем
    if (handle_version_flag(argc, argv))
        return 0;

    int len;
    char* s = NULL;

    // массив точек и проверка выделени€ пам€ти
    int points_size = 0, points_capacity = 1;
    struct Point* points = malloc(sizeof(struct Point));
    if (!points) {
        fprintf(stderr, "Ќе удалось выделить пам€ть\n");
        return io_fail;
    }

    // счЄтчик дл€ вывода ошибок
    int i = 0;
    while (1) {
        // текуща€ строка
        s = get_string(&len);
        ++i;

        // сразу после прочтени€ проверка на проблему с IO
        if (len == -1) {
            if (ferror(stdin)) {
                fprintf(stderr, "—трока %d - сбой ввода-вывода\n", i);
                free(s);
                s = NULL;
                free(points);
                points = NULL;
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
        if (ex_code != 0) {
            // недостаточно данных
            if (ex_code == NUMBER_TOO_FEW)
                fprintf(stderr, "—трока %d - недостаточно координат. ќжидалось X Y Z, получено: %s\n", i, s);

            // нечисловые данные
            else if (ex_code == NUMBER_NOT_NUMBER)
                fprintf(stderr, "—трока %d. Ќечисловые данные: %s\n", i, s);

            // слишком много координат
            else if (ex_code == NUMBER_TOO_MUCH)
                fprintf(stderr, "—трока %d - слишком много координат. ќжидалось X Y Z, получено: %s\n", i, s);

            // одна из координат это inf или Nan
            else if (ex_code == NUMBER_NOT_FINITE)
                fprintf(stderr, "—трока %d. —реди X Y Z обнаружена не конечна€ координата: %s\n", i, s);

            // в точке есть число выход€щее за диапазон допустимого
            else if (ex_code == NUMBER_OUT_OF_RANGE)
                fprintf(stderr, "—трока %d. —реди X Y Z обнаружена координата, выход€ща€ за допустимый диапазон: %s\n",
                        i, s);

            free(s);
            s = NULL;
            free(points);
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

    // центроид
    struct Point center = centroid(points, points_size);
    // среднее расст. до центроида
    double d = 0;
    // границы - минимумы и максимумы
    double max[3] = {points[0].x, points[0].y, points[0].z};
    double min[3] = {points[0].x, points[0].y, points[0].z};

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

        d += point_distance(points[i], center);
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