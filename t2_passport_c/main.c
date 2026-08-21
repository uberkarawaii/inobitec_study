#include <ctype.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

#include "../common/exit_codes.h"
#include "../common/string_utils.h"

int get_vertex_name(int N) {
    if ((N / 10) % 10 != 1 && N % 10 == 1)
        return 0;
    else if ((N / 10) % 10 != 1 && N % 10 >= 2 && N % 10 <= 4)
        return 1;
    else
        return 2;
}

int main() {
    //  считывание строки полностью
    int len_name;
    char* s = get_string(&len_name);

    // если из string_utils вернулась нулева€ длина, то строка сост. из символа конца строки
    if (len_name == -1) {
        free(s);
        fprintf(stderr, "EOF вместо имени фигуры\n");
        return no_input;
    }

    // очистить строку от пробелов в начале и в конце
    char* clean_s = trim_string(s, &len_name);
    // если дошли до сюда и длина 0, то значит строка полностю состо€ла из пробелов
    if (len_name == 0) {
        free(s);
        // clean_s освободитс€ автоматически, т.к. s окружающее его было освобождено
        fprintf(stderr, "ѕустой ввод вместо имени фигуры\n");
        return data;
    }
    // считывание след. строки с числом вершин
    int len_num;
    char* s_num = get_string(&len_num);
    // также, если длина = -1, то это полностью пуста€ строка
    if (len_num == -1) {
        free(s);
        free(s_num);
        fprintf(stderr, "EOF вместо числа вершин\n");
        return no_input;
    }
    // очистка от пробелов
    char* clean_s_num = trim_string(s_num, &len_num);
    // если после очистки от пробелов длина = 0, то получаетс€ вс€ строка была из пробелов
    if (len_num == 0) {
        free(s);
        free(s_num);
        fprintf(stderr, "ѕустой ввод вместо кол-ва вершин\n");
        return data;
    }
    // указатель на символ, на котором остановитс€ распознавание числа
    char* end_ptr;
    long int N = strtol(clean_s_num, &end_ptr, 10);
    // если этот указатель на что-то указывает (а пробелов к этому моменту уже нет)
    // то значит в строке есть недопустимые дл€ целого числа символы
    if (*end_ptr != 0) {
        fprintf(stderr, " ол-во вершин должно быть целым числом. ѕолучено: %s\n", clean_s_num);
        free(s);
        free(s_num);
        return data;
    }
    // число вершин не м.б. менее 1, провер€ем это
    if (N < 1) {
        free(s);
        free(s_num);
        fprintf(stderr, " ол-во вершин не может быть менее 1. ѕолучено: %ld\n", N);
        return usage;
    }
    // массив словоформ со склонением
    const char* words[] = {"вершина", "вершины", "вершин"};
    // форматный вывод со склонением
    printf("‘игура Ђ%sї: %ld %s.\n", clean_s, N, words[get_vertex_name(N)]);
    free(s);
    free(s_num);
    return 0;
}