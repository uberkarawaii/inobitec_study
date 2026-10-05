#pragma once
#include <format>
#include <ostream>

#include "../../common/geometry.hpp"

// для gtest, "как печатать объект типа Point"
// глобально т.к. с Point связан глобальный namespace - adl будет искать именно в нём. в приватном не найдёт
// пример с adl (argument-dependent lookup):
// если есть namespace K{func f(s); type m}; K:m perem; f(perem) - f будет искаться не только в glob, но и в K
// потому что аргумент f имеет тип из этого namespace
inline void PrintTo(const Point& p, std::ostream* os) { *os << std::format("({:.3f}, {:.3f}, {:.3f})", p.x, p.y, p.z); }