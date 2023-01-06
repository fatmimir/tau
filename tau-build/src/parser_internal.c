//
// Created on 1/5/23.
//
#include "parser_internal.h"

#include <assert.h>
#include <malloc.h>

#include "log.h"
#include "parser_match.h"

// NOLINTNEXTLINE(misc-no-recursion)
void node_free(struct tau_node *node) {
  if (node->left != NULL) {
    node_free(node->left);
  }

  if (node->right != NULL) {
    node_free(node->right);
  }

  free(node);
}

struct tau_node *node_new_empty(enum tau_node_type type, struct tau_token token) {
  struct tau_node *node = calloc(1, sizeof(struct tau_node));
  node->token = token;
  node->type = type;
  return node;
}

struct tau_node *node_new_unary(enum tau_node_type type, struct tau_token token, struct tau_node *operand) {
  struct tau_node *node = calloc(1, sizeof(struct tau_node));
  node->token = token;
  node->left = operand;
  node->type = type;
  return node;
}

struct tau_node *node_new_binary(enum tau_node_type type, struct tau_token token, struct tau_node *left, struct tau_node *right) {
  struct tau_node *node = calloc(1, sizeof(struct tau_node));
  node->token = token;
  node->left = left;
  node->right = right;
  node->type = type;
  return node;
}

struct tau_node *parse_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_expr: ahead cannot be NULL");
  return parse_cast_expr(ahead);
}

struct tau_node *parse_cast_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_cast_expr: ahead cannot be NULL");
  struct tau_node *left = parse_log_or_expr(ahead);
  for (;;) {
    struct tau_token as_token = *ahead;
    if (match_and_consume(ahead, TAU_TOKEN_TYPE_KEYWORD, TAU_PUNCT_NONE, TAU_KEYWORD_AS)) {
      struct tau_node *right = parse_log_or_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      left = node_new_binary(TAU_NODE_CAST_EXPR, as_token, left, right);
      continue;
    }

    break;
  }

  return left;
handle_fail:
  if (left != NULL) {
    node_free(left);
  }

  return NULL;
}

struct tau_node *parse_log_or_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_log_or_expr: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_log_and_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_log_and_expr: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_rel_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_rel_expr: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_bit_or_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_bit_or_expr: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_bit_and_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_bit_and_expr: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_bit_shift_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_bit_shift_expr: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_fact_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_fact_expr: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_proof_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_proof_expr: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_static_lookup_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_static_lookup_expr: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_value_lookup_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_value_lookup_expr: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_unary_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_unary_expr: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_call_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_call_expr: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_index_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_index_expr: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_primary_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_primary_expr: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_atom(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_atom: ahead cannot be NULL");
  return NULL;
}

struct tau_node *passing_args(struct tau_token *ahead) {
  assert(ahead != NULL && "passing_args: ahead cannot be NULL");
  return NULL;
}

struct tau_node *index_lookup(struct tau_token *ahead) {
  assert(ahead != NULL && "index_lookup: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_return_stmt(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_return_stmt: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_continue_stmt(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_continue_stmt: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_break_stmt(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_break_stmt: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_if_stmt(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_if_stmt: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_main_case(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_main_case: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_elif_case(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_elif_case: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_else_case(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_else_case: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_while_stmt(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_while_stmt: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_type_bind(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_type_bind: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_data_bind(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_data_bind: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_statement_or_decl(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_statement_or_decl: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_block(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_block: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_let_deconstruction(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_let_deconstruction: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_proc_signature(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_proc_signature: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_formal_arg_list(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_formal_arg_list: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_formal_arg_item(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_formal_arg_item: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_proc_deconstruction(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_proc_deconstruction: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_type_deconstruction(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_type_deconstruction: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_module_decl(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_module_decl: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_let_decl(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_let_decl: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_proc_decl(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_proc_decl: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_type_decl(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_type_decl: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_decl(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_decl: ahead cannot be NULL");
  return NULL;
}

struct tau_node *parse_compilation_unit(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_compilation_unit: ahead cannot be NULL");
  return NULL;
}

