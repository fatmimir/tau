# Macros
macro(setup_test TARGET TEST_SOURCES)
    add_executable(${TARGET} ${ARGN} test/${TARGET}.c)
    target_link_libraries(${TARGET} PRIVATE cmocka-static)
    add_test(NAME ${TARGET} COMMAND ${TARGET})
endmacro()
