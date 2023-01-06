// clang-format off
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
// clang-format on

#include "common.h"

static void test_example(void **state) {
  UNUSED(state);
  assert_true(1);
}

int main() {
  UNUSED_TYPE(jmp_buf);
  UNUSED_TYPE(va_list);

  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_example),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}
