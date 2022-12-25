//
// Created on 12/22/22.
//

#include "ptr_stack.h"

#include <assert.h>
#include <malloc.h>
#include <memory.h>

#define ASSERT_STACK_STATE(v)                                                     \
  do {                                                                            \
    assert((v) != NULL && "stack: cannot be null");                               \
    assert((v)->ptr_bottom != NULL && "stack->ptr_bottom: cannot be null");       \
    assert((v)->free_f_bottom != NULL && "stack->free_f_bottom: cannot be null"); \
    assert((v)->cap > (v)->head && "stack->head: cannot be out capacity");        \
  } while (0)

struct tau_ptr_stack *tau_ptr_stack_new() {
  struct tau_ptr_stack *stack = calloc(1, sizeof(struct tau_ptr_stack));
  stack->cap = TAU_PTR_STACK_CAP_INC;
  stack->head = -1;
  stack->free_f_bottom = calloc(TAU_PTR_STACK_CAP_INC, sizeof(free_func_t *));
  stack->ptr_bottom = calloc(TAU_PTR_STACK_CAP_INC, sizeof(void *));
  return stack;
}

void tau_ptr_stack_free(void *maybe_stack) {
  assert(maybe_stack != NULL && "stack: cannot be null");
  struct tau_ptr_stack *stack = (struct tau_ptr_stack *)maybe_stack;

  if (stack->ptr_bottom != NULL && stack->free_f_bottom != NULL) {
    for (int32_t i = stack->head; i >= 0; i--) {
      void *item = stack->ptr_bottom[i];
      free_func_t *item_free_f = stack->free_f_bottom[i];
      if (item_free_f != NULL) {
        item_free_f(item);
      }
    }
  }

  if (stack->ptr_bottom != NULL) {
    free(stack->ptr_bottom);
  }

  if (stack->free_f_bottom != NULL) {
    free(stack->free_f_bottom);
  }

  free(stack);
}

bool tau_ptr_stack_is_empty(struct tau_ptr_stack *stack) {
  ASSERT_STACK_STATE(stack);
  return stack->head < 0;
}

bool tau_ptr_stack_push(struct tau_ptr_stack *stack, void *item, free_func_t *free_f) {
  ASSERT_STACK_STATE(stack);
  assert(item != NULL && "item: cannot be null");

  if (stack->head + 1 >= stack->cap) {
    size_t old_cap = stack->cap;
    stack->cap = stack->cap + TAU_PTR_STACK_CAP_INC;
    stack->ptr_bottom = realloc(stack->ptr_bottom, stack->cap * sizeof(void *));
    stack->free_f_bottom = realloc(stack->free_f_bottom, stack->cap * sizeof(void *));
    if (stack->ptr_bottom == NULL || stack->free_f_bottom == NULL) {
      return false;
    }

    memset(stack->ptr_bottom + old_cap, 0L, TAU_PTR_STACK_CAP_INC);
    memset(stack->free_f_bottom + old_cap, 0L, TAU_PTR_STACK_CAP_INC);
  }

  stack->head += 1;
  stack->ptr_bottom[stack->head] = item;
  stack->free_f_bottom[stack->head] = free_f;
  return true;
}

void *tau_ptr_stack_pop(struct tau_ptr_stack *stack) {
  ASSERT_STACK_STATE(stack);
  if (stack->head > -1) {
    void *data = stack->ptr_bottom[stack->head];
    stack->ptr_bottom[stack->head] = NULL;
    stack->free_f_bottom[stack->head] = NULL;
    stack->head -= 1;
    return data;
  }

  return NULL;
}

void *tau_ptr_stack_get(struct tau_ptr_stack *stack, int64_t offset) {
  ASSERT_STACK_STATE(stack);
  if (stack->head >= offset && offset > -1) {
    void *data = stack->ptr_bottom[offset];
    return data;
  }

  return NULL;
}
