#include <stddef.h>
#include <stdio.h>

#include "calculator.h"

#define TEST_TOLERANCE 1e-9

typedef int (*TestFunction)(void);

static double absolute_value(double value)
{
    return value < 0.0 ? -value : value;
}

static int values_are_equal(double first, double second)
{
    return absolute_value(first - second) <= TEST_TOLERANCE;
}

static int run_test(const char *name, TestFunction test)
{
    if (test())
    {
        printf("[PASS] %s\n", name);
        return 0;
    }

    printf("[FAIL] %s\n", name);
    return 1;
}

static int test_add_positive_values(void)
{
    double result = 0.0;

    const CalculatorStatus status =
        calculator_add(2.5, 3.5, &result);

    return status == CALCULATOR_OK &&
           values_are_equal(result, 6.0);
}

static int test_add_negative_values(void)
{
    double result = 0.0;

    const CalculatorStatus status =
        calculator_add(-4.0, 1.5, &result);

    return status == CALCULATOR_OK &&
           values_are_equal(result, -2.5);
}

static int test_add_null_result(void)
{
    return calculator_add(1.0, 2.0, NULL) ==
           CALCULATOR_ERROR_INVALID_ARGUMENT;
}

static int test_determinant_1x1(void)
{
    const double matrix[] =
    {
        7.5
    };

    double result = 0.0;

    const CalculatorStatus status =
        calculator_determinant(matrix, 1U, &result);

    return status == CALCULATOR_OK &&
           values_are_equal(result, 7.5);
}

static int test_determinant_2x2(void)
{
    const double matrix[] =
    {
        1.0, 2.0,
        3.0, 4.0
    };

    double result = 0.0;

    const CalculatorStatus status =
        calculator_determinant(matrix, 2U, &result);

    return status == CALCULATOR_OK &&
           values_are_equal(result, -2.0);
}

static int test_determinant_3x3(void)
{
    const double matrix[] =
    {
        6.0,  1.0, 1.0,
        4.0, -2.0, 5.0,
        2.0,  8.0, 7.0
    };

    double result = 0.0;

    const CalculatorStatus status =
        calculator_determinant(matrix, 3U, &result);

    return status == CALCULATOR_OK &&
           values_are_equal(result, -306.0);
}

static int test_determinant_singular_matrix(void)
{
    const double matrix[] =
    {
        1.0, 2.0,
        2.0, 4.0
    };

    double result = 0.0;

    const CalculatorStatus status =
        calculator_determinant(matrix, 2U, &result);

    return status == CALCULATOR_OK &&
           values_are_equal(result, 0.0);
}

static int test_determinant_row_swap(void)
{
    const double matrix[] =
    {
        0.0, 1.0,
        1.0, 0.0
    };

    double result = 0.0;

    const CalculatorStatus status =
        calculator_determinant(matrix, 2U, &result);

    return status == CALCULATOR_OK &&
           values_are_equal(result, -1.0);
}

static int test_determinant_dimension_zero(void)
{
    const double matrix[] =
    {
        1.0
    };

    double result = 0.0;

    return calculator_determinant(matrix, 0U, &result) ==
           CALCULATOR_ERROR_INVALID_DIMENSION;
}

static int test_determinant_dimension_too_large(void)
{
    const double matrix[] =
    {
        1.0
    };

    double result = 0.0;

    return calculator_determinant(
               matrix,
               CALCULATOR_MAX_MATRIX_DIMENSION + 1U,
               &result
           ) == CALCULATOR_ERROR_INVALID_DIMENSION;
}

static int test_determinant_null_matrix(void)
{
    double result = 0.0;

    return calculator_determinant(NULL, 1U, &result) ==
           CALCULATOR_ERROR_INVALID_ARGUMENT;
}

static int test_determinant_null_result(void)
{
    const double matrix[] =
    {
        1.0
    };

    return calculator_determinant(matrix, 1U, NULL) ==
           CALCULATOR_ERROR_INVALID_ARGUMENT;
}

static int test_determinant_preserves_input(void)
{
    double matrix[] =
    {
        6.0,  1.0, 1.0,
        4.0, -2.0, 5.0,
        2.0,  8.0, 7.0
    };

    const double original[] =
    {
        6.0,  1.0, 1.0,
        4.0, -2.0, 5.0,
        2.0,  8.0, 7.0
    };

    double result = 0.0;

    const CalculatorStatus status =
        calculator_determinant(matrix, 3U, &result);

    if (status != CALCULATOR_OK)
    {
        return 0;
    }

    for (size_t i = 0U; i < 9U; ++i)
    {
        if (!values_are_equal(matrix[i], original[i]))
        {
            return 0;
        }
    }

    return 1;
}

int main(void)
{
    int failures = 0;

    failures += run_test(
        "Addition with positive values",
        test_add_positive_values
    );

    failures += run_test(
        "Addition with negative values",
        test_add_negative_values
    );

    failures += run_test(
        "Addition with null result",
        test_add_null_result
    );

    failures += run_test(
        "Determinant 1x1",
        test_determinant_1x1
    );

    failures += run_test(
        "Determinant 2x2",
        test_determinant_2x2
    );

    failures += run_test(
        "Determinant 3x3",
        test_determinant_3x3
    );

    failures += run_test(
        "Singular matrix",
        test_determinant_singular_matrix
    );

    failures += run_test(
        "Row swap",
        test_determinant_row_swap
    );

    failures += run_test(
        "Dimension zero",
        test_determinant_dimension_zero
    );

    failures += run_test(
        "Dimension too large",
        test_determinant_dimension_too_large
    );

    failures += run_test(
        "Null matrix",
        test_determinant_null_matrix
    );

    failures += run_test(
        "Null determinant result",
        test_determinant_null_result
    );

    failures += run_test(
        "Determinant preserves input matrix",
        test_determinant_preserves_input
    );

    puts("");

    if (failures == 0)
    {
        puts("All calculator tests passed.");
        return 0;
    }

    printf("%d test(s) failed.\n", failures);
    return 1;
}