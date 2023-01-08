// clang-format off
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
// clang-format on

#include "../src/common.h"
#include "../src/parser_internal.h"
#include "../src/parser_match.h"
#include "topology_helper.h"

static void test_parse_let_decl(void **state) {
  UNUSED(state);
  const char *test =
      "let a: A prototype;"
      "let b: B = 0;";
  struct tau_token start = tau_token_start(__func__, test, strlen(test));
  struct tau_token token = tau_token_next(start);
  struct tau_node *node = NULL;

  node = parse_let_decl(&token);
  assert_non_null(node);
  assert_node_topology(node, "(LET_DECL (LET_DECONSTRUCTION a (TYPE_BIND A)) (PROTOTYPE_SUFFIX))");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_let_decl(&token);
  assert_non_null(node);
  assert_node_topology(node, "(LET_DECL (LET_DECONSTRUCTION b (TYPE_BIND B)) (DATA_BIND 0))");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_proc_decl(void **state) {
  UNUSED(state);
  const char *topology = NULL;
  const char *test =
      "proc a(): A prototype;"
      "proc b(): B = 1;"
      "proc c(): C {};"
      "proc d(arg: Arg): D prototype;"
      "proc e(x: X, y: Y): E prototype;";
  struct tau_token start = tau_token_start(__func__, test, strlen(test));
  struct tau_token token = tau_token_next(start);
  struct tau_node *node = NULL;

  node = parse_proc_decl(&token);
  assert_non_null(node);
  topology =
      "(PROC_DECL "
      " (PROC_DECONSTRUCTION a "
      "   (PROC_SIGNATURE "
      "     (FORMAL_ARGS) "
      "     (TYPE_BIND A)"
      "   )"
      " ) "
      " (PROTOTYPE_SUFFIX)"
      ")";
  assert_node_topology(node, topology);
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_proc_decl(&token);
  assert_non_null(node);
  topology =
      "(PROC_DECL "
      " (PROC_DECONSTRUCTION b "
      "   (PROC_SIGNATURE "
      "     (FORMAL_ARGS) "
      "     (TYPE_BIND B)"
      "   )"
      " ) "
      " (DATA_BIND 1)"
      ")";
  assert_node_topology(node, topology);
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_proc_decl(&token);
  assert_non_null(node);
  topology =
      "(PROC_DECL "
      " (PROC_DECONSTRUCTION c "
      "   (PROC_SIGNATURE "
      "     (FORMAL_ARGS) "
      "     (TYPE_BIND C)"
      "   )"
      " ) "
      " (BLOCK)"
      ")";
  assert_node_topology(node, topology);
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_proc_decl(&token);
  assert_non_null(node);
  topology =
      "(PROC_DECL "
      " (PROC_DECONSTRUCTION d "
      "   (PROC_SIGNATURE "
      "     (FORMAL_ARGS "
      "       (FORMAL_ARG "
      "         (ARG_BIND arg (TYPE_BIND Arg))"
      "       )"
      "     )"
      "     (TYPE_BIND D)"
      "   )"
      " ) "
      " (PROTOTYPE_SUFFIX)"
      ")";
  assert_node_topology(node, topology);
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_proc_decl(&token);
  assert_non_null(node);
  topology =
      "(PROC_DECL "
      " (PROC_DECONSTRUCTION e "
      "   (PROC_SIGNATURE "
      "     (FORMAL_ARGS "
      "       (FORMAL_ARG "
      "         (ARG_BIND x (TYPE_BIND X))"
      "         (FORMAL_ARG "
      "           (ARG_BIND y (TYPE_BIND Y))"
      "         )"
      "       )"
      "     )"
      "     (TYPE_BIND E)"
      "   )"
      " ) "
      " (PROTOTYPE_SUFFIX)"
      ")";
  assert_node_topology(node, topology);
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_type_decl(void **state) {
  UNUSED(state);
  const char *test =
      "type A prototype;"
      "type B = b;";
  struct tau_token start = tau_token_start(__func__, test, strlen(test));
  struct tau_token token = tau_token_next(start);
  struct tau_node *node = NULL;

  node = parse_type_decl(&token);
  assert_non_null(node);
  assert_node_topology(node, "(TYPE_DECL (TYPE_DECONSTRUCTION A) (PROTOTYPE_SUFFIX))");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_type_decl(&token);
  assert_non_null(node);
  assert_node_topology(node, "(TYPE_DECL (TYPE_DECONSTRUCTION B) (DATA_BIND b))");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_extern_decl(void **state) {
  UNUSED(state);
  const char *test =
      "extern let a: A prototype;"
      "extern proc b(): B prototype;"
      "extern type C prototype;";
  struct tau_token start = tau_token_start(__func__, test, strlen(test));
  struct tau_token token = tau_token_next(start);
  struct tau_node *node = NULL;

  node = parse_extern_decl(&token);
  assert_non_null(node);
  assert_node_topology(node, "(EXTERN_DECL (LET_DECL (LET_DECONSTRUCTION a (TYPE_BIND A)) (PROTOTYPE_SUFFIX)))");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_extern_decl(&token);
  assert_non_null(node);
  const char *topology =
      "(EXTERN_DECL "
      " (PROC_DECL "
      "  (PROC_DECONSTRUCTION b "
      "    (PROC_SIGNATURE "
      "      (FORMAL_ARGS) "
      "      (TYPE_BIND B)"
      "    )"
      "  ) "
      "  (PROTOTYPE_SUFFIX)"
      " )"
      ")";
  assert_node_topology(node, topology);
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_extern_decl(&token);
  assert_non_null(node);
  assert_node_topology(node, "(EXTERN_DECL (TYPE_DECL (TYPE_DECONSTRUCTION C) (PROTOTYPE_SUFFIX)))");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_decl(void **state) {
  UNUSED(state);
  const char *test =
      "let a: A prototype;"
      "proc b(): B prototype;"
      "type C prototype;"
      "extern type D prototype;";
  struct tau_token start = tau_token_start(__func__, test, strlen(test));
  struct tau_token token = tau_token_next(start);
  struct tau_node *node = NULL;

  node = parse_decl(&token);
  assert_non_null(node);
  assert_node_topology(node, "(DECL (LET_DECL (LET_DECONSTRUCTION a (TYPE_BIND A)) (PROTOTYPE_SUFFIX)))");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_decl(&token);
  assert_non_null(node);
  const char *topology =
      "(DECL"
      " (PROC_DECL "
      "  (PROC_DECONSTRUCTION b "
      "    (PROC_SIGNATURE "
      "      (FORMAL_ARGS) "
      "      (TYPE_BIND B)"
      "    )"
      "  ) "
      "  (PROTOTYPE_SUFFIX)"
      " )"
      ")";
  assert_node_topology(node, topology);
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_decl(&token);
  assert_non_null(node);
  assert_node_topology(node, "(DECL (TYPE_DECL (TYPE_DECONSTRUCTION C) (PROTOTYPE_SUFFIX)))");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_decl(&token);
  assert_non_null(node);
  assert_node_topology(node, "(DECL (EXTERN_DECL (TYPE_DECL (TYPE_DECONSTRUCTION D) (PROTOTYPE_SUFFIX))))");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_decls(void **state) {
  UNUSED(state);
  // we test using type-decl because they are easier to write down as topology trees, in theory all decls should be
  // parsed
  const char *test =
      "type A prototype;"
      "type B prototype;"
      "type C prototype;";
  struct tau_token start = tau_token_start(__func__, test, strlen(test));
  struct tau_token token = tau_token_next(start);
  struct tau_node *node = NULL;

  node = parse_decls(&token);
  assert_non_null(node);
  const char *topology =
      "(DECLS"
      " (DECL (TYPE_DECL (TYPE_DECONSTRUCTION A) (PROTOTYPE_SUFFIX))"
      "   (DECL (TYPE_DECL (TYPE_DECONSTRUCTION B) (PROTOTYPE_SUFFIX))"
      "     (DECL (TYPE_DECL (TYPE_DECONSTRUCTION C) (PROTOTYPE_SUFFIX)))"
      "   )"
      " )"
      ")";
  assert_node_topology(node, topology);
  node_free(node);
}

int main() {
  UNUSED_TYPE(jmp_buf);
  UNUSED_TYPE(va_list);

  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_parse_let_decl),     // all let variations
      cmocka_unit_test(test_parse_proc_decl),    // all proc variations
      cmocka_unit_test(test_parse_type_decl),    // all type variations
      cmocka_unit_test(test_parse_extern_decl),  // all extern variations
      cmocka_unit_test(test_parse_decl),         // any decl
      cmocka_unit_test(test_parse_decls),        // decls
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}
