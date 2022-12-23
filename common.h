//
// Created on 12/21/22.
//

#ifndef TAU_COMMON_H
#define TAU_COMMON_H

#include <stddef.h>

struct tau_loc {
  const char *buf_name;
  size_t row;
  size_t col;
};

typedef void(free_func_t)(void *);

#define UNUSED(x) ((void)x)
#define UNUSED_TYPE(x) ((void *)(x *)0)

#endif  // TAU_COMMON_H
