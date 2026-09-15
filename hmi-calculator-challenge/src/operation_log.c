#include "operation_log.h"

#include <stdio.h>

static OperationLogStatus close_log_file(FILE *file)
{
    if (fclose(file) != 0)
    {
        return OPERATION_LOG_ERROR_IO;
    }

    return OPERATION_LOG_OK;
}

OperationLogStatus operation_log_addition(
    double first_value,
    double second_value,
    double result)
{
    FILE *file = fopen(OPERATION_LOG_FILE_NAME, "a");

    if (file == NULL)
    {
        return OPERATION_LOG_ERROR_IO;
    }

    if (fprintf(
            file,
            "ADDITION | first=%.15g | second=%.15g | result=%.15g\n",
            first_value,
            second_value,
            result
        ) < 0)
    {
        (void)fclose(file);
        return OPERATION_LOG_ERROR_IO;
    }

    return close_log_file(file);
}

OperationLogStatus operation_log_determinant(
    const double *matrix,
    size_t dimension,
    double result)
{
    if (matrix == NULL || dimension == 0U)
    {
        return OPERATION_LOG_ERROR_INVALID_ARGUMENT;
    }

    FILE *file = fopen(OPERATION_LOG_FILE_NAME, "a");

    if (file == NULL)
    {
        return OPERATION_LOG_ERROR_IO;
    }

    if (fprintf(
            file,
            "DETERMINANT | dimension=%zu | matrix=[",
            dimension
        ) < 0)
    {
        (void)fclose(file);
        return OPERATION_LOG_ERROR_IO;
    }

    for (size_t row = 0U; row < dimension; ++row)
    {
        if (row > 0U && fputc(';', file) == EOF)
        {
            (void)fclose(file);
            return OPERATION_LOG_ERROR_IO;
        }

        for (size_t column = 0U; column < dimension; ++column)
        {
            if (column > 0U && fputc(',', file) == EOF)
            {
                (void)fclose(file);
                return OPERATION_LOG_ERROR_IO;
            }

            if (fprintf(
                    file,
                    "%.15g",
                    matrix[row * dimension + column]
                ) < 0)
            {
                (void)fclose(file);
                return OPERATION_LOG_ERROR_IO;
            }
        }
    }

    if (fprintf(file, "] | result=%.15g\n", result) < 0)
    {
        (void)fclose(file);
        return OPERATION_LOG_ERROR_IO;
    }

    return close_log_file(file);
}