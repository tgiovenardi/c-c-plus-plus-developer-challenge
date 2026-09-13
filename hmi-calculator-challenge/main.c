#include <stdio.h>

#include "calculator.h"

int main(void)
{
    double result = 0.0;

    CalculatorStatus status = calculator_add(10, -2, &result);

    if (status != CALCULATOR_OK)
    {
        puts("Calculation failed");
        return 1;
    }

    printf("Result: %.2f\n", result);

    return 0;
}