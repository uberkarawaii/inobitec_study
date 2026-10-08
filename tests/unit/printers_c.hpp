#pragma once
#include <format>
#include <ostream>

#include "../../common/geometry.h"

// для gtest, "как напечатать значение типа Point"
// C-версия: тип struct Point из geometry.h (без operator== и без C++-членов),
// поэтому отдельный заголовок, а не printers.hpp: geometry.h и geometry.hpp
// в одном .cpp дадут redefinition struct Point
inline void PrintTo(const struct Point& p, std::ostream* os) {
    *os << std::format("({:.3f}, {:.3f}, {:.3f})", p.x, p.y, p.z);
}
