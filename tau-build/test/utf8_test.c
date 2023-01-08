//
// Created on 12/13/22.
//
// clang-format off
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
// clang-format on

#include "../src/utf8.h"

#include "../src/common.h"

static void test_tau_enc_cp_to_bytes(void **state) {
  (void)state;

  char output[5] = {0};
  assert_int_equal(tau_enc_cp_to_bytes(0x0045, output), 1);
  assert_string_equal(output, "E");

  assert_int_equal(tau_enc_cp_to_bytes(0x01C2, output), 2);
  assert_string_equal(output, "ǂ");

  assert_int_equal(tau_enc_cp_to_bytes(0x3297, output), 3);
  assert_string_equal(output, "㊗");

  assert_int_equal(tau_enc_cp_to_bytes(0x1F60C, output), 4);
  assert_string_equal(output, "😌");
}

static void test_tau_dec_bytes_to_cp(void **state) {
  (void)state;

  uint32_t codepoint = 0L;
  assert_int_equal(tau_dec_bytes_to_cp("E\0\0\0", &codepoint), 1);
  assert_int_equal(codepoint, 0x0045);

  assert_int_equal(tau_dec_bytes_to_cp("ǂ\0\0", &codepoint), 2);
  assert_int_equal(codepoint, 0x01C2);

  assert_int_equal(tau_dec_bytes_to_cp("㊗\0", &codepoint), 3);
  assert_int_equal(codepoint, 0x3297);

  assert_int_equal(tau_dec_bytes_to_cp("😌", &codepoint), 4);
  assert_int_equal(codepoint, 0x1F60C);
}

static void test_boundary_condition(void **state) {
  UNUSED(state);
  uint32_t codepoint = 0L;

  // 2.1 First possible sequence of a certain length
  // 2.1.1 1 byte (U-00000000):
  assert_int_equal(tau_dec_bytes_to_cp("\x00\0\0\0", &codepoint), 1);
  assert_int_equal(codepoint, UNICODE_REPLACEMENT_CHARACTER);

  // 2.1.2 2 bytes (U-00000080):
  assert_int_equal(tau_dec_bytes_to_cp("\xC1\x80\0\0", &codepoint), 2);
  assert_int_equal(codepoint, UNICODE_REPLACEMENT_CHARACTER);

  // 2.1.3 3 bytes (U-00000800):
  assert_int_equal(tau_dec_bytes_to_cp("\xE0\xA0\x80\0", &codepoint), 3);
  assert_int_equal(codepoint, UNICODE_REPLACEMENT_CHARACTER);

  // 2.1.4 4 bytes (U-00010000):
  assert_int_equal(tau_dec_bytes_to_cp("\xF0\x90\x80\x80", &codepoint), 4);
  assert_int_equal(codepoint, UNICODE_REPLACEMENT_CHARACTER);

  // 2.1.5 5 bytes (U-00200000):
  assert_int_equal(tau_dec_bytes_to_cp("\xF8\x88\x80\x80\x80", &codepoint), 5);
  assert_int_equal(codepoint, UNICODE_REPLACEMENT_CHARACTER);

  // 2.1.6 6 bytes (U-04000000):
  assert_int_equal(tau_dec_bytes_to_cp("\xFC\x84\x80\x80\x80\x80", &codepoint), 6);
  assert_int_equal(codepoint, UNICODE_REPLACEMENT_CHARACTER);

  // 2.2 Last possible sequence of a certain length
  // 2.2.1 1 byte  (U-0000007F):
  assert_int_equal(tau_dec_bytes_to_cp("\x7F\0\0", &codepoint), 1);
  assert_int_equal(codepoint, UNICODE_REPLACEMENT_CHARACTER);

  // 2.2.2  2 bytes (U-000007FF):
  assert_int_equal(tau_dec_bytes_to_cp("\xDF\xBF\0", &codepoint), 2);
  assert_int_equal(codepoint, UNICODE_REPLACEMENT_CHARACTER);

  // 2.2.3  3 bytes (U-0000FFFF):
  assert_int_equal(tau_dec_bytes_to_cp("\xEF\xBF\xBF", &codepoint), 3);
  assert_int_equal(codepoint, UNICODE_REPLACEMENT_CHARACTER);

  // 2.2.4  4 bytes (U-001FFFFF):
  assert_int_equal(tau_dec_bytes_to_cp("\xF7\xBF\xBF\xBF", &codepoint), 4);
  assert_int_equal(codepoint, UNICODE_REPLACEMENT_CHARACTER);

  // 2.2.5  5 bytes (U-03FFFFFF):
  assert_int_equal(tau_dec_bytes_to_cp("\xFB\xBF\xBF\xBF\xBF", &codepoint), 5);
  assert_int_equal(codepoint, UNICODE_REPLACEMENT_CHARACTER);

  // 2.2.6  6 bytes (U-7FFFFFFF):
  assert_int_equal(tau_dec_bytes_to_cp("\xFD\xBF\xBF\xBF\xBF\xBF", &codepoint), 6);
  assert_int_equal(codepoint, UNICODE_REPLACEMENT_CHARACTER);

  // 2.3 Other boundary conditions
  // 2.3.1  U-0000D7FF = ed 9f bf
  assert_int_equal(tau_dec_bytes_to_cp("\xED\x9F\xBF", &codepoint), 3);
  assert_int_equal(codepoint, 0xD7FF);  // NOTE(cedmundo): Not really searching for non-defined unicode characters
  // assert_int_equal(codepoint, UNICODE_REPLACEMENT_CHARACTER);

  // 2.3.2  U-0000E000 = ee 80 80
  assert_int_equal(tau_dec_bytes_to_cp("\xEE\x80\x80", &codepoint), 3);
  assert_int_equal(codepoint, 0xE000);  // NOTE(cedmundo): Not really searching for non-defined unicode characters
  // assert_int_equal(codepoint, UNICODE_REPLACEMENT_CHARACTER);

  // 2.3.3  U-0000FFFD = ef bf bd
  assert_int_equal(tau_dec_bytes_to_cp("\xEF\xBF\xBD", &codepoint), 3);
  assert_int_equal(codepoint, UNICODE_REPLACEMENT_CHARACTER);

  // 2.3.4  U-0010FFFF = f4 8f bf bf
  assert_int_equal(tau_dec_bytes_to_cp("\xF4\x8F\xBF\xBF", &codepoint), 4);
  assert_int_equal(codepoint, UNICODE_REPLACEMENT_CHARACTER);

  // 2.3.5  U-00110000 = f4 90 80 80
  assert_int_equal(tau_dec_bytes_to_cp("\xF4\x90\x80\x80", &codepoint), 4);
  assert_int_equal(codepoint, UNICODE_REPLACEMENT_CHARACTER);
}

static void test_malformed_sequences(void **state) {
  UNUSED(state);
  // 3.1 Unexpected continuation bytes
  // 3.2 Lonely start characters
  // 3.3 Sequences with last continuation byte missing
  // 3.4 Concatenation of incomplete sequences
  // 3.5 Impossible bytes
}

static void test_overlong_sequences(void **state) {
  UNUSED(state);
  // 4.1 Examples of an overlong ASCII character
  // 4.2 Maximum overlong sequences
  // 4.3 Overlong representation of NUL character
}

static void test_illegal_code_positions(void **state) {
  UNUSED(state);
  // 5.1 Single UTF-16 surrogates
  // 5.2 Paired UTF-16 surrogates
  // 5.3 Non-character code positions
}

int main() {
  UNUSED_TYPE(jmp_buf);
  UNUSED_TYPE(va_list);
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_tau_enc_cp_to_bytes), cmocka_unit_test(test_tau_dec_bytes_to_cp),
      cmocka_unit_test(test_boundary_condition),  cmocka_unit_test(test_malformed_sequences),
      cmocka_unit_test(test_overlong_sequences),  cmocka_unit_test(test_illegal_code_positions),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}