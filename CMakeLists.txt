cmake_minimum_required(VERSION 3.16)
project(tau
        VERSION 0.0.1
        LANGUAGES C
        DESCRIPTION "Tau programming language")

enable_language(C)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include scripts
include(cmake/FetchCMocka.cmake)

# Macros
macro(setup_test TARGET TEST_SOURCES)
    add_executable(${TARGET} ${ARGN} ${TARGET}.c)
    target_link_libraries(${TARGET} PRIVATE cmocka-static)
    add_test(NAME ${TARGET} COMMAND ${TARGET})
endmacro()

# Testing
enable_testing()
setup_test(test_dummy test_dummy.c)
