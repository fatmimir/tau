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

static void test_parse_if_stmt(void **state) {
  UNUSED(state);
  const char *topology = NULL;
  const char *test =
      "if a { };"
      "if a { } elif b { };"
      "if a { } elif b { } elif c { };"
      "if a { } elif b { } elif c { } else { };"
      "if a { } else { };";
  struct tau_token start = tau_token_start(__func__, test, strlen(test));
  struct tau_token token = tau_token_next(start);
  struct tau_node *node = NULL;

  node = parse_if_stmt(&token);
  assert_non_null(node);
  topology =
      ""
      "(IF_STMT "
      " (MAIN_BRANCH "
      "   (EXPR_WITH_BLOCK a "
      "     (BLOCK)"
      "   )"
      " )"
      ")";
  assert_node_topology(node, topology);
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_if_stmt(&token);
  assert_non_null(node);
  topology =
      ""
      "(IF_STMT "
      " (MAIN_BRANCH "
      "   (EXPR_WITH_BLOCK a "
      "     (BLOCK)"
      "   )"
      "   (ELIF_BRANCH"
      "     (EXPR_WITH_BLOCK b"
      "       (BLOCK)"
      "     )"
      "   )"
      " )"
      ")";
  assert_node_topology(node, topology);
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_if_stmt(&token);
  assert_non_null(node);
  topology =
      ""
      "(IF_STMT "
      " (MAIN_BRANCH "
      "   (EXPR_WITH_BLOCK a "
      "     (BLOCK)"
      "   )"
      "   (ELIF_BRANCH"
      "     (EXPR_WITH_BLOCK b"
      "       (BLOCK)"
      "     )"
      "     (ELIF_BRANCH"
      "       (EXPR_WITH_BLOCK c"
      "         (BLOCK)"
      "       )"
      "     )"
      "   )"
      " )"
      ")";
  assert_node_topology(node, topology);
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_if_stmt(&token);
  assert_non_null(node);
  topology =
      ""
      "(IF_STMT "
      " (MAIN_BRANCH "
      "   (EXPR_WITH_BLOCK a "
      "     (BLOCK)"
      "   )"
      "   (ELIF_BRANCH"
      "     (EXPR_WITH_BLOCK b"
      "       (BLOCK)"
      "     )"
      "     (ELIF_BRANCH"
      "       (EXPR_WITH_BLOCK c"
      "         (BLOCK)"
      "       )"
      "     )"
      "   )"
      " )"
      " (ELSE_BRANCH (BLOCK))"
      ")";
  assert_node_topology(node, topology);
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_if_stmt(&token);
  assert_non_null(node);
  topology =
      ""
      "(IF_STMT "
      " (MAIN_BRANCH "
      "   (EXPR_WITH_BLOCK a "
      "     (BLOCK)"
      "   )"
      " )"
      " (ELSE_BRANCH (BLOCK))"
      ")";
  assert_node_topology(node, topology);
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_while_stmt(void **state) {
  const char *test = "while a { };";
  struct tau_token start = tau_token_start(__func__, test, strlen(test));
  struct tau_token token = tau_token_next(start);
  struct tau_node *node = NULL;

  node = parse_while_stmt(&token);
  assert_non_null(node);
  assert_node_topology(node, "(WHILE_STMT (EXPR_WITH_BLOCK a (BLOCK)))");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

int main() {
  UNUSED_TYPE(jmp_buf);
  UNUSED_TYPE(va_list);

  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_parse_return_stmt),              // return <expr>
      cmocka_unit_test(test_parse_continue_and_break_stmt),  // continue, break
      cmocka_unit_test(test_parse_if_stmt),                  // all if variants
      cmocka_unit_test(test_parse_while_stmt),               // all while variants
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}
