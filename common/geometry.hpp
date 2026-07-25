#pragma once

#include <expected>
#include <string_view>

struct Point {
    double x;
    double y;
    double z;
};

// распознавание x y z
// возвращает либо Point, либо код ошибки
std::expected<Point, int> parse_point(std::string_view s);
