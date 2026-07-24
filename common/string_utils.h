#ifndef IRINA_STRING_UTILS_H
#define IRINA_STRING_UTILS_H

// массив символов из входного потока до \0 через динамич. массив
char* get_string(int* len);

// проверка на пустоту
int is_empty(const char* s);

// срез пробелов по бокам
char* trim_string(char* s, int* len);

#endif