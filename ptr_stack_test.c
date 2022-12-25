//
// Created on 12/22/22.
//
// clang-format off
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
// clang-format on

#include "ptr_stack.h"

#include "common.h"

// USED TO DETECT ISSUES WITH VALGRIND
static void test_memory_safety(void **state) {
  UNUSED(state);
  struct tau_ptr_stack *stack;

  stack = tau_ptr_stack_new();
  tau_ptr_stack_free(stack);

  stack = tau_ptr_stack_new();
  assert_true(tau_ptr_stack_push(stack, (void *)0xFE0, NULL));
  tau_ptr_stack_free(stack);

  stack = tau_ptr_stack_new();
  assert_true(tau_ptr_stack_push(stack, (void *)0xFE0, NULL));
  assert_ptr_equal(tau_ptr_stack_pop(stack), (void *)0xFE0);
  tau_ptr_stack_free(stack);
}

static void test_push_pop_within_limits(void **state) {
  UNUSED(state);
  struct tau_ptr_stack *stack = tau_ptr_stack_new();
  assert_int_equal(stack->cap, TAU_PTR_STACK_CAP_INC);
  assert_true(tau_ptr_stack_is_empty(stack));

  for (int32_t i = 0; i < TAU_PTR_STACK_CAP_INC; i++) {
    size_t value = 0xF0 + i;
    assert_true(tau_ptr_stack_push(stack, (void *)value, NULL));
  }

  assert_int_equal(stack->cap, TAU_PTR_STACK_CAP_INC);
  assert_false(tau_ptr_stack_is_empty(stack));

  for (int32_t i = TAU_PTR_STACK_CAP_INC - 1; i >= 0; i--) {
    size_t expected = 0xF0 + i;
    size_t stored = (size_t)tau_ptr_stack_pop(stack);
    assert_int_equal(expected, stored);
  }

  assert_int_equal(stack->cap, TAU_PTR_STACK_CAP_INC);
  assert_true(tau_ptr_stack_is_empty(stack));
  tau_ptr_stack_free(stack);
}

static void test_push_outside_limits(void **state) {
  UNUSED(state);
  struct tau_ptr_stack *stack = tau_ptr_stack_new();
  assert_int_equal(stack->cap, TAU_PTR_STACK_CAP_INC);
  assert_true(tau_ptr_stack_is_empty(stack));

  for (int32_t i = 0; i <= TAU_PTR_STACK_CAP_INC + 5; i++) {
    size_t value = 0xF0 + i;
    assert_true(tau_ptr_stack_push(stack, (void *)value, NULL));
  }

  assert_int_equal(stack->cap, TAU_PTR_STACK_CAP_INC * 2);
  tau_ptr_stack_free(stack);
}

static void test_push_get_within_limits(void **state) {
  UNUSED(state);
  struct tau_ptr_stack *stack = tau_ptr_stack_new();
  assert_int_equal(stack->cap, TAU_PTR_STACK_CAP_INC);
  assert_true(tau_ptr_stack_is_empty(stack));

  for (int32_t i = 0; i <= TAU_PTR_STACK_CAP_INC + 5; i++) {
    size_t value = 0xF0 + i;
    assert_true(tau_ptr_stack_push(stack, (void *)value, NULL));
  }

  for (int32_t i = 0; i <= TAU_PTR_STACK_CAP_INC + 5; i++) {
    size_t expected_value = 0xF0 + i;
    assert_int_equal(expected_value, tau_ptr_stack_get(stack, i));
  }

  assert_int_equal(stack->cap, TAU_PTR_STACK_CAP_INC * 2);
  tau_ptr_stack_free(stack);
}

static void free_func_mock(void *maybe_stack) { assert_int_equal((size_t)maybe_stack, 0xB00B); }

static void test_item_free_on_stack_free(void **state) {
  UNUSED(state);

  struct tau_ptr_stack *stack = tau_ptr_stack_new();
  assert_true(tau_ptr_stack_push(stack, (void *)0xB00B, free_func_mock));
  tau_ptr_stack_free(stack);
}

int main() {
  UNUSED_TYPE(jmp_buf);
  UNUSED_TYPE(va_list);

  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_memory_safety),           cmocka_unit_test(test_push_pop_within_limits),
      cmocka_unit_test(test_push_outside_limits),     cmocka_unit_test(test_push_get_within_limits),
      cmocka_unit_test(test_item_free_on_stack_free),
  };

  return cmocka_run_group_tests(tests, NULL, NULL);
}