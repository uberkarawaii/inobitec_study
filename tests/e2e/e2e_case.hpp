#pragma once

#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace e2e {

enum class Stream { out, err };
enum class ExeKind { c, cpp };

// name     Ч ASCII, валидное им€ дл€ gtest-параметра
// input    Ч им€ файла в tests/input_data
// expected Ч байты CP1251 дл€ основного канала (primary)
// второй канал об€зан быть пустым
struct Case {
    std::string_view name;
    std::vector<std::string> args;
    std::string_view input;
    int expected_code;
    Stream primary;
    std::string_view expected;
};

// ƒл€ gtest: печатаем только ASCII-метки, без сырых CP1251-байт.
inline void PrintTo(ExeKind kind, std::ostream* os) { *os << (kind == ExeKind::c ? "c" : "cpp"); }

inline void PrintTo(const Case& c, std::ostream* os) { *os << c.name; }

} // namespace e2e