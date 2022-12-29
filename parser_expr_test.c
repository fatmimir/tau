//
// Created on 12/21/22.
//
// clang-format off
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
// clang-format on

#include <string.h>

#include "common.h"
#include "lexer.h"
#include "parser.h"
#include "parser_internals.h"
#include "parser_test_utils.h"

static void test_parse_identifier_ok(void **state) {
  UNUSED(state);
  const char *test_data = "id123";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *identifier = parse_identifier(&token);
  assert_topology(identifier, "id123");
  tau_anode_free(identifier);
}

static void test_parse_identifier_bad(void **state) {
  UNUSED(state);
  const char *test_data = "10";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *identifier = parse_identifier(&token);
  assert_null(identifier);
}

static void test_parse_atom_simple_ok(void **state) {
  UNUSED(state);
  const char *test_data = "id 123 \"hello\" true";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *atom = NULL;

  atom = parse_atom_expr(&token);
  assert_topology(atom, "id");
  tau_ptr_stack_free(atom);

  atom = parse_atom_expr(&token);
  assert_topology(atom, "123");
  tau_ptr_stack_free(atom);

  atom = parse_atom_expr(&token);
  assert_topology(atom, "\"hello\"");
  tau_ptr_stack_free(atom);

  atom = parse_atom_expr(&token);
  assert_topology(atom, "true");
  tau_ptr_stack_free(atom);
}

static void test_parse_atom_simple_bad(void **state) {
  UNUSED(state);
  const char *test_data = "+";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *atom = parse_atom_expr(&token);
  assert_null(atom);
}

static void test_parse_type_lookup_ok(void **state) {
  UNUSED(state);
  const char *test_data = "std::type_lookup strange::0::but::valid";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *expr = NULL;

  expr = parse_type_lookup_expr(&token);
  assert_topology(expr, "(TYPE_LOOKUP_EXPR std type_lookup)");
  tau_anode_free(expr);

  expr = parse_type_lookup_expr(&token);
  assert_topology(expr, "(TYPE_LOOKUP_EXPR (TYPE_LOOKUP_EXPR (TYPE_LOOKUP_EXPR strange 0) but) valid)");
  tau_anode_free(expr);
}

static void test_parse_type_lookup_bad(void **state) {
  UNUSED(state);
  const char *test_data = "bad::+";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *expr = NULL;

  expr = parse_type_lookup_expr(&token);
  assert_null(expr);
}

static void test_parse_data_lookup_ok(void **state) {
  UNUSED(state);
  const char *test_data = "a.b a.0 a.b.c a.b::c.d";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *expr = NULL;

  expr = parse_data_lookup_expr(&token);
  assert_topology(expr, "(DATA_LOOKUP_EXPR a b)");
  tau_anode_free(expr);

  expr = parse_data_lookup_expr(&token);
  assert_topology(expr, "(DATA_LOOKUP_EXPR a 0)");
  tau_anode_free(expr);

  expr = parse_data_lookup_expr(&token);
  assert_topology(expr, "(DATA_LOOKUP_EXPR (DATA_LOOKUP_EXPR a b) c)");
  tau_anode_free(expr);

  expr = parse_data_lookup_expr(&token);
  assert_topology(expr, "(DATA_LOOKUP_EXPR (DATA_LOOKUP_EXPR a (TYPE_LOOKUP_EXPR b c)) d)");
  tau_anode_free(expr);
}

static void test_parse_data_lookup_bad(void **state) {
  UNUSED(state);
  const char *test_data = "bad.+";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *expr = NULL;

  expr = parse_data_lookup_expr(&token);
  assert_null(expr);
}

static void test_parse_call_ok(void **state) {
  UNUSED(state);
  const char *test_data = "a() a(b) a(b, c) a::b(c, d) a(b)(c); (a)(b)";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *expr = NULL;

  expr = parse_call_expr(&token);
  assert_topology(expr, "(CALL_EXPR a (PASSING_ARGS))");
  tau_anode_free(expr);

  expr = parse_call_expr(&token);
  assert_topology(expr, "(CALL_EXPR a (PASSING_ARGS b))");
  tau_anode_free(expr);

  expr = parse_call_expr(&token);
  assert_topology(expr, "(CALL_EXPR a (PASSING_ARGS b c))");
  tau_anode_free(expr);

  expr = parse_call_expr(&token);
  assert_topology(expr, "(CALL_EXPR (TYPE_LOOKUP_EXPR a b) (PASSING_ARGS c d))");
  tau_anode_free(expr);

  expr = parse_call_expr(&token);
  assert_topology(expr, "(CALL_EXPR (CALL_EXPR a (PASSING_ARGS b)) (PASSING_ARGS c))");
  tau_anode_free(expr);

  // for the remaining ";"
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  expr = parse_call_expr(&token);
  assert_topology(expr, "(CALL_EXPR a (PASSING_ARGS b))");
  tau_anode_free(expr);
}

static void test_parse_call_bad(void **state) {
  UNUSED(state);
  const char *test_data = "a(b";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *expr = NULL;

  expr = parse_call_expr(&token);
  assert_null(expr);
}

int main() {
  UNUSED_TYPE(jmp_buf);
  UNUSED_TYPE(va_list);

  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_parse_identifier_ok),    // simple identifier
      cmocka_unit_test(test_parse_identifier_bad),   // bad identifier
      cmocka_unit_test(test_parse_atom_simple_ok),   // simple atoms (non-recursive)
      cmocka_unit_test(test_parse_atom_simple_bad),  // not atoms (non-recursive)
      cmocka_unit_test(test_parse_type_lookup_ok),   // common type lookup expressions
      cmocka_unit_test(test_parse_type_lookup_bad),  // not type lookup expressions
      cmocka_unit_test(test_parse_data_lookup_ok),   // data lookup expressions with descending
      cmocka_unit_test(test_parse_data_lookup_bad),  // not data lookup expressions
      cmocka_unit_test(test_parse_call_ok),          // call expressions with descending
      cmocka_unit_test(test_parse_call_bad),         // not a good call expression
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}