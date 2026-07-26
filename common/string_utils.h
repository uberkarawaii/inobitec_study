#ifndef IRINA_STRING_UTILS_H
#define IRINA_STRING_UTILS_H

#ifdef COMMON_EXPORTS
#define COMMON_API __declspec(dllexport)
#else
#define COMMON_API __declspec(dllimport)
#endif

// массив символов из входного потока до \0 через динамич. массив
COMMON_API char* get_string(int* len);

// проверка на пустоту
COMMON_API int is_empty(const char* s);

// срез пробелов по бокам
COMMON_API char* trim_string(char* s, int* len);

#endif