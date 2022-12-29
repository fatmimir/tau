//
// Created on 12/29/22.
//

#ifndef TAU_PARSE_ALLOC_H
#define TAU_PARSE_ALLOC_H

struct tau_anode *new_anode(enum tau_anode_type type, struct tau_token token) {
  struct tau_anode *node = calloc(1, sizeof(struct tau_anode));
  node->type = type;
  node->token = token;
  return node;
}

struct tau_anode *new_anode_unary(enum tau_anode_type type, struct tau_token token, struct tau_anode *operand) {
  struct tau_anode *node = calloc(1, sizeof(struct tau_anode));
  node->type = type;
  node->token = token;
  node->left = operand;
  return node;
}

struct tau_anode *new_anode_binary(enum tau_anode_type type, struct tau_token token, struct tau_anode *left,
                                   struct tau_anode *right) {
  struct tau_anode *node = calloc(1, sizeof(struct tau_anode));
  node->type = type;
  node->token = token;
  node->left = left;
  node->right = right;
  return node;
}

struct tau_anode *new_anode_stack(enum tau_anode_type type, struct tau_token token) {
  struct tau_anode *node = calloc(1, sizeof(struct tau_anode));
  node->type = type;
  node->token = token;
  node->stack = tau_ptr_stack_new();
  return node;
}

#endif  // TAU_PARSE_ALLOC_H
