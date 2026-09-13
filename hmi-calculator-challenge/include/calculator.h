#ifndef CALCULATOR_H
#define CALCULATOR_H

typedef enum
{
    CALCULATOR_OK = 0,
    CALCULATOR_ERROR_INVALID_ARGUMENT
} CalculatorStatus;

typedef enum
{
    CALCULATOR_OPERATION_ADD = 0,
    CALCULATOR_OPERATION_DETERMINANT
} CalculatorOperation;

CalculatorStatus calculator_add(double a, double b, double *result);

#endif