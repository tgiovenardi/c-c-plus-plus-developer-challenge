cmake_minimum_required(VERSION 3.16)

project(
    hmi_calculator_challenge
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(
    calculator
    STATIC
    src/calculator.c
)

target_include_directories(
    calculator
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(
    hmi
    STATIC
    src/hmi.c
)

target_include_directories(
    hmi
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    hmi
    PRIVATE
    calculator
)

add_executable(
    hmi_calculator
    main.c
)

target_link_libraries(
    hmi_calculator
    PRIVATE
    hmi
)

include(CTest)

if(BUILD_TESTING)
    add_executable(
        calculator_tests
        tests/calculator_tests.c
    )

    target_link_libraries(
        calculator_tests
        PRIVATE
        calculator
    )

    add_test(
        NAME calculator_tests
        COMMAND calculator_tests
    )
endif()