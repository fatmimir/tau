//
// Created on 12/22/22.
//

#ifndef TAU_PTR_STACK_H
#define TAU_PTR_STACK_H

#include <stdbool.h>
#include <stdint.h>

#include "common.h"

struct tau_ptr_stack {
  free_func_t **free_f_bottom;
  void **ptr_bottom;
  int32_t head;
  int32_t cap;
};

struct tau_ptr_stack *tau_ptr_stack_new();
void tau_ptr_stack_free(void *maybe_stack);
bool tau_ptr_stack_is_empty(struct tau_ptr_stack *stack);
bool tau_ptr_stack_push(struct tau_ptr_stack *stack, void *item, free_func_t *free_f);
void *tau_ptr_stack_pop(struct tau_ptr_stack *stack);

#define TAU_PTR_STACK_CAP_INC 10
#endif  // TAU_PTR_STACK_H
