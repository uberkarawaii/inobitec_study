#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../common/cli.h"
#include "../common/exit_codes.h"
#include "../common/parse_codes.h"
#include "../common/string_utils.h"

static const char HELP_TEXT[] = "t2_passport: паспорт фигуры\n"
                                "Использование: [--help] [--version]\n"
                                "Вход (stdin): название фигуры (строка, может быть из нескольких слов), затем\n"
                                "целое число вершин V > 0.\n"
                                "Выводит: Фигура «<название>»: <V> <вершина|вершины|вершин>.\n"
                                "Коды возврата: 0 — успех; 65 — некорректные данные; 66 — нет входных данных.";

int main(int argc, char* argv[]) {
    // если первый аргумент --version - отдаём версию и завершаем
    if (handle_version_flag(argc, argv))
        return 0;
    // если первый арг. --help/-h - выводим справку и завершаем
    if (handle_help_flag(argc, argv, HELP_TEXT))
        return 0;

    //  считывание строки полностью
    int len_name;
    char* s = get_string(&len_name);

    // если из string_utils вернулась нулевая длина, то строка сост. из символа конца строки
    if (len_name == -1) {
        free(s);
        fprintf(stderr, "EOF вместо имени фигуры\n");
        return no_input;
    }

    // очистить строку от пробелов в начале и в конце
    char* clean_s = trim_string(s, &len_name);
    // если дошли до сюда и длина 0, то значит строка полностю состояла из пробелов
    if (len_name == 0) {
        free(s);
        // clean_s освободится автоматически, т.к. s окружающее его было освобождено
        fprintf(stderr, "Пустой ввод вместо имени фигуры\n");
        return no_input;
    }
    // считывание след. строки с числом вершин
    int len_num;
    char* s_num = get_string(&len_num);
    // также, если длина = -1, то это полностью пустая строка
    if (len_num == -1) {
        free(s);
        free(s_num);
        fprintf(stderr, "EOF вместо числа вершин\n");
        return no_input;
    }

    // распознавание числа библиотечной ф-цией из string_utils
    int32_t N = 0;
    int code = parse_int32(s_num, &N);

    if (code == NUMBER_EMPTY) {
        fprintf(stderr, "Пустой ввод вместо кол-ва вершин\n");
        free(s);
        free(s_num);
        return no_input;
    }

    if (code == NUMBER_OUT_OF_RANGE) {
        fprintf(stderr, "Кол-во вершин не помещается в 32-битное целое. Получено: %s\n", s_num);
        free(s);
        free(s_num);
        return data;
    }
    if (code == NUMBER_NOT_NUMBER) {
        fprintf(stderr, "Кол-во вершин должно быть целым числом. Получено: %s\n", s_num);
        free(s);
        free(s_num);
        return data;
    }
    if (N < 1) {
        fprintf(stderr, "Кол-во вершин должно быть положительным. Получено: %" PRId32 "\n", N);
        free(s);
        free(s_num);
        return data;
    }

    // массив словоформ со склонением
    const char* words[] = {"вершина", "вершины", "вершин"};
    // форматный вывод со склонением
    printf("Фигура «%s»: %" PRId32 " %s.\n", clean_s, N, words[vertex_form_index(N)]);
    free(s);
    free(s_num);
    return 0;
}
