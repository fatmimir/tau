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

static void test_parse_nested_call_ok(void **state) {
  UNUSED(state);
  const char *test_data = "a(b())";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *expr = NULL;

  expr = parse_call_expr(&token);
  assert_topology(expr, "(CALL_EXPR a (PASSING_ARGS (CALL_EXPR b (PASSING_ARGS))))");
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

static void test_parse_tag_ok(void **state) {
  UNUSED(state);
  const char *test_data = "a'b a.b'c a'b()";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *expr = NULL;

  expr = parse_tag_expr(&token);
  assert_topology(expr, "(TAG_EXPR a b)");
  tau_anode_free(expr);

  expr = parse_tag_expr(&token);
  assert_topology(expr, "(TAG_EXPR (DATA_LOOKUP_EXPR a b) c)");
  tau_anode_free(expr);

  expr = parse_tag_expr(&token);
  assert_topology(expr, "(TAG_EXPR a (CALL_EXPR b (PASSING_ARGS)))");
  tau_anode_free(expr);
}

static void test_parse_unary_ok(void **state) {
  UNUSED(state);
  const char *test_data = "-a ~-a -a.b -a()";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *expr = NULL;

  expr = parse_unary_expr(&token);
  assert_topology(expr, "(UNARY_MINUS_EXPR a)");
  tau_anode_free(expr);

  expr = parse_unary_expr(&token);
  assert_topology(expr, "(UNARY_BIT_NOT_EXPR (UNARY_MINUS_EXPR a))");
  tau_anode_free(expr);

  expr = parse_unary_expr(&token);
  assert_topology(expr, "(UNARY_MINUS_EXPR (DATA_LOOKUP_EXPR a b))");
  tau_anode_free(expr);

  expr = parse_unary_expr(&token);
  assert_topology(expr, "(UNARY_MINUS_EXPR (CALL_EXPR a (PASSING_ARGS)))");
  tau_anode_free(expr);
}

static void test_parse_mul_ok(void **state) {
  UNUSED(state);
  const char *test_data = "a*b 2*a.b a*-b";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *expr = NULL;

  expr = parse_mul_expr(&token);
  assert_topology(expr, "(MUL_EXPR a b)");
  tau_anode_free(expr);

  expr = parse_mul_expr(&token);
  assert_topology(expr, "(MUL_EXPR 2 (DATA_LOOKUP_EXPR a b))");
  tau_anode_free(expr);

  expr = parse_mul_expr(&token);
  assert_topology(expr, "(MUL_EXPR a (UNARY_MINUS_EXPR b))");
  tau_anode_free(expr);
}

static void test_parse_sum_ok(void **state) {
  UNUSED(state);
  const char *test_data = "a+b 2+a*b a--b";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *expr = NULL;

  expr = parse_sum_expr(&token);
  assert_topology(expr, "(ADD_EXPR a b)");
  tau_anode_free(expr);

  expr = parse_sum_expr(&token);
  assert_topology(expr, "(ADD_EXPR 2 (MUL_EXPR a b))");
  tau_anode_free(expr);

  expr = parse_sum_expr(&token);
  assert_topology(expr, "(SUB_EXPR a (UNARY_MINUS_EXPR b))");
  tau_anode_free(expr);
}

static void test_parse_bit_shift_ok(void **state) {
  UNUSED(state);
  const char *test_data = "a>>b 2+a>>b a<<-b";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *expr = NULL;

  expr = parse_bit_shift_expr(&token);
  assert_topology(expr, "(BIT_RSH_EXPR a b)");
  tau_anode_free(expr);

  expr = parse_bit_shift_expr(&token);
  assert_topology(expr, "(BIT_RSH_EXPR (ADD_EXPR 2 a) b)");
  tau_anode_free(expr);

  expr = parse_bit_shift_expr(&token);
  assert_topology(expr, "(BIT_LSH_EXPR a (UNARY_MINUS_EXPR b))");
  tau_anode_free(expr);
}

static void test_parse_bit_and_ok(void **state) {
  UNUSED(state);
  const char *test_data = "a&b 2<<a&b a&b()";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *expr = NULL;

  expr = parse_bit_and_expr(&token);
  assert_topology(expr, "(BIT_AND_EXPR a b)");
  tau_anode_free(expr);

  expr = parse_bit_and_expr(&token);
  assert_topology(expr, "(BIT_AND_EXPR (BIT_LSH_EXPR 2 a) b)");
  tau_anode_free(expr);

  expr = parse_bit_and_expr(&token);
  assert_topology(expr, "(BIT_AND_EXPR a (CALL_EXPR b (PASSING_ARGS)))");
  tau_anode_free(expr);
}

static void test_parse_bit_or_ok(void **state) {
  UNUSED(state);
  const char *test_data = "a|b a&b|c a|b()";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *expr = NULL;

  expr = parse_bit_or_expr(&token);
  assert_topology(expr, "(BIT_OR_EXPR a b)");
  tau_anode_free(expr);

  expr = parse_bit_or_expr(&token);
  assert_topology(expr, "(BIT_OR_EXPR (BIT_AND_EXPR a b) c)");
  tau_anode_free(expr);

  expr = parse_bit_or_expr(&token);
  assert_topology(expr, "(BIT_OR_EXPR a (CALL_EXPR b (PASSING_ARGS)))");
  tau_anode_free(expr);
}

static void test_parse_cmp_ok(void **state) {
  UNUSED(state);
  const char *test_data = "a>b a|b>=c a<b()";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *expr = NULL;

  expr = parse_cmp_expr(&token);
  assert_topology(expr, "(CMP_GT_EXPR a b)");
  tau_anode_free(expr);

  expr = parse_cmp_expr(&token);
  assert_topology(expr, "(CMP_GE_EXPR (BIT_OR_EXPR a b) c)");
  tau_anode_free(expr);

  expr = parse_cmp_expr(&token);
  assert_topology(expr, "(CMP_LT_EXPR a (CALL_EXPR b (PASSING_ARGS)))");
  tau_anode_free(expr);
}

static void test_parse_rel_ok(void **state) {
  UNUSED(state);
  const char *test_data = "a==b a>b!=c a==b()";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *expr = NULL;

  expr = parse_rel_expr(&token);
  assert_topology(expr, "(REL_EQ_EXPR a b)");
  tau_anode_free(expr);

  expr = parse_rel_expr(&token);
  assert_topology(expr, "(REL_NE_EXPR (CMP_GT_EXPR a b) c)");
  tau_anode_free(expr);

  expr = parse_rel_expr(&token);
  assert_topology(expr, "(REL_EQ_EXPR a (CALL_EXPR b (PASSING_ARGS)))");
  tau_anode_free(expr);
}

static void test_parse_log_and_ok(void **state) {
  UNUSED(state);
  const char *test_data = "a&&b a==b&&c a&&b()";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *expr = NULL;

  expr = parse_log_and_expr(&token);
  assert_topology(expr, "(LOG_AND_EXPR a b)");
  tau_anode_free(expr);

  expr = parse_log_and_expr(&token);
  assert_topology(expr, "(LOG_AND_EXPR (REL_EQ_EXPR a b) c)");
  tau_anode_free(expr);

  expr = parse_log_and_expr(&token);
  assert_topology(expr, "(LOG_AND_EXPR a (CALL_EXPR b (PASSING_ARGS)))");
  tau_anode_free(expr);
}

static void test_parse_log_or_ok(void **state) {
  UNUSED(state);
  const char *test_data = "a||b a&&b||c a||b()";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *expr = NULL;

  expr = parse_log_or_expr(&token);
  assert_topology(expr, "(LOG_OR_EXPR a b)");
  tau_anode_free(expr);

  expr = parse_log_or_expr(&token);
  assert_topology(expr, "(LOG_OR_EXPR (LOG_AND_EXPR a b) c)");
  tau_anode_free(expr);

  expr = parse_log_or_expr(&token);
  assert_topology(expr, "(LOG_OR_EXPR a (CALL_EXPR b (PASSING_ARGS)))");
  tau_anode_free(expr);
}

static void test_parse_expr_ok(void **state) {
  UNUSED(state);
  const char *test_data = "add(2, std::random()) / -2";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *expr = NULL;

  expr = parse_expr(&token);
  assert_topology(expr,
                  "(DIV_EXPR (CALL_EXPR add (PASSING_ARGS 2 "
                  "(CALL_EXPR (TYPE_LOOKUP_EXPR std random) (PASSING_ARGS)))) "
                  "(UNARY_MINUS_EXPR 2))");
  tau_anode_free(expr);
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
      cmocka_unit_test(test_parse_nested_call_ok),   // call expressions with descending
      cmocka_unit_test(test_parse_call_bad),         // not a good call expression
      cmocka_unit_test(test_parse_tag_ok),           // tag expression: a'b
      cmocka_unit_test(test_parse_unary_ok),         // unary expressions
      cmocka_unit_test(test_parse_mul_ok),           // mul expressions
      cmocka_unit_test(test_parse_sum_ok),           // sum expressions
      cmocka_unit_test(test_parse_bit_shift_ok),     // bit shift expressions
      cmocka_unit_test(test_parse_bit_and_ok),       // bit and expressions
      cmocka_unit_test(test_parse_bit_or_ok),        // bit or expressions
      cmocka_unit_test(test_parse_cmp_ok),           // comparison expressions
      cmocka_unit_test(test_parse_rel_ok),           // relational expressions
      cmocka_unit_test(test_parse_log_and_ok),       // logic and expressions
      cmocka_unit_test(test_parse_log_or_ok),        // logic or expressions
      cmocka_unit_test(test_parse_expr_ok),          // any expression
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}