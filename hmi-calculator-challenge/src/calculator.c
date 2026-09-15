#include "calculator.h"

#include <stddef.h>

static double absolute_value(double value)
{
    return value < 0.0 ? -value : value;
}

CalculatorStatus calculator_add(double a, double b, double *result)
{
    if (result == NULL)
    {
        return CALCULATOR_ERROR_INVALID_ARGUMENT;
    }

    *result = a + b;

    return CALCULATOR_OK;
}

CalculatorStatus calculator_determinant(const double *matrix, size_t dimension, double *result)
{
    double work[CALCULATOR_MAX_MATRIX_DIMENSION * CALCULATOR_MAX_MATRIX_DIMENSION];

    /*
     * The determinant will be accumulated from the pivots of the
     * upper-triangular matrix produced by Gaussian elimination.
     *
     * sign tracks row swaps because swapping two rows changes
     * the sign of the determinant.
     */
    double determinant = 1.0;
    int sign = 1;

    /* The input matrix and output location must both be valid. */
    if (matrix == NULL || result == NULL)
    {
        return CALCULATOR_ERROR_INVALID_ARGUMENT;
    }

    /*
     * The matrix must have a valid square dimension supported by
     * the fixed-size working buffer.
     */
    if (dimension == 0U || dimension > CALCULATOR_MAX_MATRIX_DIMENSION)
    {
        return CALCULATOR_ERROR_INVALID_DIMENSION;
    }

    /*
     * Gaussian elimination modifies matrix elements.
     * Copy the input so the caller's matrix remains unchanged.
     */
    const size_t element_count = dimension * dimension;

    for (size_t i = 0U; i < element_count; ++i)
    {
        work[i] = matrix[i];
    }

    /*
     * Process one pivot column at a time.
     *
     * The goal is to transform the matrix into upper-triangular form
     *
     * Once triangular, the determinant is the product of the diagonal
     * elements, corrected for any row swaps.
     */
    for (size_t pivot_index = 0U; pivot_index < dimension; ++pivot_index)
    {
        /*
         * Partial pivoting:
         * search the current column for the element with the greatest
         * absolute value, starting at the current pivot row.
         *
         * Using a larger pivot reduces numerical problems caused by
         * division by very small floating-point values.
         */
        size_t pivot_row = pivot_index;
        double pivot_abs = absolute_value(work[pivot_index * dimension + pivot_index]);

        for (size_t row = pivot_index + 1U; row < dimension; ++row)
        {
            const double candidate_abs = absolute_value(work[row * dimension + pivot_index]);

            if (candidate_abs > pivot_abs)
            {
                pivot_abs = candidate_abs;
                pivot_row = row;
            }
        }

        /*
         * No non-zero pivot exists in this column.
         * The matrix is singular, so its determinant is zero.
         */
        if (pivot_abs == 0.0)
        {
            *result = 0.0;
            return CALCULATOR_OK;
        }

        /*
         * Move the selected pivot into the diagonal position.
         *
         * Mathematical property:
         * swapping two rows multiplies the determinant by -1.
         */
        if (pivot_row != pivot_index)
        {
            for (size_t column = 0U; column < dimension; ++column)
            {
                const size_t current_index = pivot_index * dimension + column;
                const size_t swap_index = pivot_row * dimension + column;
                const double temporary = work[current_index];

                work[current_index] = work[swap_index];
                work[swap_index] = temporary;
            }

            sign = -sign;
        }

        /*
         * The pivot is now on the main diagonal.
         * Each pivot contributes to the determinant of the final
         * upper-triangular matrix.
         */
        const double pivot = work[pivot_index * dimension + pivot_index];

        determinant *= pivot;

        /*
         * Eliminate every element below the pivot.
         *
         * For each row:
         *
         *     factor = current_element / pivot
         *
         * and then:
         *
         *     current_row = current_row - factor * pivot_row
         *
         * Adding/subtracting a multiple of one row from another
         * does not change the determinant.
         */
        for (size_t row = pivot_index + 1U; row < dimension; ++row)
        {
            const double factor = work[row * dimension + pivot_index] / pivot;

            /*
             * The elimination operation mathematically makes this
             * element zero. Assign it directly to avoid keeping small
             * floating-point residue in the lower triangle.
             */
            work[row * dimension + pivot_index] = 0.0;

            /*
             * Update only the columns to the right of the pivot.
             * Previous columns have already been eliminated.
             */
            for (size_t column = pivot_index + 1U; column < dimension; ++column)
            {
                work[row * dimension + column] -= factor * work[pivot_index * dimension + column];
            }
        }
    }

    /*
     * For an upper-triangular matrix, the determinant is the product
     * of the diagonal elements.
     */
    *result = determinant * sign;

    return CALCULATOR_OK;
}