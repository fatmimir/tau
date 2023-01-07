include(${CMAKE_SOURCE_DIR}/cmake/TestMacros.cmake)

set(HEADERS src/common.h src/lexer.h src/log.h src/utf8.h src/uc_names.h src/parser_match.h src/parser_internal.h)
set(SOURCES src/lexer.c src/log.c src/utf8.c src/parser_match.c src/parser_internal.c)

setup_test(utf8_test ${HEADERS} ${SOURCES})
setup_test(lexer_test ${HEADERS} ${SOURCES})
setup_test(parser_expr_test ${HEADERS} ${SOURCES})
setup_test(parser_stmt_test ${HEADERS} ${SOURCES})
setup_test(parser_decl_test ${HEADERS} ${SOURCES})

add_library(tau-parser STATIC ${HEADERS} ${SOURCES} src/parser.c)
target_include_directories(tau-parser PUBLIC include)