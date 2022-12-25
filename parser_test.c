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

static void assert_module_decl(struct tau_ast_compilation_unit *compilation_unit, ...) {
  assert_non_null(compilation_unit);
  assert_non_null(compilation_unit->module_decl);
  assert_non_null(compilation_unit->module_decl->name);
  assert_non_null(compilation_unit->module_decl->name->identifiers);

  va_list args;
  va_start(args, compilation_unit);
  for (int32_t i = 0; i < compilation_unit->module_decl->name->identifiers->cap; i++) {
    const char *name = va_arg(args, const char *);
    if (name == NULL) {
      break;
    }

    struct tau_ast_identifier *identifier = tau_ptr_stack_get(compilation_unit->module_decl->name->identifiers, i);
    assert_non_null(identifier);
    assert_int_equal(identifier->data.type, TAU_TOKEN_TYPE_IDENTIFIER);
    size_t name_len = strlen(name);
    assert_memory_equal(name, identifier->data.buf, name_len);
  }
  va_end(args);
}

static void test_parse_module_plain_name(void **state) {
  UNUSED(state);

  const char *buf_name = __func__;
  const char *buf_data = "module example;";
  size_t buf_size = strlen(buf_data);

  struct tau_parser *parser = tau_parser_new(buf_name, buf_data, buf_size);
  parser->strict_mode = false;

  struct tau_ast_compilation_unit *compilation_unit = tau_parse_compilation_unit(parser);
  assert_module_decl(compilation_unit, "example", NULL);

  tau_ast_compilation_unit_free(compilation_unit);
  tau_parser_free(parser);
}

static void test_parse_module_with_lookup(void **state) {
  UNUSED(state);

  const char *buf_name = __func__;
  const char *buf_data = "module example::submodule;";
  size_t buf_size = strlen(buf_data);

  struct tau_parser *parser = tau_parser_new(buf_name, buf_data, buf_size);
  parser->strict_mode = false;

  struct tau_ast_compilation_unit *compilation_unit = tau_parse_compilation_unit(parser);
  assert_module_decl(compilation_unit, "example", "submodule", NULL);

  tau_ast_compilation_unit_free(compilation_unit);
  tau_parser_free(parser);
}

static void test_parse_module_invalid_syntax(void **state) {
  UNUSED(state);

  const char *buf_name = __func__;
  const char *buf_data = "module ::";
  size_t buf_size = strlen(buf_data);

  struct tau_parser *parser = tau_parser_new(buf_name, buf_data, buf_size);
  parser->strict_mode = false;

  struct tau_ast_compilation_unit *compilation_unit = tau_parse_compilation_unit(parser);
  assert_null(compilation_unit);

  tau_parser_free(parser);
}

int main() {
  UNUSED_TYPE(jmp_buf);
  UNUSED_TYPE(va_list);

  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_parse_module_plain_name),      // module with a single identifier as name
      cmocka_unit_test(test_parse_module_with_lookup),     // module with static lookup
      cmocka_unit_test(test_parse_module_invalid_syntax),  // module with invalid syntax
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}