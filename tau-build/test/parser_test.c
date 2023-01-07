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
  const char *test = "-1; +a; !~1;";
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
  assert_node_topology(node, "(U_POS_EXPR a)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_unary_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(U_LOG_NOT_EXPR (U_BIT_NOT_EXPR 1))");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_static_lookup_expr(void **state) {
  UNUSED(state);
  const char *test = "a::b; a::b::c; a::b.0;";
  struct tau_token start = tau_token_start(__func__, test, strlen(test));
  struct tau_token token = tau_token_next(start);
  struct tau_node *node = NULL;

  node = parse_value_lookup_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(STATIC_LOOKUP_EXPR a b)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_value_lookup_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(STATIC_LOOKUP_EXPR (STATIC_LOOKUP_EXPR a b) c)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_value_lookup_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(VALUE_LOOKUP_EXPR (STATIC_LOOKUP_EXPR a b) 0)");
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
  assert_node_topology(node, "(VALUE_LOOKUP_EXPR a b)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_value_lookup_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(VALUE_LOOKUP_EXPR (VALUE_LOOKUP_EXPR a b) c)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_value_lookup_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(VALUE_LOOKUP_EXPR (VALUE_LOOKUP_EXPR a b) 0)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_proof_expr(void **state) {
  UNUSED(state);
  const char *test = "a:b; a:b:c; a:b::c;";
  struct tau_token start = tau_token_start(__func__, test, strlen(test));
  struct tau_token token = tau_token_next(start);
  struct tau_node *node = NULL;

  node = parse_proof_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(PROOF_EXPR a b)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_proof_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(PROOF_EXPR (PROOF_EXPR a b) c)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_proof_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(PROOF_EXPR a (STATIC_LOOKUP_EXPR b c))");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_ref_expr(void **state) {
  UNUSED(state);
  const char *test = "&a; &a::b; &a::b.c;";
  struct tau_token start = tau_token_start(__func__, test, strlen(test));
  struct tau_token token = tau_token_next(start);
  struct tau_node *node = NULL;

  node = parse_ref_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(U_REF_EXPR a)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_ref_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(U_REF_EXPR (STATIC_LOOKUP_EXPR a b))");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_ref_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(U_REF_EXPR (VALUE_LOOKUP_EXPR (STATIC_LOOKUP_EXPR a b) c))");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_fact_expr(void **state) {
  UNUSED(state);
  const char *test = "a*b; a*b/c; -a%b;";
  struct tau_token start = tau_token_start(__func__, test, strlen(test));
  struct tau_token token = tau_token_next(start);
  struct tau_node *node = NULL;

  node = parse_fact_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(MUL_EXPR a b)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_fact_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(DIV_EXPR (MUL_EXPR a b) c)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_fact_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(REM_EXPR (U_NEG_EXPR a) b)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_term_expr(void **state) {
  UNUSED(state);
  const char *test = "a+b; a+b-c; -a-b;";
  struct tau_token start = tau_token_start(__func__, test, strlen(test));
  struct tau_token token = tau_token_next(start);
  struct tau_node *node = NULL;

  node = parse_term_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(ADD_EXPR a b)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_term_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(SUB_EXPR (ADD_EXPR a b) c)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_term_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(SUB_EXPR (U_NEG_EXPR a) b)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_bit_shift_expr(void **state) {
  UNUSED(state);
  const char *test = "a>>b; a<<b>>c; a>>b*c;";
  struct tau_token start = tau_token_start(__func__, test, strlen(test));
  struct tau_token token = tau_token_next(start);
  struct tau_node *node = NULL;

  node = parse_bit_shift_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(RSH_EXPR a b)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_bit_shift_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(RSH_EXPR (LSH_EXPR a b) c)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_bit_shift_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(RSH_EXPR a (MUL_EXPR b c))");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_bit_and_expr(void **state) {
  UNUSED(state);
  const char *test = "a&b; a&b&c; a&b>>c;";
  struct tau_token start = tau_token_start(__func__, test, strlen(test));
  struct tau_token token = tau_token_next(start);
  struct tau_node *node = NULL;

  node = parse_bit_and_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(BIT_AND_EXPR a b)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_bit_and_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(BIT_AND_EXPR (BIT_AND_EXPR a b) c)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_bit_and_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(BIT_AND_EXPR a (RSH_EXPR b c))");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_bit_or_expr(void **state) {
  UNUSED(state);
  const char *test = "a|b; a|b^c; a|b&c;";
  struct tau_token start = tau_token_start(__func__, test, strlen(test));
  struct tau_token token = tau_token_next(start);
  struct tau_node *node = NULL;

  node = parse_bit_or_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(BIT_OR_EXPR a b)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_bit_or_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(BIT_XOR_EXPR (BIT_OR_EXPR a b) c)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_bit_or_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(BIT_OR_EXPR a (BIT_AND_EXPR b c))");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_cmp_expr(void **state) {
  UNUSED(state);
  const char *test = "a>b; a>=b<=c; a<b|c;";
  struct tau_token start = tau_token_start(__func__, test, strlen(test));
  struct tau_token token = tau_token_next(start);
  struct tau_node *node = NULL;

  node = parse_cmp_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(GT_EXPR a b)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_cmp_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(LE_EXPR (GE_EXPR a b) c)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_cmp_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(LT_EXPR a (BIT_OR_EXPR b c))");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_rel_expr(void **state) {
  UNUSED(state);
  const char *test = "a==b; a==b!=c; a==b>c;";
  struct tau_token start = tau_token_start(__func__, test, strlen(test));
  struct tau_token token = tau_token_next(start);
  struct tau_node *node = NULL;

  node = parse_rel_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(EQ_EXPR a b)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_rel_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(NE_EXPR (EQ_EXPR a b) c)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_rel_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(EQ_EXPR a (GT_EXPR b c))");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_log_and_expr(void **state) {
  UNUSED(state);
  const char *test = "a&&b; a&&b&&c; a&&b==c;";
  struct tau_token start = tau_token_start(__func__, test, strlen(test));
  struct tau_token token = tau_token_next(start);
  struct tau_node *node = NULL;

  node = parse_log_and_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(LOG_AND_EXPR a b)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_log_and_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(LOG_AND_EXPR (LOG_AND_EXPR a b) c)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_log_and_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(LOG_AND_EXPR a (EQ_EXPR b c))");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_log_or_expr(void **state) {
  UNUSED(state);
  const char *test = "a||b; a||b||c; a||b&&c;";
  struct tau_token start = tau_token_start(__func__, test, strlen(test));
  struct tau_token token = tau_token_next(start);
  struct tau_node *node = NULL;

  node = parse_log_or_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(LOG_OR_EXPR a b)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_log_or_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(LOG_OR_EXPR (LOG_OR_EXPR a b) c)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_log_or_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(LOG_OR_EXPR a (LOG_AND_EXPR b c))");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

static void test_parse_cast_expr(void **state) {
  UNUSED(state);
  const char *test = "a as b; a as b as c; a as b || c;";
  struct tau_token start = tau_token_start(__func__, test, strlen(test));
  struct tau_token token = tau_token_next(start);
  struct tau_node *node = NULL;

  node = parse_cast_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(CAST_EXPR a b)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_cast_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(CAST_EXPR (CAST_EXPR a b) c)");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  node = parse_cast_expr(&token);
  assert_non_null(node);
  assert_node_topology(node, "(CAST_EXPR a (LOG_OR_EXPR b c))");
  node_free(node);
  assert_true(match_and_consume(&token, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));
}

int main() {
  UNUSED_TYPE(jmp_buf);
  UNUSED_TYPE(va_list);

  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_parse_atom),                // <identifier> or <literal>
      cmocka_unit_test(test_parse_primary_expr),        // "(" <expr> ")" or <atom>
      cmocka_unit_test(test_parse_index_expr),          // <expr> "[" <expr> "]" ...
      cmocka_unit_test(test_parse_call_expr),           // <expr> "(" <expr> ")" ...
      cmocka_unit_test(test_parse_unary_expr),          // <op><expr>
      cmocka_unit_test(test_parse_static_lookup_expr),  // <expr>::<expr> ...
      cmocka_unit_test(test_parse_value_lookup_expr),   // <expr>.<expr> ...
      cmocka_unit_test(test_parse_proof_expr),          // <expr>:<expr> ...
      cmocka_unit_test(test_parse_ref_expr),            // &<expr> ...
      cmocka_unit_test(test_parse_fact_expr),           // <expr> <* / %> <expr>,
      cmocka_unit_test(test_parse_term_expr),           // <expr> <+ -> <expr>,
      cmocka_unit_test(test_parse_bit_shift_expr),      // <expr> <'>>' '<<'> <expr>,
      cmocka_unit_test(test_parse_bit_and_expr),        // <expr> & <expr>,
      cmocka_unit_test(test_parse_bit_or_expr),         // <expr> <| ^> <expr>,
      cmocka_unit_test(test_parse_cmp_expr),            // <expr> <'>=' '>' '<' '<='> <expr>,
      cmocka_unit_test(test_parse_rel_expr),            // <expr> <== !=> <expr>,
      cmocka_unit_test(test_parse_log_and_expr),        // <expr> <&&> <expr>,
      cmocka_unit_test(test_parse_log_or_expr),         // <expr> <||> <expr>,
      cmocka_unit_test(test_parse_cast_expr),           // <expr> as <expr>,
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}
