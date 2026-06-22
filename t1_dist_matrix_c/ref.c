#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE 20
#define MIN_SIZE 3
#define IRINA_PI 3.14159265358979323846

int main(void) {

    char lineN[32];
    fgets(lineN, sizeof(lineN), stdin);
    char* endptr;
    long int N = strtol(lineN, &endptr, 10);

    // вершины. Х и У это косинусы и синусы от углов
    double x[MAX_SIZE], y[MAX_SIZE];
    for (int i = 0; i < N; ++i) {
        x[i] = cos(i * 2 * IRINA_PI / N);
        y[i] = sin(i * 2 * IRINA_PI / N);
    }

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            if (i == j)
                printf("%8.3f ", 0.0);
            else
                printf("%8.3f ", sqrt(pow(x[i] - x[j], 2) + pow(y[i] - y[j], 2)));
        }
        printf("\n");
    }

    return 0;
}