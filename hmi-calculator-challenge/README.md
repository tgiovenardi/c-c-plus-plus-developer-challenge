# Dynamox C/C++ Developer Challenge

This repository contains my implementation of the Dynamox C/C++ Developer Challenge.

The application was developed in C and provides a terminal-based Human-Machine Interface (HMI) for selecting and executing mathematical operations.

The implementation focuses on separation of responsibilities, predictable resource usage, input validation, testability, and maintainability.

---

# Candidate Implementation

## Overview

The application currently provides two mathematical operations:

1. Addition of two scalar values
2. Determinant calculation of a square matrix

The project is organized into separate modules for:

* mathematical operations;
* terminal interaction;
* persistent operation logging.

The calculator logic is independent from the HMI, allowing the mathematical functions to be tested without depending on terminal input and output.

---

# Requirement Coverage

## Functional Requirements

| Requirement                                                                     | Status      | How it was implemented                                                                       |
| ------------------------------------------------------------------------------- | ----------- | -------------------------------------------------------------------------------------------- |
| Create an HMI where users can choose an operation and execute it                | Implemented | A terminal-based HMI displays the available operations and executes the selected handler     |
| Users should be able to input single values and get the result                  | Implemented | The Addition operation receives two scalar `double` values and displays the resulting sum    |
| Users should be able to input array of values and get the result, if applicable | Implemented | The Determinant operation receives the matrix elements as an array stored in row-major order |
| Add at least two operations                                                     | Implemented | Addition and Determinant are available                                                       |
| Users must be able to select the operation type                                 | Implemented | The HMI displays the registered operations and receives the desired menu option              |
| One of the operations must calculate the Determinant                            | Implemented | `calculator_determinant()` calculates the determinant of a square matrix                     |

## Technical Requirements

| Requirement                           | Status      | How it was implemented                                                          |
| ------------------------------------- | ----------- | ------------------------------------------------------------------------------- |
| Use C, C++ or both                    | Implemented | The project is implemented in C11                                               |
| The code must have a `main.c` file    | Implemented | `main.c` is the application entry point and starts the HMI through `hmi_run()`  |
| Create libraries to organize the code | Implemented | CMake builds separate `calculator`, `hmi`, and `operation_log` static libraries |

## Bonus Requirements

| Bonus Requirement                                                          | Status      | How it was implemented                                                                                                                     |
| -------------------------------------------------------------------------- | ----------- | ------------------------------------------------------------------------------------------------------------------------------------------ |
| Add possibility to create new operation type                               | Implemented | Operations are registered in an `HmiOperation` table containing the menu option, display name, and handler function                        |
| Add persistent operation log                                               | Implemented | Successful operations are appended to `operation.log`, preserving previous entries between executions                                      |
| Enhance determinant calculation to allow users to select matrix dimensions | Implemented | The user selects a square matrix dimension between `1` and `CALCULATOR_MAX_MATRIX_DIMENSION`                                               |
| Detect and handle errors on operation input                                | Implemented | Menu options, integers, floating-point values, dimensions, invalid trailing characters, non-finite values, and API arguments are validated |

---

# Project Structure

```text
hmi-calculator-challenge/
├── CMakeLists.txt
├── main.c
├── README.md
├── include/
│   ├── calculator.h
│   ├── hmi.h
│   └── operation_log.h
├── src/
│   ├── calculator.c
│   ├── hmi.c
│   └── operation_log.c
└── tests/
    └── calculator_tests.c
```

## Modules

### `calculator`

Contains the mathematical operations and validates their API arguments.

Current operations:

* scalar addition;
* matrix determinant.

The module does not depend on terminal input/output.

### `hmi`

Responsible for interaction with the user.

The module:

* displays the menu;
* reads user input;
* validates input;
* selects the requested operation;
* collects operation parameters;
* calls the calculator module;
* displays results;
* requests persistent logging after successful calculations.

### `operation_log`

Responsible for persistent operation history.

Successful operations are appended to:

```text
operation.log
```

Example:

```text
ADDITION | first=5 | second=7 | result=12
DETERMINANT | dimension=2 | matrix=[1,2;3,4] | result=-2
```

The file is opened in append mode, so previous operations are preserved between executions.

A failure while writing the log does not invalidate a successfully calculated result. The HMI reports a warning and continues operating.

### `main.c`

The application entry point is intentionally minimal:

```c
#include "hmi.h"

int main(void)
{
    return hmi_run();
}
```

The application behavior is delegated to the HMI module.

---

# Extensible Operation Registration

Available HMI operations are described by:

```c
typedef struct
{
    long option;
    const char *name;
    HmiOperationHandler handler;
} HmiOperation;
```

The currently registered operations are:

```c
static const HmiOperation HMI_OPERATIONS[] =
{
    {1L, "Addition", run_addition},
    {2L, "Determinant", run_determinant}
};
```

The HMI uses this table both to display available operations and to find the appropriate operation handler.

Because the main HMI loop does not contain operation-specific dispatch logic, a new operation can be introduced by implementing its handler and adding a new entry to the operation table.

For example:

```c
static const HmiOperation HMI_OPERATIONS[] =
{
    {1L, "Addition", run_addition},
    {2L, "Determinant", run_determinant},
    {3L, "New operation", run_new_operation}
};
```

---

# Mathematical Implementation

## Addition

Addition receives two scalar `double` values:

```c
CalculatorStatus calculator_add(
    double a,
    double b,
    double *result
);
```

The calculated value is returned through the output pointer.

The function returns a `CalculatorStatus`, allowing invalid arguments to be reported without coupling the calculator module to the HMI.

---

## Determinant

The determinant function receives a square matrix stored as a one-dimensional array in row-major order:

```c
CalculatorStatus calculator_determinant(
    const double *matrix,
    size_t dimension,
    double *result
);
```

The implementation uses Gaussian elimination with partial pivoting.

The maximum supported matrix dimension is defined by:

```c
#define CALCULATOR_MAX_MATRIX_DIMENSION 8U
```

Therefore, the supported matrix dimensions currently range from `1x1` to `8x8`.

### Working Matrix

Gaussian elimination modifies matrix values during processing.

To preserve the matrix supplied by the caller, the input values are copied to an internal fixed-size working buffer before elimination begins.

### Partial Pivoting

For each pivot position, the implementation searches the remaining rows for the element with the greatest absolute value in the current column.

Selecting a larger pivot improves numerical behavior compared with always using the current diagonal value.

### Row Swaps

When another row is selected as the pivot row, the two rows are exchanged.

Because swapping two rows changes the sign of a determinant, the implementation tracks this change using a sign variable.

### Gaussian Elimination

Elements below each pivot are eliminated until the matrix reaches upper-triangular form.

Adding or subtracting a multiple of one row from another does not change the determinant.

For an upper-triangular matrix, the determinant corresponds to the product of the diagonal elements, corrected by the sign introduced by row swaps.

### Singular Matrices

If no non-zero pivot can be found for a column, the matrix is singular.

In this situation, the determinant is returned as zero.

---

# Input Validation

Terminal input is first read as text and only then converted to the expected numeric type.

Integer values are parsed using:

```c
strtol()
```

Floating-point values are parsed using:

```c
strtod()
```

The HMI handles invalid input cases including:

* invalid menu options;
* non-numeric values;
* numeric values followed by unexpected characters;
* integer conversion outside the supported range;
* floating-point conversion outside the supported range;
* non-finite floating-point values;
* matrix dimensions below the minimum;
* matrix dimensions above the configured maximum;
* input lines larger than the HMI input buffer.

Invalid user input does not terminate the application. The user is prompted to enter a valid value again.

---

# Persistent Operation Log

After a successful operation, the HMI sends the operation information to the logging module.

Addition logs contain:

* first input value;
* second input value;
* result.

Determinant logs contain:

* matrix dimension;
* matrix elements;
* result.

The file uses append mode:

```c
fopen(OPERATION_LOG_FILE_NAME, "a");
```

Therefore, the previous history remains available after the application is closed and started again.

The log file is created in the application's current working directory.

Logging is intentionally independent from calculation success. If the calculated result is valid but the log cannot be written, the result is still displayed and the HMI reports a warning.

---

# Building

## Requirements

The project requires:

* CMake 3.16 or newer;
* a C11-compatible compiler;
* a build system supported by CMake.

Compatible environments may include:

* GCC with MinGW;
* GCC on Linux;
* Clang;
* Microsoft Visual C++.

## Generic CMake Build

From the `hmi-calculator-challenge` directory:

```sh
cmake -S . -B build
cmake --build build
```

CMake selects a generator according to the available development environment.

## Windows with MinGW

When MinGW is available in `PATH`, it can be selected explicitly:

```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
```

If the `build` directory was previously generated with another CMake generator, remove the existing build directory first:

```powershell
Remove-Item -Recurse -Force build
```

Then configure and build the project again:

```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
```

Generated build artifacts are not intended to be committed to the repository.

---

# Running

After building the project, run the generated executable.

## Windows

```powershell
.\build\hmi_calculator.exe
```

## Linux/macOS

Depending on the selected generator:

```sh
./build/hmi_calculator
```

The application displays:

```text
Dynamox HMI Calculator

1 - Addition
2 - Determinant
0 - Exit
```

---

# Usage

## Addition

Select:

```text
1
```

The HMI requests:

```text
First value:
Second value:
```

The resulting sum is displayed and, when possible, persisted in `operation.log`.

## Determinant

Select:

```text
2
```

The application first requests the square matrix dimension:

```text
Matrix dimension (1-8):
```

It then requests each element individually.

For a `2x2` matrix, for example:

```text
Value [1][1]:
Value [1][2]:
Value [2][1]:
Value [2][2]:
```

The calculated determinant is displayed and persisted in the operation log.

## Exit

Select:

```text
0
```

to finish the application normally.

---

# Automated Tests

The mathematical calculator module has an automated test suite integrated with CTest.

Tests are implemented separately from the production `main.c`.

## Running the Test Suite

After configuring and building the project:

```sh
ctest --test-dir build --output-on-failure
```

On Windows, the test executable can also be executed directly:

```powershell
.\build\calculator_tests.exe
```

## Current Automated Test Coverage

The test suite currently validates:

* addition with positive values;
* addition with negative values;
* invalid addition output pointer;
* `1x1` determinant;
* `2x2` determinant;
* `3x3` determinant;
* singular matrix;
* determinant requiring a row swap;
* zero matrix dimension;
* dimension greater than the supported maximum;
* null matrix pointer;
* null determinant result pointer;
* preservation of the original input matrix.

The test executable returns a non-zero status when any test fails, allowing CTest to identify failures automatically.

---

# Manual Integration Validation

Because the HMI interacts directly with terminal `stdin` and `stdout`, its complete user flow is validated manually.

Recommended integration checks include:

* valid addition;
* valid determinant calculation;
* invalid menu option;
* invalid numeric input;
* invalid matrix dimension;
* repeated operations without restarting the application;
* persistent operation log between executions;
* normal application exit.

---

# Design Decisions

## Separation of Responsibilities

The project separates:

* mathematical calculation;
* terminal interaction;
* persistence.

This keeps each module focused on a single responsibility and reduces coupling between components.

## Explicit Status Codes

The calculator and logging APIs return explicit status values instead of printing internal errors directly.

This allows the caller to decide how errors should be presented.

## Fixed Maximum Matrix Dimension

The determinant implementation uses a fixed-size internal working buffer instead of dynamic memory allocation.

This provides:

* predictable memory consumption;
* no dynamic allocation failure path;
* simpler resource ownership;
* behavior suitable for a firmware-oriented implementation.

## Input Matrix Preservation

The determinant function does not modify the matrix provided by the caller.

Gaussian elimination is performed on an internal copy.

This behavior is also covered by the automated test suite.

## Extensible HMI Dispatch

The HMI uses operation descriptors and function pointers instead of a growing operation-specific `switch`.

This allows the list of available operations to be extended without changing the main HMI control flow.

## Non-Critical Logging

Persistence is treated as secondary to the actual calculation.

A failure to write `operation.log` produces a warning but does not convert a successfully completed calculation into a failure.

## C11

The project explicitly uses the C11 language standard.

CMake also disables compiler-specific C extensions:

```cmake
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
```

---

# Possible Future Improvements

Although the functional, technical, and bonus requirements implemented for this challenge are covered, possible future extensions include:

* additional mathematical operations;
* timestamps in persistent operation logs;
* automated HMI integration tests;
* configurable log destinations;
* support for additional persistence backends;
* more extensive numerical analysis for floating-point determinant calculations.

For an embedded target, the current logging interface could also be adapted to use flash memory or another non-volatile storage mechanism instead of a regular file.

---

# Validation Before Submission

A final validation can be performed with:

```sh
cmake --build build
ctest --test-dir build --output-on-failure
```

Then run:

```powershell
.\build\hmi_calculator.exe
```

and verify:

* Addition;
* Determinant;
* invalid input handling;
* repeated operations;
* persistent log creation;
* application exit.

---

# Summary

The implementation fulfills the requested functional and technical requirements and includes the proposed bonus features:

* extensible operation registration;
* persistent operation logging;
* user-selectable matrix dimensions;
* operation input error handling.

The architecture prioritizes readability, modularity, predictable resource usage, explicit error handling, and testability.
