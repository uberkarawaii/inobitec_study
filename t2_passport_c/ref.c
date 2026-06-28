#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "..\common\string_utils.h"

// метод от deepseek для сравнения
const char* get_vertex_name(int N) {
    int r10 = N % 10;
    int r100 = N % 100;
    if (r100 >= 11 && r100 <= 14)
        return "вершин";

    static const char* kForms[] = {
        "вершин",  // 0
        "вершина", // 1
        "вершины", // 2
        "вершины", // 3
        "вершины", // 4
        "вершин",  // 5
        "вершин",  // 6
        "вершин",  // 7
        "вершин",  // 8
        "вершин"   // 9
    };
    return kForms[r10];
}

int main() {
    setlocale(LC_ALL, "Russian_Russia.1251");
    int len_name;
    char* name_ptr = get_string(&len_name);
    char* name = trim_string(name_ptr, &len_name);

    int len_num;
    char* ptr_N0 = get_string(&len_num);
    char* ptr_N1 = trim_string(ptr_N0, &len_num);
    char* end_ptr;
    int N = strtol(ptr_N1, &end_ptr, 10);

    const char* vertex_name = get_vertex_name(N);
    printf("Фигура «%s»: %ld %s.\n", name, N, vertex_name);

    free(name_ptr);
    free(ptr_N0);
    return 0;
}
