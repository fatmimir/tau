//
// Created on 12/21/22.
//

#ifndef TAU_PARSER_H
#define TAU_PARSER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "common.h"
#include "lexer.h"
#include "ptr_stack.h"

struct tau_parser {
  struct tau_loc loc;
  struct tau_token cur;
};

struct tau_parser *tau_parser_new();
void tau_parser_free(void *maybe_parser);

#endif  // TAU_PARSER_H
