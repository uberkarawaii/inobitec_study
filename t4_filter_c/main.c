#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "..\common\exit_codes.h"
#include "..\common\geometry.h"
#include "..\common\string_utils.h"

int get_points(double r) {
    // счётчик для вывода ошибок и длина текущей строки
    int i = 0, len = 0;
    // счётчик удачно прочитанных строк
    int ctr = 0;
    // указатель на динамич. массив в последующем
    char* s;

    // считывание строк с точками
    while (1) {
        s = get_string(&len);
        ++i;

        // сразу после прочтения проверка на проблему с IO или конец вход. потока
        if (len == -1) {
            free(s);
            s = NULL;

            if (ferror(stdin)) {
                fprintf(stderr, "Ошибка в IO\n");
                return io_fail;
            }
            // если len == -1 без проблем с потоком ошибок, то был достигнут EOF
            else
                break;
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

        if (ex_code != 0) {
            // если мало аргументов
            if (ex_code == 1)
                fprintf(stderr, "Строка %d. Ожидалось X Y Z, получено: %s\n", i, s);
            // если значение нечисловое
            else
                fprintf(stderr, "Строка %d. Нечисловое значение: %s\n", i, s);
            free(s);
            s = NULL;
            return data;
        }

        // если дошли сюда, то код возврата из parse_points == 0 - распознавание удачно
        // + 1 прочитанная точка
        ++ctr;

        // вывод точек, у которых расст. до центра < r
        if (sqrt(p.x * p.x + p.y * p.y + p.z * p.z) < r)
            printf("%.3f %.3f %.3f\n", p.x, p.y, p.z);

        // освободить s т.к. в неё положится новая строка
        free(s);
        s = NULL;
    }

    if (ctr == 0) {
        fprintf(stderr, "Точки отсутствуют\n");
        free(s);
        return no_input;
    }

    return 0;
}

int main(int argc, char* argv[]) {
    // для отложенного вывода буфера stdout
    // освобождение произойдёт в момент return
    setvbuf(stdout, NULL, _IOFBF, 4096);

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

    // вызов вспомогат. ф-ции.если код возвр. != 0, она уже что-то напечатала
    // и можно вернуть этот же код
    int status = get_points(r);
    if (status != 0)
        return status;

    return 0;
}