// clang-format off
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
// clang-format on

#include "../src/common.h"
#include "topology_helper.h"

static void test_parse_atom(void **state) {
  UNUSED(state);
  const char *test = "120 id 0.1 +";
  struct tau_token start = tau_token_start(__func__, test, strlen(test));
  struct tau_token token = tau_token_next(start);
  struct tau_node *node = NULL;

  node = parse_atom(&token);
  assert_non_null(node);
  assert_node_topology(node, "120");
  node_free(node);

  node = parse_atom(&token);
  assert_non_null(node);
  assert_node_topology(node, "id");
  node_free(node);

  node = parse_atom(&token);
  assert_non_null(node);
  assert_node_topology(node, "0.1");
  node_free(node);

  node = parse_atom(&token);
  assert_null(node);
}

static void test_parse_primary_expr(void **state) {
  UNUSED(state);
  const char *test = "123; (123); ((123));";
  struct tau_token start = tau_token_start(__func__, test, strlen(test));
  struct tau_token token = tau_token_next(start);
  struct tau_node *node = NULL;

  node = parse_primary_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "123");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_primary_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "123");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_primary_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "123");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_index_expr(void **state) {
  UNUSED(state);
  const char *test = "a[1]; a[b][c];";
  struct tau_token start = tau_token_start(__func__, test, strlen(test));
  struct tau_token token = tau_token_next(start);
  struct tau_node *node = NULL;

  node = parse_index_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(INDEX_EXPR a 1)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_index_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(INDEX_EXPR (INDEX_EXPR a b) c)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_call_expr(void **state) {
  UNUSED(state);
  const char *test = "a(1); a(b)(c); a[b](c);";
  struct tau_token start = tau_token_start(__func__, test, strlen(test));
  struct tau_token token = tau_token_next(start);
  struct tau_node *node = NULL;

  node = parse_call_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(CALL_EXPR a 1)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_call_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(CALL_EXPR (CALL_EXPR a b) c)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_call_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(CALL_EXPR (INDEX_EXPR a b) c)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_unary_expr(void **state) {
  UNUSED(state);
  const char *test = "-1; &a; !~1;";
  struct tau_token start = tau_token_start(__func__, test, strlen(test));
  struct tau_token token = tau_token_next(start);
  struct tau_node *node = NULL;

  node = parse_unary_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(U_NEG_EXPR 1)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_unary_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(U_REF_EXPR a)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_unary_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(U_LOG_NOT_EXPR (U_BIT_NOT_EXPR 1))");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_value_lookup_expr(void **state) {
  UNUSED(state);
  const char *test = "a.b; a.b.c; a.b.0;";
  struct tau_token start = tau_token_start(__func__, test, strlen(test));
  struct tau_token token = tau_token_next(start);
  struct tau_node *node = NULL;

  node = parse_value_lookup_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(B_VALUE_LOOKUP_EXPR a b)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_value_lookup_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(B_VALUE_LOOKUP_EXPR (B_VALUE_LOOKUP_EXPR a b) c)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_value_lookup_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(B_VALUE_LOOKUP_EXPR (B_VALUE_LOOKUP_EXPR a b) 0)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}


int main() {
  UNUSED_TYPE(jmp_buf);
  UNUSED_TYPE(va_list);

  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_parse_atom),                  // <identifier> or <literal>
      cmocka_unit_test(test_parse_primary_expr),          // "(" <expr> ")" or <atom>
      cmocka_unit_test(test_parse_index_expr),            // <expr> "[" <expr> "]" ...
      cmocka_unit_test(test_parse_call_expr),             // <expr> "(" <expr> ")" ...
      cmocka_unit_test(test_parse_unary_expr),            // <op><expr>
      cmocka_unit_test(test_parse_value_lookup_expr),     // <expr>.<expr> ...
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}
