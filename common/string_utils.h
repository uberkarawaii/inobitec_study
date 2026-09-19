#ifndef IRINA_STRING_UTILS_H
#define IRINA_STRING_UTILS_H

#ifdef _WIN32

#ifdef COMMON_STATIC
#define COMMON_API

#elif defined(COMMON_EXPORTS)
#define COMMON_API __declspec(dllexport)

#else
#define COMMON_API __declspec(dllimport)
#endif

#else
#define COMMON_API
#endif

#include <stdint.h>

// массив символов из входного потока до \0 через динамич. массив
COMMON_API char* get_string(int* len);

// проверка на пустоту
COMMON_API int is_empty(const char* s);

// срез пробелов по бокам
COMMON_API char* trim_string(char* s, int* len);

// разбор целого (десятичное); строка уже обрезана от пробелов
// 0 - успех
// INT_PARSE_NOT_NUMBER  - не число
// INT_PARSE_OUT_OF_RANGE - не помещается в int32
COMMON_API int parse_int32(const char* s, int32_t* out);

// коды именно для целочисленного разбора, чтобы не сталкиваться с PARSE_* из geometry
enum { INT_PARSE_NOT_NUMBER = 1, INT_PARSE_OUT_OF_RANGE = 2 };

#endif
