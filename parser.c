//
// Created by carlos on 1/5/23.
//

#include "parser.h"

#include <assert.h>
#include <malloc.h>

// NOLINTNEXTLINE(misc-no-recursion)
void tau_node_free(void *maybe_node) {
  assert(maybe_node != NULL && "tau_node_free: maybe_node cannot be null");
  struct tau_node *node = (struct tau_node *) maybe_node;
  if (node->left != NULL) {
    tau_node_free(node->left);
  }

  if (node->right != NULL) {
    tau_node_free(node->right);
  }

  free(node);
}

struct tau_node *tau_node_new_empty(enum tau_node_type type, struct tau_token token) {
  struct tau_node *node = calloc(1, sizeof(struct tau_node));
  node->token = token;
  node->type = type;
  return node;
}

struct tau_node *tau_node_new_unary(enum tau_node_type type, struct tau_token token, struct tau_node *operand) {
  struct tau_node *node = calloc(1, sizeof(struct tau_node));
  node->token = token;
  node->left = operand;
  node->type = type;
  return node;
}

struct tau_node *tau_node_new_binary(enum tau_node_type type, struct tau_token token, struct tau_node *left, struct tau_node *right) {
  struct tau_node *node = calloc(1, sizeof(struct tau_node));
  node->token = token;
  node->left = left;
  node->right = right;
  node->type = type;
  return node;
}
