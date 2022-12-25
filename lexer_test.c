//
// Created on 12/24/22.
//
// clang-format off
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
// clang-format on

#include "lexer.h"

#include <string.h>

#include "common.h"

static void test_tokenize_spaces(void **state) {
  UNUSED(state);
  const char *buf_data = "    a";
  const char *buf_name = __func__;
  size_t buf_len = strlen(buf_data);
  struct tau_token token = tau_token_start(buf_name, buf_data, buf_len);

  token = tau_token_next(token);
  assert_int_equal(token.loc.col, 4);
  assert_int_equal(token.loc.row, 0);
}

static void test_tokenize_tabs(void **state) {
  UNUSED(state);
  const char *buf_data = "   \ta";
  const char *buf_name = __func__;
  size_t buf_len = strlen(buf_data);
  struct tau_token token = tau_token_start(buf_name, buf_data, buf_len);

  token = tau_token_next(token);
  assert_int_equal(token.loc.col, 4);
  assert_int_equal(token.loc.row, 0);
}

static void test_tokenize_eol(void **state) {
  UNUSED(state);
  const char *buf_data = "\na";
  const char *buf_name = __func__;
  size_t buf_len = strlen(buf_data);
  struct tau_token token = tau_token_start(buf_name, buf_data, buf_len);

  token = tau_token_next(token);
  assert_int_equal(token.type, TAU_TOKEN_TYPE_EOL);
  assert_int_equal(token.len, 1);
  assert_memory_equal("\n", token.buf, token.len);
  assert_int_equal(token.loc.col, 0);
  assert_int_equal(token.loc.row, 1);
}

static void test_tokenize_dec_int_lit(void **state) {
  UNUSED(state);
  const char *buf_data = "123";
  const char *buf_name = __func__;
  size_t buf_len = strlen(buf_data);
  struct tau_token token = tau_token_start(buf_name, buf_data, buf_len);

  token = tau_token_next(token);
  assert_int_equal(token.type, TAU_TOKEN_TYPE_INT_LIT);
  assert_int_equal(token.len, 3);
  assert_memory_equal("123", token.buf, token.len);
  assert_int_equal(token.num_base, TAU_NUM_BASE_DEC);
  assert_int_equal(token.loc.col, 0);
  assert_int_equal(token.loc.row, 0);
}

static void test_tokenize_bin_int_lit(void **state) {
  UNUSED(state);
  const char *buf_data = "0b1010";
  const char *buf_name = __func__;
  size_t buf_len = strlen(buf_data);
  struct tau_token token = tau_token_start(buf_name, buf_data, buf_len);

  token = tau_token_next(token);
  assert_int_equal(token.type, TAU_TOKEN_TYPE_INT_LIT);
  assert_int_equal(token.len, 6);
  assert_memory_equal("0b1010", token.buf, token.len);
  assert_int_equal(token.num_base, TAU_NUM_BASE_BIN);
  assert_int_equal(token.loc.col, 0);
  assert_int_equal(token.loc.row, 0);
}

static void test_tokenize_oct_int_lit(void **state) {
  UNUSED(state);
  const char *buf_data = "0o7070";
  const char *buf_name = __func__;
  size_t buf_len = strlen(buf_data);
  struct tau_token token = tau_token_start(buf_name, buf_data, buf_len);

  token = tau_token_next(token);
  assert_int_equal(token.type, TAU_TOKEN_TYPE_INT_LIT);
  assert_int_equal(token.len, 6);
  assert_memory_equal("0o7070", token.buf, token.len);
  assert_int_equal(token.num_base, TAU_NUM_BASE_OCT);
  assert_int_equal(token.loc.col, 0);
  assert_int_equal(token.loc.row, 0);
}

static void test_tokenize_hex_int_lit(void **state) {
  UNUSED(state);
  const char *buf_data = "0xFAFA";
  const char *buf_name = __func__;
  size_t buf_len = strlen(buf_data);
  struct tau_token token = tau_token_start(buf_name, buf_data, buf_len);

  token = tau_token_next(token);
  assert_int_equal(token.type, TAU_TOKEN_TYPE_INT_LIT);
  assert_int_equal(token.len, 6);
  assert_memory_equal("0xFAFA", token.buf, token.len);
  assert_int_equal(token.num_base, TAU_NUM_BASE_HEX);
  assert_int_equal(token.loc.col, 0);
  assert_int_equal(token.loc.row, 0);
}

static void test_tokenize_flt_lit(void **state) {
  UNUSED(state);
  const char *buf_data = "0.12";
  const char *buf_name = __func__;
  size_t buf_len = strlen(buf_data);
  struct tau_token token = tau_token_start(buf_name, buf_data, buf_len);

  token = tau_token_next(token);
  assert_int_equal(token.type, TAU_TOKEN_TYPE_FLT_LIT);
  assert_int_equal(token.len, 4);
  assert_memory_equal("0.12", token.buf, token.len);
  assert_int_equal(token.num_base, TAU_NUM_BASE_DEC);
  assert_int_equal(token.loc.col, 0);
  assert_int_equal(token.loc.row, 0);
}

static void test_tokenize_flt_with_exp_lit(void **state) {
  UNUSED(state);
  const char *buf_data = "0.12e+10";
  const char *buf_name = __func__;
  size_t buf_len = strlen(buf_data);
  struct tau_token token = tau_token_start(buf_name, buf_data, buf_len);

  token = tau_token_next(token);
  assert_int_equal(token.type, TAU_TOKEN_TYPE_FLT_LIT);
  assert_int_equal(token.len, 8);
  assert_memory_equal("0.12e+10", token.buf, token.len);
  assert_int_equal(token.num_base, TAU_NUM_BASE_DEC);
  assert_int_equal(token.loc.col, 0);
  assert_int_equal(token.loc.row, 0);
}

static void test_tokenize_str_lit(void **state) {
  UNUSED(state);
  const char *buf_data = "\"abc\"";
  const char *buf_name = __func__;
  size_t buf_len = strlen(buf_data);
  struct tau_token token = tau_token_start(buf_name, buf_data, buf_len);

  token = tau_token_next(token);
  assert_int_equal(token.type, TAU_TOKEN_TYPE_STR_LIT);
  assert_int_equal(token.len, 5);
  assert_memory_equal("\"abc\"", token.buf, token.len);
  assert_int_equal(token.loc.col, 0);
  assert_int_equal(token.loc.row, 0);
}

static void test_tokenize_str_with_esc_lit(void **state) {
  UNUSED(state);
  const char *buf_data = "\"\\n\\xFA\"";
  const char *buf_name = __func__;
  size_t buf_len = strlen(buf_data);
  struct tau_token token = tau_token_start(buf_name, buf_data, buf_len);

  token = tau_token_next(token);
  assert_int_equal(token.type, TAU_TOKEN_TYPE_STR_LIT);
  assert_int_equal(token.len, 8);
  assert_memory_equal("\"\\n\\xFA\"", token.buf, token.len);
  assert_int_equal(token.loc.col, 0);
  assert_int_equal(token.loc.row, 0);
}

static void test_tokenize_punct_single(void **state) {
  UNUSED(state);
  const char *buf_data = "+";
  const char *buf_name = __func__;
  size_t buf_len = strlen(buf_data);
  struct tau_token token = tau_token_start(buf_name, buf_data, buf_len);

  token = tau_token_next(token);
  assert_int_equal(token.type, TAU_TOKEN_TYPE_PUNCT);
  assert_int_equal(token.len, 1);
  assert_memory_equal("+", token.buf, token.len);
  assert_int_equal(token.punct, TAU_PUNCT_PLUS);
  assert_int_equal(token.loc.col, 0);
  assert_int_equal(token.loc.row, 0);
}

static void test_tokenize_punct_multiple(void **state) {
  UNUSED(state);
  const char *buf_data = ">>=";
  const char *buf_name = __func__;
  size_t buf_len = strlen(buf_data);
  struct tau_token token = tau_token_start(buf_name, buf_data, buf_len);

  token = tau_token_next(token);
  assert_int_equal(token.type, TAU_TOKEN_TYPE_PUNCT);
  assert_int_equal(token.len, 3);
  assert_memory_equal(">>=", token.buf, token.len);
  assert_int_equal(token.punct, TAU_PUNCT_D_GT_EQ);
  assert_int_equal(token.loc.col, 0);
  assert_int_equal(token.loc.row, 0);
}

static void test_tokenize_punct_lumped(void **state) {
  UNUSED(state);
  const char *buf_data = "+==";
  const char *buf_name = __func__;
  size_t buf_len = strlen(buf_data);
  struct tau_token token = tau_token_start(buf_name, buf_data, buf_len);

  token = tau_token_next(token);
  assert_int_equal(token.type, TAU_TOKEN_TYPE_PUNCT);
  assert_int_equal(token.len, 2);
  assert_memory_equal("+=", token.buf, token.len);
  assert_int_equal(token.punct, TAU_PUNCT_PLUS_EQ);
  assert_int_equal(token.loc.col, 0);
  assert_int_equal(token.loc.row, 0);

  token = tau_token_next(token);
  assert_int_equal(token.type, TAU_TOKEN_TYPE_PUNCT);
  assert_int_equal(token.len, 1);
  assert_memory_equal("=", token.buf, token.len);
  assert_int_equal(token.punct, TAU_PUNCT_EQ);
  assert_int_equal(token.loc.col, 2);
  assert_int_equal(token.loc.row, 0);
}

static void test_tokenize_identifier(void **state) {
  UNUSED(state);
  const char *buf_data = "a a1 a_2 a$3";
  const char *buf_name = __func__;
  size_t buf_len = strlen(buf_data);
  struct tau_token token = tau_token_start(buf_name, buf_data, buf_len);

  token = tau_token_next(token);
  assert_int_equal(token.type, TAU_TOKEN_TYPE_IDENTIFIER);
  assert_int_equal(token.len, 1);
  assert_memory_equal("a", token.buf, token.len);
  assert_int_equal(token.loc.col, 0);
  assert_int_equal(token.loc.row, 0);

  token = tau_token_next(token);
  assert_int_equal(token.type, TAU_TOKEN_TYPE_IDENTIFIER);
  assert_int_equal(token.len, 2);
  assert_memory_equal("a1", token.buf, token.len);
  assert_int_equal(token.loc.col, 2);
  assert_int_equal(token.loc.row, 0);

  token = tau_token_next(token);
  assert_int_equal(token.type, TAU_TOKEN_TYPE_IDENTIFIER);
  assert_int_equal(token.len, 3);
  assert_memory_equal("a_2", token.buf, token.len);
  assert_int_equal(token.loc.col, 5);
  assert_int_equal(token.loc.row, 0);

  token = tau_token_next(token);
  assert_int_equal(token.type, TAU_TOKEN_TYPE_IDENTIFIER);
  assert_int_equal(token.len, 3);
  assert_memory_equal("a$3", token.buf, token.len);
  assert_int_equal(token.loc.col, 9);
  assert_int_equal(token.loc.row, 0);
}

static void test_tokenize_bol_lit(void **state) {
  UNUSED(state);
  const char *buf_data = "true false";
  const char *buf_name = __func__;
  size_t buf_len = strlen(buf_data);
  struct tau_token token = tau_token_start(buf_name, buf_data, buf_len);

  token = tau_token_next(token);
  assert_int_equal(token.type, TAU_TOKEN_TYPE_BOL_LIT);
  assert_int_equal(token.len, 4);
  assert_memory_equal("true", token.buf, token.len);
  assert_int_equal(token.loc.col, 0);
  assert_int_equal(token.loc.row, 0);

  token = tau_token_next(token);
  assert_int_equal(token.type, TAU_TOKEN_TYPE_BOL_LIT);
  assert_int_equal(token.len, 5);
  assert_memory_equal("false", token.buf, token.len);
  assert_int_equal(token.loc.col, 5);
  assert_int_equal(token.loc.row, 0);
}

static void test_tokenize_nil_unit_lit(void **state) {
  UNUSED(state);
  const char *buf_data = "unit nil";
  const char *buf_name = __func__;
  size_t buf_len = strlen(buf_data);
  struct tau_token token = tau_token_start(buf_name, buf_data, buf_len);

  token = tau_token_next(token);
  assert_int_equal(token.type, TAU_TOKEN_TYPE_UNI_LIT);
  assert_int_equal(token.len, 4);
  assert_memory_equal("unit", token.buf, token.len);
  assert_int_equal(token.loc.col, 0);
  assert_int_equal(token.loc.row, 0);

  token = tau_token_next(token);
  assert_int_equal(token.type, TAU_TOKEN_TYPE_NIL_LIT);
  assert_int_equal(token.len, 3);
  assert_memory_equal("nil", token.buf, token.len);
  assert_int_equal(token.loc.col, 5);
  assert_int_equal(token.loc.row, 0);
}

static void test_tokenize_keyword(void **state) {
  UNUSED(state);
  const char *buf_data = "module";
  const char *buf_name = __func__;
  size_t buf_len = strlen(buf_data);
  struct tau_token token = tau_token_start(buf_name, buf_data, buf_len);

  token = tau_token_next(token);
  assert_int_equal(token.type, TAU_TOKEN_TYPE_KEYWORD);
  assert_int_equal(token.len, 6);
  assert_memory_equal("module", token.buf, token.len);
  assert_int_equal(token.keyword, TAU_KEYWORD_MODULE);
  assert_int_equal(token.loc.col, 0);
  assert_int_equal(token.loc.row, 0);
}

static void test_bracket_balance(void **state) {
  UNUSED(state);

  const char *buf_data = "({[]})";
  const char *buf_name = __func__;
  size_t buf_len = strlen(buf_data);
  struct tau_token token = tau_token_start(buf_name, buf_data, buf_len);

  token = tau_token_next(token);
  assert_int_equal(token.type, TAU_TOKEN_TYPE_PUNCT);
  assert_int_equal(token.punct, TAU_PUNCT_LPAR);
  assert_int_equal(token.par_balance, 1);
  assert_int_equal(token.sbr_balance, 0);
  assert_int_equal(token.cbr_balance, 0);

  token = tau_token_next(token);
  assert_int_equal(token.type, TAU_TOKEN_TYPE_PUNCT);
  assert_int_equal(token.punct, TAU_PUNCT_LCBR);
  assert_int_equal(token.par_balance, 1);
  assert_int_equal(token.sbr_balance, 0);
  assert_int_equal(token.cbr_balance, 1);

  token = tau_token_next(token);
  assert_int_equal(token.type, TAU_TOKEN_TYPE_PUNCT);
  assert_int_equal(token.punct, TAU_PUNCT_LSBR);
  assert_int_equal(token.par_balance, 1);
  assert_int_equal(token.sbr_balance, 1);
  assert_int_equal(token.cbr_balance, 1);

  token = tau_token_next(token);
  assert_int_equal(token.type, TAU_TOKEN_TYPE_PUNCT);
  assert_int_equal(token.punct, TAU_PUNCT_RSBR);
  assert_int_equal(token.par_balance, 1);
  assert_int_equal(token.sbr_balance, 0);
  assert_int_equal(token.cbr_balance, 1);

  token = tau_token_next(token);
  assert_int_equal(token.type, TAU_TOKEN_TYPE_PUNCT);
  assert_int_equal(token.punct, TAU_PUNCT_RCBR);
  assert_int_equal(token.par_balance, 1);
  assert_int_equal(token.sbr_balance, 0);
  assert_int_equal(token.cbr_balance, 0);

  token = tau_token_next(token);
  assert_int_equal(token.type, TAU_TOKEN_TYPE_PUNCT);
  assert_int_equal(token.punct, TAU_PUNCT_RPAR);
  assert_int_equal(token.par_balance, 0);
  assert_int_equal(token.sbr_balance, 0);
  assert_int_equal(token.cbr_balance, 0);
}

static void test_non_eol_elision(void **state) {
  UNUSED(state);

  const char *buf_data = "(\n)\n";
  const char *buf_name = __func__;
  size_t buf_len = strlen(buf_data);
  struct tau_token token = tau_token_start(buf_name, buf_data, buf_len);

  token = tau_token_next(token);
  assert_int_equal(token.type, TAU_TOKEN_TYPE_PUNCT);
  assert_int_equal(token.punct, TAU_PUNCT_LPAR);

  // Here: lexer should not emit end of line because it's within two '('

  token = tau_token_next(token);
  assert_int_equal(token.type, TAU_TOKEN_TYPE_PUNCT);
  assert_int_equal(token.punct, TAU_PUNCT_RPAR);

  // Here: now it should emit EOL again because line it's expected to end
  
  token = tau_token_next(token);
  assert_int_equal(token.type, TAU_TOKEN_TYPE_EOL);
}

int main() {
  UNUSED_TYPE(jmp_buf);
  UNUSED_TYPE(va_list);

  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_tokenize_spaces),            // simple spaces
      cmocka_unit_test(test_tokenize_tabs),              // simple tabs
      cmocka_unit_test(test_tokenize_eol),               // end of lines
      cmocka_unit_test(test_tokenize_dec_int_lit),       // decimal integer literal
      cmocka_unit_test(test_tokenize_bin_int_lit),       // binary integer literal
      cmocka_unit_test(test_tokenize_oct_int_lit),       // octal integer literal
      cmocka_unit_test(test_tokenize_hex_int_lit),       // hexadecimal integer literal
      cmocka_unit_test(test_tokenize_flt_lit),           // floating point literal
      cmocka_unit_test(test_tokenize_flt_with_exp_lit),  // floating point with exponent literal
      cmocka_unit_test(test_tokenize_str_lit),           // simple string literal
      cmocka_unit_test(test_tokenize_str_with_esc_lit),  // string with escape sequences literal
      cmocka_unit_test(test_tokenize_punct_single),      // a single punct symbol
      cmocka_unit_test(test_tokenize_punct_multiple),    // multiple punct symbols
      cmocka_unit_test(test_tokenize_punct_lumped),      // multiple punct symbols lumped together
      cmocka_unit_test(test_tokenize_identifier),        // identifier
      cmocka_unit_test(test_tokenize_bol_lit),           // boolean literal
      cmocka_unit_test(test_tokenize_nil_unit_lit),      // "unit" or "nil" literal
      cmocka_unit_test(test_tokenize_keyword),           // "module" keyword
      cmocka_unit_test(test_bracket_balance),            // bracket balance count
      cmocka_unit_test(test_non_eol_elision),            // don't emit EOL if brackets are unbalanced
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}