#include <ctype.h>
#include <locale.h>
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

int main() {

    int len;
    char* s = NULL;

    // массив точек
    int points_size = 0, points_capacity = 1;
    struct Point* points = malloc(sizeof(struct Point));
    // центроид
    struct Point center = {.x = 0, .y = 0, .z = 0};

    // счЄтчик дл€ вывода ошибок
    int i = 0;
    while (!feof(stdin)) {
        // текуща€ строка
        s = get_string(&len);
        ++i;
        // сразу после прочтени€ проверка на проблему с IO
        if (len == -1 && ferror(stdin)) {
            fprintf(stderr, "ќшибка в IO\n");
            free(s);
            s = NULL;
            return io_fail;
        }

        // обрезка пробелов по кра€м
        char* clean_s = trim_string(s, &len);
        // если после обрезки строка пуста, пропуск, идЄм к след. строке
        if (strlen(clean_s) == 0) {
            free(s);
            s = NULL;
            continue;
        }

        // нахождение символов пробелов слева и справа в очищенной строке. если они не совпадают,
        // значит между ними есть три элемента, которые можно попробовать распознать
        // иначе - там не достаточно элементов
        if (strchr(clean_s, ' ') == strrchr(clean_s, ' ')) {
            fprintf(stderr, "—трока %d. ќжидались координаты X Y Z. ѕолучено: %s\n", i, clean_s);
            free(s);
            s = NULL;
            return data;
        }
        // распознавание чисел в очищенной строке
        char* start_ptr = clean_s;
        char* end_ptr;
        double d[3];
        int j = 0;
        while (j < 3) {
            d[j] = strtod(start_ptr, &end_ptr);
            if (end_ptr == start_ptr || (*end_ptr != ' ' && *end_ptr != '\0')) {
                fprintf(stderr, "—трока %d. Ќечисловые данные: %s\n", i, clean_s);
                free(s);
                s = NULL;
                return data;
            }
            start_ptr = end_ptr;
            ++j;
        }

        // суммаирование в центроид
        center.x += d[0];
        center.y += d[1];
        center.z += d[2];

        // текущие x y z кладутс€ в общий массив
        points[points_size] = (struct Point){.x = d[0], .y = d[1], .z = d[2]};
        points_size++;
        // если размер >= вместимость то надо увеличить вместимость
        if (points_size >= points_capacity) {
            points_capacity *= 2;
            struct Point* tmp = realloc(points, points_capacity * sizeof(*tmp));
            if (!tmp) {
                fprintf(stderr, "ќшибка при выделении пам€ти\n");
                free(s);
                exit(EXIT_FAILURE);
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