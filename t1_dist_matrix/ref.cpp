#include <cmath>
#include <iostream>
#include <numbers>
#include <print>
#include <vector>

int main() {
    int N;
    std::cin >> N;
    
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            int k = std::abs(i - j);
            double d = (k == 0) ? 0.0 : 2.0 * std::sin(std::numbers::pi * k / N);
            std::print("{:8.3f}", d);
        }
        std::println();
    }
}
