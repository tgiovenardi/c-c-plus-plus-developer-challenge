#include "hmi.h"

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "calculator.h"
#include "operation_log.h"

#define HMI_INPUT_BUFFER_SIZE 64U
#define HMI_MENU_OPTION_EXIT 0L

typedef enum
{
    HMI_INPUT_OK = 0,
    HMI_INPUT_INVALID = 1,
    HMI_INPUT_END = 2
} HmiInputStatus;

typedef HmiInputStatus (*HmiOperationHandler)(void);

typedef struct
{
    long option;
    const char *name;
    HmiOperationHandler handler;
} HmiOperation;

/*
 * Operation handlers are declared before the operation table so they can
 * be registered without exposing them outside this source file.
 */
static HmiInputStatus run_addition(void);
static HmiInputStatus run_determinant(void);

/*
 * Available operations are registered in this table.
 *
 * To add a new operation, implement its handler and register a new entry
 * here. The menu and operation dispatch do not need additional changes.
 */
static const HmiOperation HMI_OPERATIONS[] =
{
    {1L, "Addition", run_addition},
    {2L, "Determinant", run_determinant}
};

#define HMI_OPERATION_COUNT \
    (sizeof(HMI_OPERATIONS) / sizeof(HMI_OPERATIONS[0]))

static void print_menu(void)
{
    puts("");
    puts("Dynamox HMI Calculator");
    puts("");

    for (size_t i = 0U; i < HMI_OPERATION_COUNT; ++i)
    {
        printf(
            "%ld - %s\n",
            HMI_OPERATIONS[i].option,
            HMI_OPERATIONS[i].name
        );
    }

    printf("%ld - Exit\n", HMI_MENU_OPTION_EXIT);
    puts("");
}

static const HmiOperation *find_operation(long option)
{
    for (size_t i = 0U; i < HMI_OPERATION_COUNT; ++i)
    {
        if (HMI_OPERATIONS[i].option == option)
        {
            return &HMI_OPERATIONS[i];
        }
    }

    return NULL;
}

static HmiInputStatus read_line(
    const char *prompt,
    char *buffer,
    size_t buffer_size)
{
    if (buffer == NULL || buffer_size == 0U)
    {
        return HMI_INPUT_INVALID;
    }

    fputs(prompt, stdout);
    fflush(stdout);

    if (fgets(buffer, buffer_size, stdin) == NULL)
    {
        return HMI_INPUT_END;
    }

    /*
     * If the newline does not fit in the buffer, discard the remaining
     * characters so they do not affect the next input operation.
     */
    if (strchr(buffer, '\n') == NULL && !feof(stdin))
    {
        int character;

        do
        {
            character = getchar();
        } while (character != '\n' && character != EOF);

        return HMI_INPUT_INVALID;
    }

    return HMI_INPUT_OK;
}

static HmiInputStatus read_integer(const char *prompt, long *value)
{
    char buffer[HMI_INPUT_BUFFER_SIZE];
    char *end = NULL;

    if (value == NULL)
    {
        return HMI_INPUT_INVALID;
    }

    const HmiInputStatus input_status =
        read_line(prompt, buffer, sizeof(buffer));

    if (input_status != HMI_INPUT_OK)
    {
        return input_status;
    }

    errno = 0;
    const long parsed_value = strtol(buffer, &end, 10);

    /*
     * strtol must consume at least one digit and the converted value
     * must fit in the range supported by long.
     */
    if (end == buffer || errno == ERANGE)
    {
        return HMI_INPUT_INVALID;
    }

    /*
     * After the number, only whitespace is accepted.
     * This rejects inputs such as "1abc".
     */
    while (*end != '\0')
    {
        if (!isspace((unsigned char)*end))
        {
            return HMI_INPUT_INVALID;
        }

        ++end;
    }

    *value = parsed_value;

    return HMI_INPUT_OK;
}

static HmiInputStatus read_double(const char *prompt, double *value)
{
    char buffer[HMI_INPUT_BUFFER_SIZE];
    char *end = NULL;

    if (value == NULL)
    {
        return HMI_INPUT_INVALID;
    }

    const HmiInputStatus input_status =
        read_line(prompt, buffer, sizeof(buffer));

    if (input_status != HMI_INPUT_OK)
    {
        return input_status;
    }

    errno = 0;
    const double parsed_value = strtod(buffer, &end);

    /*
     * The input must contain a valid finite floating-point value.
     * Values outside the representable range are rejected.
     */
    if (end == buffer || errno == ERANGE || !isfinite(parsed_value))
    {
        return HMI_INPUT_INVALID;
    }

    /*
     * After the number, only whitespace is accepted.
     * This rejects inputs such as "10.5abc".
     */
    while (*end != '\0')
    {
        if (!isspace((unsigned char)*end))
        {
            return HMI_INPUT_INVALID;
        }

        ++end;
    }

    *value = parsed_value;

    return HMI_INPUT_OK;
}

static HmiInputStatus read_valid_double(
    const char *prompt,
    double *value)
{
    for (;;)
    {
        const HmiInputStatus input_status =
            read_double(prompt, value);

        if (input_status == HMI_INPUT_END)
        {
            return HMI_INPUT_END;
        }

        if (input_status == HMI_INPUT_OK)
        {
            return HMI_INPUT_OK;
        }

        puts("Invalid number. Try again.");
    }
}

static HmiInputStatus read_matrix_dimension(size_t *dimension)
{
    char prompt[48];
    long parsed_dimension = 0;

    if (dimension == NULL)
    {
        return HMI_INPUT_INVALID;
    }

    (void)snprintf(
        prompt,
        sizeof(prompt),
        "Matrix dimension (1-%u): ",
        (unsigned int)CALCULATOR_MAX_MATRIX_DIMENSION
    );

    for (;;)
    {
        const HmiInputStatus input_status =
            read_integer(prompt, &parsed_dimension);

        if (input_status == HMI_INPUT_END)
        {
            return HMI_INPUT_END;
        }

        if (input_status != HMI_INPUT_OK)
        {
            printf(
                "Invalid dimension. Enter an integer from 1 to %u.\n",
                (unsigned int)CALCULATOR_MAX_MATRIX_DIMENSION
            );
            continue;
        }

        if (parsed_dimension < 1L ||
            parsed_dimension > (long)CALCULATOR_MAX_MATRIX_DIMENSION)
        {
            printf(
                "Invalid dimension. Enter a value from 1 to %u.\n",
                (unsigned int)CALCULATOR_MAX_MATRIX_DIMENSION
            );
            continue;
        }

        *dimension = (size_t)parsed_dimension;

        return HMI_INPUT_OK;
    }
}

static HmiInputStatus run_addition(void)
{
    double first_value = 0.0;
    double second_value = 0.0;
    double result = 0.0;

    if (read_valid_double("First value: ", &first_value) == HMI_INPUT_END)
    {
        return HMI_INPUT_END;
    }

    if (read_valid_double("Second value: ", &second_value) == HMI_INPUT_END)
    {
        return HMI_INPUT_END;
    }

    const CalculatorStatus calculator_status =
        calculator_add(first_value, second_value, &result);

    if (calculator_status != CALCULATOR_OK)
    {
        puts("Calculation failed.");
        return HMI_INPUT_OK;
    }

    printf("Result: %.15g\n", result);

    /*
     * Logging is intentionally independent from the calculation.
     * A logging failure must not prevent the calculator from working.
     */
    const OperationLogStatus log_status =
        operation_log_addition(
            first_value,
            second_value,
            result
        );

    if (log_status != OPERATION_LOG_OK)
    {
        puts("Warning: operation could not be written to log.");
    }

    return HMI_INPUT_OK;
}

static HmiInputStatus run_determinant(void)
{
    double matrix[
        CALCULATOR_MAX_MATRIX_DIMENSION *
        CALCULATOR_MAX_MATRIX_DIMENSION
    ];

    size_t dimension = 0U;
    double result = 0.0;

    HmiInputStatus input_status =
        read_matrix_dimension(&dimension);

    if (input_status == HMI_INPUT_END)
    {
        return HMI_INPUT_END;
    }

    for (size_t row = 0U; row < dimension; ++row)
    {
        for (size_t column = 0U; column < dimension; ++column)
        {
            char prompt[32];

            (void)snprintf(
                prompt,
                sizeof(prompt),
                "Value [%zu][%zu]: ",
                row + 1U,
                column + 1U
            );

            input_status =
                read_valid_double(
                    prompt,
                    &matrix[row * dimension + column]
                );

            if (input_status == HMI_INPUT_END)
            {
                return HMI_INPUT_END;
            }
        }
    }

    const CalculatorStatus calculator_status =
        calculator_determinant(matrix, dimension, &result);

    if (calculator_status != CALCULATOR_OK)
    {
        puts("Calculation failed.");
        return HMI_INPUT_OK;
    }

    printf("Determinant: %.15g\n", result);

    /*
     * Store the successful operation in the persistent log.
     * Logging failure is reported but does not invalidate the calculation.
     */
    const OperationLogStatus log_status =
        operation_log_determinant(
            matrix,
            dimension,
            result
        );

    if (log_status != OPERATION_LOG_OK)
    {
        puts("Warning: operation could not be written to log.");
    }

    return HMI_INPUT_OK;
}

int hmi_run(void)
{
    for (;;)
    {
        long option = 0;

        print_menu();

        const HmiInputStatus input_status =
            read_integer("Select an operation: ", &option);

        if (input_status == HMI_INPUT_END)
        {
            puts("");
            puts("Input ended.");
            return 0;
        }

        if (input_status != HMI_INPUT_OK)
        {
            puts("Invalid input. Enter a valid menu option.");
            continue;
        }

        if (option == HMI_MENU_OPTION_EXIT)
        {
            puts("Exiting calculator.");
            return 0;
        }

        const HmiOperation *operation =
            find_operation(option);

        if (operation == NULL)
        {
            puts("Invalid operation. Select one of the available options.");
            continue;
        }

        if (operation->handler() == HMI_INPUT_END)
        {
            puts("");
            puts("Input ended.");
            return 0;
        }
    }
}