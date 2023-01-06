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
include(${CMAKE_SOURCE_DIR}/cmake/FetchCMocka.cmake)

# Testing
enable_testing()

# Include subdirectories
add_subdirectory(tau-build)