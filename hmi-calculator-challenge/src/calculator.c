#include "calculator.h"

#include <stddef.h>

CalculatorStatus calculator_add(double a, double b, double *result)

{
    if (result == NULL)
    {
        return CALCULATOR_ERROR_INVALID_ARGUMENT;
    }

    *result = a + b;

    return CALCULATOR_OK;
}