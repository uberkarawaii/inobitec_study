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

#include "parse_codes.h"

// массив символов из входного потока до \0 через динамич. массив
COMMON_API char* get_string(int* len);

// проверка на пустоту
COMMON_API int is_empty(const char* s);

// срез пробелов по бокам
COMMON_API char* trim_string(char* s, int* len);

// пропускает ведущ. пробелы и смотрит по префиксу - это 16ричное число?
COMMON_API int is_hex_prefix(const char* ptr);

// разбор целого (десятичное); строка уже обрезана от пробелов
// 0 - успех
// NUMBER_NOT_NUMBER  - не число
// NUMBER_OUT_OF_RANGE - не помещается в int32
COMMON_API int parse_int32(const char* s, int32_t* out);

#endif
