//
// Created by carlos on 1/5/23.
//

#ifndef TAU_PARSER_H
#define TAU_PARSER_H

#include <stddef.h>
#include "lexer.h"

enum tau_node_type {
  TAU_NODE_NONE,
  TAU_NODE_COUNT,
};

struct tau_node {
  struct tau_token token;
  struct tau_node *left;
  struct tau_node *right;
  enum tau_node_type type;
};

void tau_node_free(void *maybe_node);
struct tau_node *tau_node_new_empty(enum tau_node_type type, struct tau_token token);
struct tau_node *tau_node_new_unary(enum tau_node_type type, struct tau_token token, struct tau_node *operand);
struct tau_node *tau_node_new_binary(enum tau_node_type type, struct tau_token token, struct tau_node *left, struct tau_node *right);

#endif  // TAU_PARSER_H
