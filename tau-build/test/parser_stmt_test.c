// clang-format off
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
// clang-format on

#include "../src/common.h"
#include "topology_helper.h"

static void test_parse_return_stmt(void **state) {
  UNUSED(state);
  const char *test = "return; return 0; return a + b;";
  struct tau_token start = tau_token_start(__func__, test, strlen(test));
  struct tau_token token = tau_token_next(start);
  struct tau_node *node = NULL;

  node = parse_return_stmt(&token);
  assert_non_null(node);
  assert_node_topology(node, "(RETURN_STMT)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_return_stmt(&token);
  assert_non_null(node);
  assert_node_topology(node, "(RETURN_STMT 0)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_return_stmt(&token);
  assert_non_null(node);
  assert_node_topology(node, "(RETURN_STMT (ADD_EXPR a b))");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_continue_and_break_stmt(void **state) {
  UNUSED(state);
  const char *test = "continue; break;";
  struct tau_token start = tau_token_start(__func__, test, strlen(test));
  struct tau_token token = tau_token_next(start);
  struct tau_node *node = NULL;

  node = parse_continue_stmt(&token);
  assert_non_null(node);
  assert_node_topology(node, "(CONTINUE_STMT)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_break_stmt(&token);
  assert_non_null(node);
  assert_node_topology(node, "(BREAK_STMT)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

int main() {
  UNUSED_TYPE(jmp_buf);
  UNUSED_TYPE(va_list);

  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_parse_return_stmt),  // return <expr>
      cmocka_unit_test(test_parse_continue_and_break_stmt),  // continue, break
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}
