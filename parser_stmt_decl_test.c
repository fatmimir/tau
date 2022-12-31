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

static void test_parse_continue_break_return_ok(void **state) {
  UNUSED(state);
  const char *test_data = "continue; break; return; return 1;";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *statement = NULL;

  statement = parse_continue_stmt(&token);
  assert_topology(statement, "(CONTINUE_STMT)");
  tau_anode_free(statement);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  statement = parse_break_stmt(&token);
  assert_topology(statement, "(BREAK_STMT)");
  tau_anode_free(statement);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  statement = parse_return_stmt(&token);
  assert_topology(statement, "(RETURN_WITHOUT_EXPR_STMT)");
  tau_anode_free(statement);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  statement = parse_return_stmt(&token);
  assert_topology(statement, "(RETURN_STMT 1)");
  tau_anode_free(statement);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_assign_or_call_stmt_ok(void **state) {
  UNUSED(state);
  const char *test_data = "x = 2; y::z = 3; a.b = 4; x += 1; hey();";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *statement = NULL;

  statement = parse_assign_or_call_stmt(&token);
  assert_topology(statement, "(ASSIGN_SET_STMT x 2)");
  tau_anode_free(statement);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  statement = parse_assign_or_call_stmt(&token);
  assert_topology(statement, "(ASSIGN_SET_STMT (TYPE_LOOKUP_EXPR y z) 3)");
  tau_anode_free(statement);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  statement = parse_assign_or_call_stmt(&token);
  assert_topology(statement, "(ASSIGN_SET_STMT (DATA_LOOKUP_EXPR a b) 4)");
  tau_anode_free(statement);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  statement = parse_assign_or_call_stmt(&token);
  assert_topology(statement, "(ASSIGN_INC_STMT x 1)");
  tau_anode_free(statement);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  statement = parse_assign_or_call_stmt(&token);
  assert_topology(statement, "(CALL_STMT hey (PASSING_ARGS))");
  tau_anode_free(statement);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_if_stmt_ok(void **state) {
  UNUSED(state);
  const char *test_data =
      "if a { return; }; "
      "if a { return; } else { return; }; "
      "if a { return; } elif b { return; }; "
      "if a { return; } elif b { return; } elif c { return; }; "
      "if a { return; } elif b { return; } elif c { return; } else { return; };";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *statement = NULL;

  statement = parse_if_stmt(&token);
  assert_topology(statement, "(IF_BRANCH_STMT (THEN_CASE a (BLOCK (RETURN_WITHOUT_EXPR_STMT))))");
  tau_anode_free(statement);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  statement = parse_if_stmt(&token);
  assert_topology(statement,
                  "(IF_BRANCH_STMT "
                  "  (THEN_CASE a (BLOCK (RETURN_WITHOUT_EXPR_STMT)))"
                  "  (ELSE_CASE (BLOCK (RETURN_WITHOUT_EXPR_STMT)))"
                  ")");
  tau_anode_free(statement);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  statement = parse_if_stmt(&token);
  assert_topology(statement,
                  "(IF_BRANCH_STMT "
                  "  (THEN_CASE a (BLOCK (RETURN_WITHOUT_EXPR_STMT)))"
                  "  (ELIF_CASE b (BLOCK (RETURN_WITHOUT_EXPR_STMT)))"
                  ")");
  tau_anode_free(statement);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  statement = parse_if_stmt(&token);
  assert_topology(statement,
                  "(IF_BRANCH_STMT "
                  "  (THEN_CASE a (BLOCK (RETURN_WITHOUT_EXPR_STMT)))"
                  "  (ELIF_CASE b (BLOCK (RETURN_WITHOUT_EXPR_STMT)))"
                  "  (ELIF_CASE c (BLOCK (RETURN_WITHOUT_EXPR_STMT)))"
                  ")");
  tau_anode_free(statement);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  statement = parse_if_stmt(&token);
  assert_topology(statement,
                  "(IF_BRANCH_STMT "
                  "  (THEN_CASE a (BLOCK (RETURN_WITHOUT_EXPR_STMT)))"
                  "  (ELIF_CASE b (BLOCK (RETURN_WITHOUT_EXPR_STMT)))"
                  "  (ELIF_CASE c (BLOCK (RETURN_WITHOUT_EXPR_STMT)))"
                  "  (ELSE_CASE (BLOCK (RETURN_WITHOUT_EXPR_STMT)))"
                  ")");
  tau_anode_free(statement);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_loop_stmt_ok(void **state) {
  UNUSED(state);
  const char *test_data = "loop { break; };";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *statement = NULL;

  statement = parse_loop_stmt(&token);
  assert_topology(statement, "(LOOP_BRANCH_STMT (BLOCK (BREAK_STMT)))");
  tau_anode_free(statement);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_statement_ok(void **state) {
  UNUSED(state);
  const char *test_data = "loop { continue; if n { break; }; return; }; ";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *statement = NULL;

  statement = parse_statement(&token);
  assert_topology(statement,
                  "(LOOP_BRANCH_STMT "
                  " (BLOCK "
                  "   (CONTINUE_STMT)"
                  "   (IF_BRANCH_STMT (THEN_CASE n (BLOCK (BREAK_STMT) )))"
                  "   (RETURN_WITHOUT_EXPR_STMT)"
                  " )"
                  ")");
  tau_anode_free(statement);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_let_decl_ok(void **state) {
  UNUSED(state);
  const char *test_data = "let x: Y = 0; let x: Y prototype;";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *decl = NULL;

  decl = parse_let_decl(&token);
  assert_topology(decl, "(LET_DECL x (TYPE_BIND Y) (DATA_BIND 0))");
  tau_anode_free(decl);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  decl = parse_let_decl(&token);
  assert_topology(decl, "(LET_DECL x (TYPE_BIND Y) (PROTOTYPE))");
  tau_anode_free(decl);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_proc_decl_ok(void **state) {
  UNUSED(state);
  const char *test_data =
      "proc p(): O prototype;"
      "proc p(i: I): O prototype;"
      "proc p(i: I): O = 0;"
      "proc p(i: I): O { return 0; };";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *decl = NULL;

  decl = parse_proc_decl(&token);
  assert_topology(decl, "(PROC_DECL p (FORMAL_ARGS) (TYPE_BIND O) (PROTOTYPE))");
  tau_anode_free(decl);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  decl = parse_proc_decl(&token);
  assert_topology(decl, "(PROC_DECL p (FORMAL_ARGS (FORMAL_ARG i (TYPE_BIND I))) (TYPE_BIND O) (PROTOTYPE))");
  tau_anode_free(decl);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  decl = parse_proc_decl(&token);
  assert_topology(decl, "(PROC_DECL p (FORMAL_ARGS (FORMAL_ARG i (TYPE_BIND I))) (TYPE_BIND O) (DATA_BIND 0))");
  tau_anode_free(decl);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  decl = parse_proc_decl(&token);
  assert_topology(decl,
                  "(PROC_DECL p (FORMAL_ARGS (FORMAL_ARG i (TYPE_BIND I))) (TYPE_BIND O) (BLOCK (RETURN_STMT 0)))");
  tau_anode_free(decl);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_extern_decl_ok(void **state) {
  UNUSED(state);
  const char *test_data =
      "extern let v: T prototype;"
      "extern proc e(i: I): O prototype;";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *decl = NULL;

  decl = parse_extern_decl(&token);
  assert_topology(decl, "(EXTERN_DECL (LET_DECL v (TYPE_BIND T) (PROTOTYPE)))");
  tau_anode_free(decl);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  decl = parse_extern_decl(&token);
  assert_topology(decl,
                  "(EXTERN_DECL (PROC_DECL e (FORMAL_ARGS (FORMAL_ARG i (TYPE_BIND I))) (TYPE_BIND O) (PROTOTYPE)))");
  tau_anode_free(decl);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_module_decl_ok(void **state) {
  UNUSED(state);
  const char *test_data =
      "module name;"
      "module name::sub;";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *decl = NULL;

  decl = parse_module_decl(&token);
  assert_topology(decl, "(MODULE_DECL name)");
  tau_anode_free(decl);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  decl = parse_module_decl(&token);
  assert_topology(decl, "(MODULE_DECL (TYPE_LOOKUP_EXPR name sub))");
  tau_anode_free(decl);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_decl_ok(void **state) {
  UNUSED(state);
  const char *test_data =
      "let v0: V prototype;"
      "proc p0(): P = 0;"
      "extern let v1: V prototype;"
      "extern proc p1(): P prototype;";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *decl = NULL;

  decl = parse_decl(&token);
  assert_topology(decl, "(LET_DECL v0 (TYPE_BIND V) (PROTOTYPE))");
  tau_anode_free(decl);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  decl = parse_decl(&token);
  assert_topology(decl, "(PROC_DECL p0 (FORMAL_ARGS) (TYPE_BIND P) (DATA_BIND 0))");
  tau_anode_free(decl);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  decl = parse_decl(&token);
  assert_topology(decl, "(EXTERN_DECL (LET_DECL v1 (TYPE_BIND V) (PROTOTYPE)))");
  tau_anode_free(decl);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  decl = parse_decl(&token);
  assert_topology(decl, "(EXTERN_DECL (PROC_DECL p1 (FORMAL_ARGS) (TYPE_BIND P) (PROTOTYPE)))");
  tau_anode_free(decl);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_decls_ok(void **state) {
  UNUSED(state);
  const char *test_data =
      "let v0: V prototype;"
      "proc p0(): P = 0;"
      "extern let v1: V prototype;"
      "extern proc p1(): P prototype;";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *decl = NULL;

  decl = parse_decls(&token);
  assert_topology(decl,
                  "(DECLS "
                  "    (LET_DECL v0 (TYPE_BIND V) (PROTOTYPE))"
                  "    (PROC_DECL p0 (FORMAL_ARGS) (TYPE_BIND P) (DATA_BIND 0))"
                  "    (EXTERN_DECL (LET_DECL v1 (TYPE_BIND V) (PROTOTYPE)))"
                  "    (EXTERN_DECL (PROC_DECL p1 (FORMAL_ARGS) (TYPE_BIND P) (PROTOTYPE)))"
                  ")");
  tau_anode_free(decl);
}

static void test_parse_block_with_decls_and_stmts_ok(void **state) {
  UNUSED(state);
  const char *test_data =
      "{"
      "   let x: X = 0;"
      "   let y: Y = 1;"
      "   if x > 0 { return 1; };"
      "   return x + y;"
      "}";
  size_t test_data_size = strlen(test_data);
  struct tau_token starting = tau_token_start(__func__, test_data, test_data_size);
  struct tau_token token = tau_token_next(starting);
  struct tau_anode *decl = NULL;

  decl = parse_block(&token);
  assert_topology(decl,
                  "(BLOCK "
                  "    (LET_DECL x (TYPE_BIND X) (DATA_BIND 0))"
                  "    (LET_DECL y (TYPE_BIND Y) (DATA_BIND 1))"
                  "    (IF_BRANCH_STMT (THEN_CASE (CMP_GT_EXPR x 0) (BLOCK (RETURN_STMT 1))))"
                  "    (RETURN_STMT (ADD_EXPR x y))"
                  ")");
  tau_anode_free(decl);
}

int main() {
  UNUSED_TYPE(jmp_buf);
  UNUSED_TYPE(va_list);

  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_parse_continue_break_return_ok),  // continue, break and return
      cmocka_unit_test(test_parse_assign_or_call_stmt_ok),    // set, inc, dec ... call
      cmocka_unit_test(test_parse_if_stmt_ok),                // if variations
      cmocka_unit_test(test_parse_loop_stmt_ok),              // loops
      cmocka_unit_test(test_parse_statement_ok),              // any statement
      cmocka_unit_test(test_parse_let_decl_ok),               // all let decl variants
      cmocka_unit_test(test_parse_proc_decl_ok),              // all proc decl variants
      cmocka_unit_test(test_parse_extern_decl_ok),            // both proc extern and let extern
      cmocka_unit_test(test_parse_module_decl_ok),            // module declaration (not really a block declaration)
      cmocka_unit_test(test_parse_decl_ok),                   // block declarations
      cmocka_unit_test(test_parse_decls_ok),                  // file level declarations
      cmocka_unit_test(test_parse_block_with_decls_and_stmts_ok),  // both decls and stmts together
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}