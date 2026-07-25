#include "string_utils.hpp"

#include <string>

// убрать пробелы слева и справа
void trim_str(std::string& s) {
    // первый слева не пробельный или служебный символ
    auto left = s.find_first_not_of(" \n\t\r");
    // если он указывает на конец строки
    if (left == std::string::npos) {
        s.clear();
        return;
    }
    s.erase(0, left);
    auto right = s.find_last_not_of(" \n\t\r");
    // если справа не было пробелов, то обрежется часть с индексом s.len(),
    // а её не существует т.к. это за строкой. т.е. ничего не обрежется
    s.erase(right + 1);
}

int is_empty(const std::string& s) {
    // первый слева не пробельный или служебный символ
    auto left = s.find_first_not_of(" \n\t\r");
    // указывает ли он на конец строки
    return left == std::string::npos;
}