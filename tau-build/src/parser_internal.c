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

struct tau_node *node_new_binary(enum tau_node_type type, struct tau_token token, struct tau_node *left,
                                 struct tau_node *right) {
  struct tau_node *node = calloc(1, sizeof(struct tau_node));
  node->token = token;
  node->left = left;
  node->right = right;
  node->type = type;
  return node;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_expr: ahead cannot be NULL");
  return parse_term_expr(ahead);
}

struct tau_node *parse_cast_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_cast_expr: ahead cannot be NULL");
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

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_term_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_term_expr: ahead cannot be NULL");
  struct tau_node *left = parse_fact_expr(ahead);
  for (;;) {
    struct tau_token infix_token = *ahead;
    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_PLUS, TAU_KEYWORD_NONE)) {
      struct tau_node *right = parse_fact_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      left = node_new_binary(TAU_NODE_ADD_EXPR, infix_token, left, right);
      continue;
    }

    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_HYPHEN, TAU_KEYWORD_NONE)) {
      struct tau_node *right = parse_fact_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      left = node_new_binary(TAU_NODE_SUB_EXPR, infix_token, left, right);
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

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_fact_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_fact_expr: ahead cannot be NULL");
  struct tau_node *left = parse_ref_expr(ahead);
  for (;;) {
    struct tau_token infix_token = *ahead;
    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_AST, TAU_KEYWORD_NONE)) {
      struct tau_node *right = parse_ref_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      left = node_new_binary(TAU_NODE_MUL_EXPR, infix_token, left, right);
      continue;
    }

    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_SLASH, TAU_KEYWORD_NONE)) {
      struct tau_node *right = parse_ref_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      left = node_new_binary(TAU_NODE_DIV_EXPR, infix_token, left, right);
      continue;
    }

    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_PCT, TAU_KEYWORD_NONE)) {
      struct tau_node *right = parse_ref_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      left = node_new_binary(TAU_NODE_REM_EXPR, infix_token, left, right);
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

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_ref_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_ref_expr: ahead cannot be NULL");
  struct tau_node *node = NULL;
  struct tau_node *root = NULL;
  for (;;) {
    struct tau_token unary_token = *ahead;
    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_AMP, TAU_KEYWORD_NONE)) {
      if (node == NULL) {
        node = node_new_unary(TAU_NODE_U_REF_EXPR, unary_token, NULL);
        root = node;
      } else {
        node->left = node_new_unary(TAU_NODE_U_REF_EXPR, unary_token, NULL);
        node = node->left;
      }

      continue;
    }

    break;
  }

  if (root == NULL) {
    return parse_proof_expr(ahead);
  }

  node->left = parse_proof_expr(ahead);
  return root;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_proof_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_proof_expr: ahead cannot be NULL");
  struct tau_node *left = parse_value_lookup_expr(ahead);
  for (;;) {
    struct tau_token infix_token = *ahead;
    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_COLON, TAU_KEYWORD_NONE)) {
      struct tau_node *right = parse_value_lookup_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      left = node_new_binary(TAU_NODE_PROOF_EXPR, infix_token, left, right);
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

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_value_lookup_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_value_lookup_expr: ahead cannot be NULL");
  struct tau_node *left = parse_static_lookup_expr(ahead);
  for (;;) {
    struct tau_token infix_token = *ahead;
    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_DOT, TAU_KEYWORD_NONE)) {
      struct tau_node *right = parse_static_lookup_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      left = node_new_binary(TAU_NODE_VALUE_LOOKUP_EXPR, infix_token, left, right);
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

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_static_lookup_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_static_lookup_expr: ahead cannot be NULL");
  struct tau_node *left = parse_unary_expr(ahead);
  for (;;) {
    struct tau_token infix_token = *ahead;
    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_D_COLON, TAU_KEYWORD_NONE)) {
      struct tau_node *right = parse_unary_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      left = node_new_binary(TAU_NODE_STATIC_LOOKUP_EXPR, infix_token, left, right);
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

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_unary_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_call_expr: ahead cannot be NULL");
  struct tau_node *node = NULL;
  struct tau_node *root = NULL;
  enum tau_punct matching_punct[] = {TAU_PUNCT_PLUS,  TAU_PUNCT_HYPHEN, TAU_PUNCT_BANG,
                                     TAU_PUNCT_TILDE, TAU_PUNCT_NONE};
  enum tau_node_type producing_types[] = {TAU_NODE_U_POS_EXPR, TAU_NODE_U_NEG_EXPR, TAU_NODE_U_LOG_NOT_EXPR,
                                          TAU_NODE_U_BIT_NOT_EXPR, TAU_NODE_NONE};
  for (;;) {
    bool should_continue = false;
    for (int i = 0; matching_punct[i] != TAU_PUNCT_NONE; i++) {
      struct tau_token unary_token = *ahead;
      if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, matching_punct[i], TAU_KEYWORD_NONE)) {
        if (node == NULL) {
          node = node_new_unary(producing_types[i], unary_token, NULL);
          root = node;
        } else {
          node->left = node_new_unary(producing_types[i], unary_token, NULL);
          node = node->left;
        }

        should_continue = true;
        break;
      }
    }

    if (!should_continue) {
      break;
    }
  }

  if (root == NULL) {
    return parse_call_expr(ahead);
  }

  node->left = parse_call_expr(ahead);
  return root;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_call_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_unary_expr: ahead cannot be NULL");
  struct tau_node *left = parse_index_expr(ahead);
  struct tau_node *right = NULL;

  for (;;) {
    struct tau_token index_token = *ahead;
    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_LPAR, TAU_KEYWORD_NONE)) {
      right = parse_index_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      MUST_OR_FAIL(match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_RPAR, TAU_KEYWORD_NONE), ahead,
                   "<closing `)`>");
      left = node_new_binary(TAU_NODE_CALL_EXPR, index_token, left, right);
      continue;
    }

    break;
  }

  return left;
handle_fail:
  if (left != NULL) {
    node_free(left);
  }

  if (right != NULL) {
    node_free(right);
  }

  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_index_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_index_expr: ahead cannot be NULL");
  struct tau_node *left = parse_primary_expr(ahead);
  struct tau_node *right = NULL;

  for (;;) {
    struct tau_token index_token = *ahead;
    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_LSBR, TAU_KEYWORD_NONE)) {
      right = parse_primary_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      MUST_OR_FAIL(match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_RSBR, TAU_KEYWORD_NONE), ahead,
                   "<closing `]`>");
      left = node_new_binary(TAU_NODE_INDEX_EXPR, index_token, left, right);
      continue;
    }

    break;
  }

  return left;
handle_fail:
  if (left != NULL) {
    node_free(left);
  }

  if (right != NULL) {
    node_free(right);
  }

  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_primary_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_primary_expr: ahead cannot be NULL");
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_LPAR, TAU_KEYWORD_NONE)) {
    struct tau_node *node = parse_expr(ahead);
    MUST_OR_RETURN_NULL(match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_RPAR, TAU_KEYWORD_NONE), ahead,
                        "<closing `)`>");
    return node;
  }

  return parse_atom(ahead);
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_atom(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_atom: ahead cannot be NULL");
  enum tau_token_type matching_types[] = {TAU_TOKEN_TYPE_IDENTIFIER, TAU_TOKEN_TYPE_UNI_LIT, TAU_TOKEN_TYPE_NIL_LIT,
                                          TAU_TOKEN_TYPE_BOL_LIT,    TAU_TOKEN_TYPE_INT_LIT, TAU_TOKEN_TYPE_FLT_LIT,
                                          TAU_TOKEN_TYPE_STR_LIT,    TAU_TOKEN_TYPE_NONE};
  for (int i = 0; matching_types[i] != TAU_TOKEN_TYPE_NONE; i++) {
    struct tau_token atom_token = *ahead;
    if (match_and_consume(ahead, matching_types[i], TAU_PUNCT_NONE, TAU_KEYWORD_NONE)) {
      return node_new_empty(TAU_NODE_ATOM, atom_token);
    }
  }

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
