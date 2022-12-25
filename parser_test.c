//
// Created on 12/21/22.
//
// clang-format off
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
// clang-format on

#include "parser.h"

#include <string.h>

#include "common.h"

static void assert_module_decl(struct tau_ast_compilation_unit *compilation_unit, const char *name) {
  assert_non_null(compilation_unit);
  assert_non_null(compilation_unit->module_decl);
  assert_non_null(compilation_unit->module_decl->name);
  assert_non_null(compilation_unit->module_decl->name->identifiers);

  struct tau_ast_identifier *identifier = tau_ptr_stack_get(compilation_unit->module_decl->name->identifiers, 0);
  assert_non_null(identifier);
  assert_int_equal(identifier->data.type, TAU_TOKEN_TYPE_IDENTIFIER);
  size_t name_len = strlen(name);
  assert_memory_equal(name, identifier->data.buf, name_len);
}

static void test_parse_module(void **state) {
  UNUSED(state);

  const char *buf_name = __func__;
  const char *buf_data = "module example;";
  size_t buf_size = strlen(buf_data);

  struct tau_parser *parser = tau_parser_new(buf_name, buf_data, buf_size);
  parser->strict_mode = false;

  struct tau_ast_compilation_unit *compilation_unit = tau_parse_compilation_unit(parser);
  assert_module_decl(compilation_unit, "example");

  tau_ast_compilation_unit_free(compilation_unit);
  tau_parser_free(parser);
}

int main() {
  UNUSED_TYPE(jmp_buf);
  UNUSED_TYPE(va_list);

  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_parse_module),  // single module declaration
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}