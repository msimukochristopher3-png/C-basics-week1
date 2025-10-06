#include <stdio.h>

float findAverage(float a, float b, float c) {
    return (a + b + c) / 3;
}

int main() {
    float num1, num2, num3, average;

    printf("Enter three numbers: ");
    scanf("%f %f %f", &num1, &num2, &num3);

    average = findAverage(num1, num2, num3);

    printf("Average = %.2f\n", average);

    return 0;
}
