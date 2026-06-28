#ifndef IRINA_STRING_UTILS_H
#define IRINA_STRING_UTILS_H

// static - локальная копия для файлов, где будет ф-ция
// inline - прописать функцию в файле
static inline char* get_string(int* len) {
    *len = 0;
    int capacity = 1;
    char *s = (char*)malloc(sizeof(char)), *temp = NULL;
    int ch;
    while ((ch = getchar()) != EOF && ch != '\n') {
        s[*len] = (char)ch;
        ++(*len);

        if (*len >= capacity) {
            capacity *= 2;
            temp = (char*)realloc(s, capacity * sizeof(char));
            // проверка на null от realloc по совету deepseek
            // + free блока, чтобы он не затёрся при возврате null и не было утечки
            if (!temp) {
                free(s);
                *len = -1;
                return NULL;
            }
            s = temp;
        }
    }
    s[*len] = '\0';
    if (ch == EOF)
        *len = -1;

    return s;
}

// срез пробелов по бокам
static inline char* trim_string(char* s, int* len) {
    // срезать пробелы в начале
    int i = 0;
    // пока i не за границей и текущий символ - пробельный
    while (i < *len && isspace((unsigned char)s[i])) {
        // если след. не пробельный или конец строки - можно сдвинуть начало строки и изменить длину
        if (!isspace((unsigned char)s[i + 1]) || s[i + 1] == '\0') {
            s = s + i + 1;
            *len = *len - i - 1;
            break;
        }
        ++i;
    }

    // строка пустая, можно её уже вернуть
    if (*len == 0)
        return s;

    // и пробелы с конца
    i = *len - 1;
    while (i > 0 && isspace((unsigned char)s[i])) {
        if (!isspace((unsigned char)s[i - 1])) {
            s[i] = '\0';
            *len = *len - (*len - i);
            break;
        }
        --i;
    }

    return s;
}

#endif