//
// Created on 12/21/22.
//
// clang-format off
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
// clang-format on

#include "common.h"
#include "parser.h"

#include <string.h>

static void test_parse_spaces(void **state) {
  UNUSED(state);
  struct tau_parser *parser;

  const char *test0 = " \n\t a";
  parser = tau_parser_new(__func__ , test0, strlen(test0));
  assert_true(tau_parse_spaces(parser));
  assert_ptr_equal(parser->cur_offset, test0+4);
  assert_int_equal(parser->loc.row, 1);
  assert_int_equal(parser->loc.col, 3);
  tau_parser_free(parser);
}

static void test_parse_line_comment(void **state) {
  UNUSED(state);
  struct tau_parser *parser;

  const char *test0 = "\t// line comment\na";
  parser = tau_parser_new(__func__ , test0, strlen(test0));
  assert_true(tau_parse_line_comment(parser));
  assert_ptr_equal(parser->cur_offset, test0+16);
  assert_int_equal(parser->loc.row, 0);
  assert_int_equal(parser->loc.col, 16);
  tau_parser_free(parser);

  const char *test1 = "////\na";
  parser = tau_parser_new(__func__ , test1, strlen(test1));
  assert_true(tau_parse_line_comment(parser));
  assert_ptr_equal(parser->cur_offset, test1+4);
  assert_int_equal(parser->loc.row, 0);
  assert_int_equal(parser->loc.col, 4);
  tau_parser_free(parser);

  const char *test2 = "////";
  parser = tau_parser_new(__func__ , test2, strlen(test2));
  assert_true(tau_parse_line_comment(parser));
  assert_ptr_equal(parser->cur_offset, test2+4);
  assert_int_equal(parser->loc.row, 0);
  assert_int_equal(parser->loc.col, 4);
  tau_parser_free(parser);
}

static void test_parse_block_comment(void **state) {
  UNUSED(state);
  struct tau_parser *parser;

  const char *test0 = "/*x*/a";
  parser = tau_parser_new(__func__ , test0, strlen(test0));
  assert_true(tau_parse_block_comment(parser));
  assert_ptr_equal(parser->cur_offset, test0+5);
  assert_int_equal(parser->loc.row, 0);
  assert_int_equal(parser->loc.col, 5);
  tau_parser_free(parser);
}

static void test_parse_comment(void **state) {
  UNUSED(state);
  struct tau_parser *parser;

  const char *test0 = "/* example */// of comment";
  parser = tau_parser_new(__func__ , test0, strlen(test0));
  assert_true(tau_parse_comment(parser));
  assert_ptr_equal(parser->cur_offset, test0+13);
  assert_int_equal(parser->loc.row, 0);
  assert_int_equal(parser->loc.col, 13);

  assert_true(tau_parse_comment(parser));
  assert_ptr_equal(parser->cur_offset, test0+26);
  assert_int_equal(parser->loc.row, 0);
  assert_int_equal(parser->loc.col, 26);
  tau_parser_free(parser);
}

static void test_parse_num_literal(void **state) {
  UNUSED(state);
  struct tau_parser *parser;
  struct tau_literal *literal;

  const char *test0 = "123";
  parser = tau_parser_new(__func__ , test0, strlen(test0));
  literal = tau_parse_num_literal(parser);
  assert_non_null(literal);
  assert_int_equal(literal->type, TAU_LITERAL_NUM_INT);
  assert_int_equal(literal->as_int, 123);
  assert_ptr_equal(parser->cur_offset, test0+3);
  assert_int_equal(parser->loc.row, 0);
  assert_int_equal(parser->loc.col, 3);
  tau_parser_free(parser);
  tau_literal_free(literal);

  const char *test1 = "0b10101010";
  parser = tau_parser_new(__func__ , test1, strlen(test1));
  literal = tau_parse_num_literal(parser);
  assert_non_null(literal);
  assert_int_equal(literal->type, TAU_LITERAL_NUM_INT);
  assert_int_equal(literal->as_int, 0b10101010);
  assert_ptr_equal(parser->cur_offset, test1+10);
  assert_int_equal(parser->loc.row, 0);
  assert_int_equal(parser->loc.col, 10);
  tau_parser_free(parser);
  tau_literal_free(literal);

  const char *test2 = "0o7070";
  parser = tau_parser_new(__func__ , test2, strlen(test2));
  literal = tau_parse_num_literal(parser);
  assert_non_null(literal);
  assert_int_equal(literal->type, TAU_LITERAL_NUM_INT);
  assert_int_equal(literal->as_int, 07070);
  assert_ptr_equal(parser->cur_offset, test2+6);
  assert_int_equal(parser->loc.row, 0);
  assert_int_equal(parser->loc.col, 6);
  tau_parser_free(parser);
  tau_literal_free(literal);

  const char *test3 = "0xFAFA";
  parser = tau_parser_new(__func__ , test3, strlen(test3));
  literal = tau_parse_num_literal(parser);
  assert_non_null(literal);
  assert_int_equal(literal->type, TAU_LITERAL_NUM_INT);
  assert_int_equal(literal->as_int, 0xFAFA);
  assert_ptr_equal(parser->cur_offset, test3+6);
  assert_int_equal(parser->loc.row, 0);
  assert_int_equal(parser->loc.col, 6);
  tau_parser_free(parser);
  tau_literal_free(literal);

  const char *test4 = "0.2";
  parser = tau_parser_new(__func__ , test4, strlen(test4));
  literal = tau_parse_num_literal(parser);
  assert_non_null(literal);
  assert_int_equal(literal->type, TAU_LITERAL_NUM_FLT);
  assert_float_equal(literal->as_flt, 0.2, 0.00001);
  assert_ptr_equal(parser->cur_offset, test4+3);
  assert_int_equal(parser->loc.row, 0);
  assert_int_equal(parser->loc.col, 3);
  tau_parser_free(parser);
  tau_literal_free(literal);

  const char *test5 = "0.2e10";
  parser = tau_parser_new(__func__ , test5, strlen(test5));
  literal = tau_parse_num_literal(parser);
  assert_non_null(literal);
  assert_int_equal(literal->type, TAU_LITERAL_NUM_FLT);
  assert_float_equal(literal->as_flt, 0.2e10, 0.00001);
  assert_ptr_equal(parser->cur_offset, test5+6);
  assert_int_equal(parser->loc.row, 0);
  assert_int_equal(parser->loc.col, 6);
  tau_parser_free(parser);
  tau_literal_free(literal);
}

static void test_parse_str_literal(void **state) {
  UNUSED(state);
  struct tau_parser *parser;
  struct tau_literal *literal;

  const char *test0 = "\"str\"";
  parser = tau_parser_new(__func__ , test0, strlen(test0));
  literal = tau_parse_str_literal(parser);
  assert_non_null(literal);
  assert_int_equal(literal->type, TAU_LITERAL_STR);
  assert_string_equal(literal->as_str, "\"str\"");
  assert_ptr_equal(parser->cur_offset, test0+5);
  assert_int_equal(parser->loc.row, 0);
  assert_int_equal(parser->loc.col, 5);
  tau_parser_free(parser);
  tau_literal_free(literal);

  const char *test1 = "\"\"";
  parser = tau_parser_new(__func__ , test1, strlen(test1));
  literal = tau_parse_str_literal(parser);
  assert_non_null(literal);
  assert_int_equal(literal->type, TAU_LITERAL_STR);
  assert_string_equal(literal->as_str, "\"\"");
  assert_ptr_equal(parser->cur_offset, test1+2);
  assert_int_equal(parser->loc.row, 0);
  assert_int_equal(parser->loc.col, 2);
  tau_parser_free(parser);
  tau_literal_free(literal);

  const char *test2 = "\"\\n\"";
  parser = tau_parser_new(__func__ , test2, strlen(test2));
  literal = tau_parse_str_literal(parser);
  assert_non_null(literal);
  assert_int_equal(literal->type, TAU_LITERAL_STR);
  assert_string_equal(literal->as_str, "\"\\n\"");
  assert_ptr_equal(parser->cur_offset, test2+4);
  assert_int_equal(parser->loc.row, 0);
  assert_int_equal(parser->loc.col, 4);
  tau_parser_free(parser);
  tau_literal_free(literal);

  const char *test3 = "\"\\\"\"";
  parser = tau_parser_new(__func__ , test3, strlen(test3));
  literal = tau_parse_str_literal(parser);
  assert_non_null(literal);
  assert_int_equal(literal->type, TAU_LITERAL_STR);
  assert_string_equal(literal->as_str, "\"\\\"\"");
  assert_ptr_equal(parser->cur_offset, test3+4);
  assert_int_equal(parser->loc.row, 0);
  assert_int_equal(parser->loc.col, 4);
  tau_parser_free(parser);
  tau_literal_free(literal);
}

static void test_parse_bol_nil_or_unit_literal(void **state) {
  UNUSED(state);
  struct tau_parser *parser;
  struct tau_literal *literal;

  const char *test0 = "nil";
  parser = tau_parser_new(__func__ , test0, strlen(test0));
  literal = tau_parse_bol_nil_or_unit_literal(parser);
  assert_non_null(literal);
  assert_int_equal(literal->type, TAU_LITERAL_NIL);
  assert_ptr_equal(parser->cur_offset, test0+3);
  assert_int_equal(parser->loc.row, 0);
  assert_int_equal(parser->loc.col, 3);
  tau_parser_free(parser);
  tau_literal_free(literal);

  const char *test1 = "unit";
  parser = tau_parser_new(__func__ , test1, strlen(test1));
  literal = tau_parse_bol_nil_or_unit_literal(parser);
  assert_non_null(literal);
  assert_int_equal(literal->type, TAU_LITERAL_UNIT);
  assert_ptr_equal(parser->cur_offset, test1+4);
  assert_int_equal(parser->loc.row, 0);
  assert_int_equal(parser->loc.col, 4);
  tau_parser_free(parser);
  tau_literal_free(literal);

  const char *test2 = "true";
  parser = tau_parser_new(__func__ , test2, strlen(test2));
  literal = tau_parse_bol_nil_or_unit_literal(parser);
  assert_non_null(literal);
  assert_int_equal(literal->type, TAU_LITERAL_BOL);
  assert_true(literal->as_bol);
  assert_ptr_equal(parser->cur_offset, test2+4);
  assert_int_equal(parser->loc.row, 0);
  assert_int_equal(parser->loc.col, 4);
  tau_parser_free(parser);
  tau_literal_free(literal);

  const char *test3 = "false";
  parser = tau_parser_new(__func__ , test3, strlen(test3));
  literal = tau_parse_bol_nil_or_unit_literal(parser);
  assert_non_null(literal);
  assert_int_equal(literal->type, TAU_LITERAL_BOL);
  assert_false(literal->as_bol);
  assert_ptr_equal(parser->cur_offset, test3+5);
  assert_int_equal(parser->loc.row, 0);
  assert_int_equal(parser->loc.col, 5);
  tau_parser_free(parser);
  tau_literal_free(literal);
}

static void test_parse_literal(void **state) {
  UNUSED(state);
  struct tau_parser *parser;
  struct tau_literal *literal;

  const char *test0 = "nil 12.2 0xFA \"true\" true";
  parser = tau_parser_new(__func__ , test0, strlen(test0));
  literal = tau_parse_literal(parser);
  assert_non_null(literal);
  assert_int_equal(literal->type, TAU_LITERAL_NIL);
  assert_ptr_equal(parser->cur_offset, test0+3);
  assert_int_equal(parser->loc.row, 0);
  assert_int_equal(parser->loc.col, 3);
  tau_literal_free(literal);

  assert_true(tau_parse_spaces(parser));

  literal = tau_parse_literal(parser);
  assert_non_null(literal);
  assert_int_equal(literal->type, TAU_LITERAL_NUM_FLT);
  assert_float_equal(literal->as_flt, 12.2, 0.00001);
  assert_ptr_equal(parser->cur_offset, test0+8);
  assert_int_equal(parser->loc.row, 0);
  assert_int_equal(parser->loc.col, 8);
  tau_literal_free(literal);

  assert_true(tau_parse_spaces(parser));

  literal = tau_parse_literal(parser);
  assert_non_null(literal);
  assert_int_equal(literal->type, TAU_LITERAL_NUM_INT);
  assert_int_equal(literal->as_int, 0xFA);
  assert_ptr_equal(parser->cur_offset, test0+13);
  assert_int_equal(parser->loc.row, 0);
  assert_int_equal(parser->loc.col, 13);
  tau_literal_free(literal);

  assert_true(tau_parse_spaces(parser));

  literal = tau_parse_literal(parser);
  assert_non_null(literal);
  assert_int_equal(literal->type, TAU_LITERAL_STR);
  assert_string_equal(literal->as_str, "\"true\"");
  assert_ptr_equal(parser->cur_offset, test0+20);
  assert_int_equal(parser->loc.row, 0);
  assert_int_equal(parser->loc.col, 20);
  tau_literal_free(literal);

  assert_true(tau_parse_spaces(parser));

  literal = tau_parse_literal(parser);
  assert_non_null(literal);
  assert_int_equal(literal->type, TAU_LITERAL_BOL);
  assert_true(literal->as_bol);
  assert_ptr_equal(parser->cur_offset, test0+25);
  assert_int_equal(parser->loc.row, 0);
  assert_int_equal(parser->loc.col, 25);
  tau_literal_free(literal);

  tau_parser_free(parser);
}

static void test_parse_identifier(void **state) {
  UNUSED(state);
  assert_true(true);
}

int main() {
  UNUSED_TYPE(jmp_buf);
  UNUSED_TYPE(va_list);

  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_parse_spaces),
      cmocka_unit_test(test_parse_line_comment),
      cmocka_unit_test(test_parse_block_comment),
      cmocka_unit_test(test_parse_comment),
      cmocka_unit_test(test_parse_num_literal),
      cmocka_unit_test(test_parse_str_literal),
      cmocka_unit_test(test_parse_bol_nil_or_unit_literal),
      cmocka_unit_test(test_parse_literal),
      cmocka_unit_test(test_parse_identifier),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}