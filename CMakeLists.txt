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
setup_test(utf8_test common.h utf8.h utf8.c)
setup_test(ptr_stack_test common.h ptr_stack.h ptr_stack.c)
setup_test(lexer_test common.h utf8.h utf8.c uc_names.h log.h log.c lexer.h lexer.c)
setup_test(parser_expr_test common.h utf8.h utf8.c uc_names.h log.h log.c ptr_stack.h ptr_stack.c
        lexer.h lexer.c parser.h parser_match.h parser_match.c parser_alloc.h parser_internals.h parser.c parser_test_utils.h)
setup_test(parser_stmt_decl_test common.h utf8.h utf8.c uc_names.h log.h log.c ptr_stack.h ptr_stack.c
        lexer.h lexer.c parser.h parser_match.h parser_match.c parser_alloc.h parser_internals.h parser.c parser_test_utils.h)
