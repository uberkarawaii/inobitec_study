#pragma once
namespace exit_code {
constexpr int usage = 64; // неверные аргументы / диапазон
constexpr int data = 65;  // не-число в данных
constexpr int no_in = 66; // пустой ввод
} // namespace exit_code