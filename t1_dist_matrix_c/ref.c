#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {

    char lineN[32];
    fgets(lineN, sizeof(lineN), stdin);
    char* endptr;
    long int N = strtol(lineN, &endptr, 10);

    for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
        int k = abs(i - j);
        double dist = (k == 0) ? 0.0 : 2.0 * sin(M_PI * k / N);
        printf("%8.3f", dist);
    }
    printf("\n");
    }   

    return 0;
}