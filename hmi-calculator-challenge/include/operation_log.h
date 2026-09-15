#ifndef OPERATION_LOG_H
#define OPERATION_LOG_H

#include <stddef.h>

#define OPERATION_LOG_FILE_NAME "operation.log"

typedef enum
{
    OPERATION_LOG_OK = 0,
    OPERATION_LOG_ERROR_INVALID_ARGUMENT = 1,
    OPERATION_LOG_ERROR_IO = 2
} OperationLogStatus;

/*
 * Appends an addition operation to the persistent operation log.
 */
OperationLogStatus operation_log_addition(
    double first_value,
    double second_value,
    double result
);

/*
 * Appends a determinant operation to the persistent operation log.
 *
 * matrix must contain dimension * dimension elements in row-major order.
 */
OperationLogStatus operation_log_determinant(
    const double *matrix,
    size_t dimension,
    double result
);

#endif