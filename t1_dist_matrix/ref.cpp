#include <cmath>
#include <iostream>
#include <numbers>
#include <print>
#include <vector>

int main() {
    int N;
    std::cin >> N;

    std::vector<double> xs(N), ys(N);
    for (int i = 0; i < N; ++i) {
        double angle = 2.0 * std::numbers::pi * i / N;
        xs[i] = std::cos(angle);
        ys[i] = std::sin(angle);
    }

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            double d = (i == j) ? 0.0 : std::hypot(xs[i] - xs[j], ys[i] - ys[j]);
            std::print("{:8.3f}", d);
        }
        std::println();
    }
}
