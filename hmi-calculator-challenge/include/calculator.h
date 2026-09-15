#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <stddef.h>

#define CALCULATOR_MAX_MATRIX_DIMENSION 8U

typedef enum
{
    CALCULATOR_OK = 0,
    CALCULATOR_ERROR_INVALID_ARGUMENT,
    CALCULATOR_ERROR_INVALID_DIMENSION
} CalculatorStatus;

typedef enum
{
    CALCULATOR_OPERATION_ADD = 0,
    CALCULATOR_OPERATION_DETERMINANT
} CalculatorOperation;

/*
 * Adds two scalar values.
 *
 * result must point to a valid double where the calculated value
 * will be stored.
 */
CalculatorStatus calculator_add(double a, double b, double *result);

/*
 * Calculates the determinant of a square matrix.
 *
 * matrix must contain dimension * dimension elements in row-major order.
 * dimension must be between 1 and CALCULATOR_MAX_MATRIX_DIMENSION.
 * result must point to a valid double where the determinant will be stored.
 */
CalculatorStatus calculator_determinant(const double *matrix, size_t dimension, double *result);

#endif