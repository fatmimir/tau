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
  return parse_log_or_expr(ahead);
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

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_log_or_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_log_or_expr: ahead cannot be NULL");
  struct tau_node *left = parse_log_and_expr(ahead);
  for (;;) {
    struct tau_token infix_token = *ahead;
    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_D_PIPE, TAU_KEYWORD_NONE)) {
      struct tau_node *right = parse_log_and_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      left = node_new_binary(TAU_NODE_LOG_OR_EXPR, infix_token, left, right);
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
struct tau_node *parse_log_and_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_log_and_expr: ahead cannot be NULL");
  struct tau_node *left = parse_rel_expr(ahead);
  for (;;) {
    struct tau_token infix_token = *ahead;
    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_D_AMP, TAU_KEYWORD_NONE)) {
      struct tau_node *right = parse_rel_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      left = node_new_binary(TAU_NODE_LOG_AND_EXPR, infix_token, left, right);
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
struct tau_node *parse_rel_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_rel_expr: ahead cannot be NULL");
  struct tau_node *left = parse_cmp_expr(ahead);
  for (;;) {
    struct tau_token infix_token = *ahead;
    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_D_EQ, TAU_KEYWORD_NONE)) {
      struct tau_node *right = parse_cmp_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      left = node_new_binary(TAU_NODE_EQ_EXPR, infix_token, left, right);
      continue;
    }

    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_BANG_EQ, TAU_KEYWORD_NONE)) {
      struct tau_node *right = parse_cmp_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      left = node_new_binary(TAU_NODE_NE_EXPR, infix_token, left, right);
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
struct tau_node *parse_cmp_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_cmp_expr: ahead cannot be NULL");
  struct tau_node *left = parse_bit_or_expr(ahead);
  enum tau_punct matching_punct[] = {TAU_PUNCT_LT, TAU_PUNCT_LT_EQ, TAU_PUNCT_GT, TAU_PUNCT_GT_EQ, TAU_PUNCT_NONE};
  enum tau_node_type producing_types[] = {TAU_NODE_LT_EXPR, TAU_NODE_LE_EXPR, TAU_NODE_GT_EXPR, TAU_NODE_GE_EXPR,
                                          TAU_NODE_NONE};

  for (;;) {
    bool should_continue = false;
    for (int i = 0; matching_punct[i] != TAU_PUNCT_NONE; i++) {
      struct tau_token infix_token = *ahead;
      if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, matching_punct[i], TAU_KEYWORD_NONE)) {
        struct tau_node *right = parse_bit_or_expr(ahead);
        MUST_OR_FAIL(right, ahead, "<expression>");
        left = node_new_binary(producing_types[i], infix_token, left, right);
        should_continue = true;
        break;
      }
    }

    if (!should_continue) {
      break;
    }
  }
  return left;
handle_fail:
  if (left != NULL) {
    node_free(left);
  }
  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_bit_or_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_bit_or_expr: ahead cannot be NULL");
  struct tau_node *left = parse_bit_and_expr(ahead);
  for (;;) {
    struct tau_token infix_token = *ahead;
    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_PIPE, TAU_KEYWORD_NONE)) {
      struct tau_node *right = parse_bit_and_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      left = node_new_binary(TAU_NODE_BIT_OR_EXPR, infix_token, left, right);
      continue;
    }

    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_CIRC, TAU_KEYWORD_NONE)) {
      struct tau_node *right = parse_bit_and_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      left = node_new_binary(TAU_NODE_BIT_XOR_EXPR, infix_token, left, right);
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
struct tau_node *parse_bit_and_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_bit_and_expr: ahead cannot be NULL");
  struct tau_node *left = parse_bit_shift_expr(ahead);
  for (;;) {
    struct tau_token infix_token = *ahead;
    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_AMP, TAU_KEYWORD_NONE)) {
      struct tau_node *right = parse_bit_shift_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      left = node_new_binary(TAU_NODE_BIT_AND_EXPR, infix_token, left, right);
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
struct tau_node *parse_bit_shift_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_bit_shift_expr: ahead cannot be NULL");
  struct tau_node *left = parse_term_expr(ahead);
  for (;;) {
    struct tau_token infix_token = *ahead;
    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_D_LT, TAU_KEYWORD_NONE)) {
      struct tau_node *right = parse_term_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      left = node_new_binary(TAU_NODE_LSH_EXPR, infix_token, left, right);
      continue;
    }

    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_D_GT, TAU_KEYWORD_NONE)) {
      struct tau_node *right = parse_term_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      left = node_new_binary(TAU_NODE_RSH_EXPR, infix_token, left, right);
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
  assert(ahead != NULL && "parse_unary_expr: ahead cannot be NULL");
  struct tau_node *node = NULL;
  struct tau_node *root = NULL;
  enum tau_punct matching_punct[] = {TAU_PUNCT_PLUS, TAU_PUNCT_HYPHEN, TAU_PUNCT_BANG, TAU_PUNCT_TILDE, TAU_PUNCT_NONE};
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
  assert(ahead != NULL && "parse_call_expr: ahead cannot be NULL");
  struct tau_node *left = parse_index_expr(ahead);
  struct tau_node *right = NULL;

  for (;;) {
    struct tau_token call_token = *ahead;
    right = parse_passing_args(ahead);
    if (right != NULL) {
      left = node_new_binary(TAU_NODE_CALL_EXPR, call_token, left, right);
      continue;
    }

    break;
  }

  return left;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_index_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_index_expr: ahead cannot be NULL");
  struct tau_node *left = parse_primary_expr(ahead);
  struct tau_node *right = NULL;

  for (;;) {
    struct tau_token index_token = *ahead;
    right = parse_index_lookup(ahead);
    if (right != NULL) {
      left = node_new_binary(TAU_NODE_INDEX_EXPR, index_token, left, right);
      continue;
    }

    break;
  }

  return left;
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

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_passing_args(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_passing_args: ahead cannot be NULL");
  struct tau_node *root = NULL;
  struct tau_node *node = NULL;
  struct tau_token call_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_LPAR, TAU_KEYWORD_NONE)) {
    for (;;) {
      struct tau_token arg_token = *ahead;
      struct tau_node *arg = parse_expr(ahead);
      if (arg != NULL) {
        if (root == NULL) {
          node = node_new_unary(TAU_NODE_PASSING_ARG, call_token, arg);
          root = node;
        } else {
          node->right = node_new_unary(TAU_NODE_PASSING_ARG, arg_token, arg);
          node = node->right;
        }

        if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_COMMA, TAU_KEYWORD_NONE)) {
          continue;
        }
      }

      break;
    }
    MUST_OR_FAIL(match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_RPAR, TAU_KEYWORD_NONE), ahead,
                 "<closing `)`>");
    return root;
  }

handle_fail:
  if (root != NULL) {
    node_free(root);
  }

  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_index_lookup(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_index_lookup: ahead cannot be NULL");
  struct tau_node *root = NULL;
  struct tau_node *node = NULL;
  struct tau_token index_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_LSBR, TAU_KEYWORD_NONE)) {
    for (;;) {
      struct tau_token arg_token = *ahead;
      struct tau_node *arg = parse_expr(ahead);
      if (arg != NULL) {
        if (root == NULL) {
          node = node_new_unary(TAU_NODE_PASSING_INDEX, index_token, arg);
          root = node;
        } else {
          node->right = node_new_unary(TAU_NODE_PASSING_INDEX, arg_token, arg);
          node = node->right;
        }

        if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_COMMA, TAU_KEYWORD_NONE)) {
          continue;
        }
      }

      break;
    }
    MUST_OR_FAIL(match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_RSBR, TAU_KEYWORD_NONE), ahead,
                 "<closing `]`>");
    return root;
  }

handle_fail:
  if (root != NULL) {
    node_free(root);
  }

  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_return_stmt(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_return_stmt: ahead cannot be NULL");
  struct tau_token stmt_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_KEYWORD, TAU_PUNCT_NONE, TAU_KEYWORD_RETURN)) {
    struct tau_node *stmt_node = node_new_empty(TAU_NODE_RETURN_STMT, stmt_token);
    stmt_node->left = parse_expr(ahead);
    return stmt_node;
  }

  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_continue_stmt(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_continue_stmt: ahead cannot be NULL");
  struct tau_token stmt_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_KEYWORD, TAU_PUNCT_NONE, TAU_KEYWORD_CONTINUE)) {
    return node_new_empty(TAU_NODE_CONTINUE_STMT, stmt_token);
  }

  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_break_stmt(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_break_stmt: ahead cannot be NULL");
  struct tau_token stmt_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_KEYWORD, TAU_PUNCT_NONE, TAU_KEYWORD_BREAK)) {
    return node_new_empty(TAU_NODE_BREAK_STMT, stmt_token);
  }

  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_if_stmt(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_if_stmt: ahead cannot be NULL");
  struct tau_node *main_branch = NULL;
  struct tau_token if_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_KEYWORD, TAU_PUNCT_NONE, TAU_KEYWORD_IF)) {
    main_branch = parse_main_branch(ahead);
    MUST_OR_FAIL(main_branch, ahead, "<main branch>");

    struct tau_node *attach_to = main_branch;
    for (;;) {
      struct tau_node *elif_branch = parse_elif_branch(ahead);
      if (elif_branch != NULL) {
        attach_to->right = elif_branch;
        attach_to = attach_to->right;
        continue;
      }

      break;
    }

    struct tau_node *if_stmt = node_new_unary(TAU_NODE_IF_STMT, if_token, main_branch);
    struct tau_node *else_branch = parse_else_branch(ahead);
    if (else_branch != NULL) {
      if_stmt->right = else_branch;
    }

    return if_stmt;
  }

handle_fail:
  if (main_branch != NULL) {
    node_free(main_branch);
  }

  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_main_branch(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_main_branch: ahead cannot be NULL");
  return node_new_unary(TAU_NODE_MAIN_BRANCH, *ahead, parse_expr_with_block(ahead));
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_elif_branch(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_elif_branch: ahead cannot be NULL");
  struct tau_token elif_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_KEYWORD, TAU_PUNCT_NONE, TAU_KEYWORD_ELIF)) {
    struct tau_node *expr_branch = parse_expr_with_block(ahead);
    MUST_OR_RETURN_NULL(expr_branch, ahead, "<elif branch>");
    return node_new_unary(TAU_NODE_ELIF_BRANCH, elif_token, expr_branch);
  }

  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_else_branch(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_else_branch: ahead cannot be NULL");
  struct tau_token else_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_KEYWORD, TAU_PUNCT_NONE, TAU_KEYWORD_ELSE)) {
    struct tau_node *block = parse_block(ahead);
    MUST_OR_RETURN_NULL(block, ahead, "<else branch>");
    return node_new_unary(TAU_NODE_ELSE_BRANCH, else_token, block);
  }

  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_while_stmt(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_while_stmt: ahead cannot be NULL");
  struct tau_token while_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_KEYWORD, TAU_PUNCT_NONE, TAU_KEYWORD_WHILE)) {
    struct tau_node *expr_with_block = parse_expr_with_block(ahead);
    MUST_OR_RETURN_NULL(expr_with_block, ahead, "<while branch>");
    return node_new_unary(TAU_NODE_WHILE_STMT, while_token, expr_with_block);
  }

  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_statement_or_decl(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_statement_or_decl: ahead cannot be NULL");
  struct tau_node *node = NULL;
  static const parser_func_t try_parsers[] = {parse_return_stmt, parse_continue_stmt, parse_break_stmt,
                                              parse_if_stmt,     parse_while_stmt,    parse_let_decl,
                                              parse_proc_decl,   parse_type_decl,     NULL};

  for (int i = 0; try_parsers[i] != NULL; i++) {
    node = try_parsers[i](ahead);
    if (node != NULL) {
      return node_new_unary(TAU_NODE_STATEMENT_OR_DECL, node->token, node);
    }
  }

  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_block(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_block: ahead cannot be NULL");
  struct tau_token block_token = *ahead;
  struct tau_node *root = NULL;
  struct tau_node *attach_to = NULL;

  if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_LCBR, TAU_KEYWORD_NONE)) {
    root = node_new_empty(TAU_NODE_BLOCK, block_token);
    for (;;) {
      struct tau_node *statement_or_decl = parse_statement_or_decl(ahead);
      if (statement_or_decl != NULL) {
        if (attach_to == NULL) {
          root->left = statement_or_decl;
          attach_to = root->left;
        } else {
          attach_to->right = statement_or_decl;
          attach_to = attach_to->right;
        }

        MUST_OR_FAIL(match_and_consume(ahead, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE), ahead,
                     "<end of line>");
        continue;
      }

      break;
    }

    MUST_OR_FAIL(match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_RCBR, TAU_KEYWORD_NONE), ahead,
                 "<closing `}`>");
  }

  return root;

handle_fail:
  if (root != NULL) {
    node_free(root);
  }

  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_expr_with_block(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_expr_with_block: ahead cannot be NULL");
  struct tau_node *expr = parse_expr(ahead);
  MUST_OR_RETURN_NULL(expr, ahead, "<expression>");

  struct tau_node *block = parse_block(ahead);
  MUST_OR_RETURN_NULL(block, ahead, "<block>");
  return node_new_binary(TAU_NODE_EXPR_WITH_BLOCK, expr->token, expr, block);
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_type_bind(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_type_bind: ahead cannot be NULL");
  struct tau_token bind_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_COLON, TAU_KEYWORD_NONE)) {
    struct tau_node *expr = parse_expr(ahead);
    return node_new_unary(TAU_NODE_TYPE_BIND, bind_token, expr);
  }

  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_data_bind(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_data_bind: ahead cannot be NULL");
  struct tau_token bind_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_EQ, TAU_KEYWORD_NONE)) {
    struct tau_node *expr = parse_expr(ahead);
    return node_new_unary(TAU_NODE_DATA_BIND, bind_token, expr);
  }

  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_let_deconstruction(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_let_deconstruction: ahead cannot be NULL");
  struct tau_node *identifier = NULL;
  struct tau_node *type_bind = NULL;

  struct tau_token decons_token = *ahead;
  identifier = parse_atom(ahead);
  MUST_OR_FAIL(identifier, ahead, "<identifier>");
  MUST_OR_FAIL(identifier->token.type == TAU_TOKEN_TYPE_IDENTIFIER, ahead, "<identifier>");

  type_bind = parse_type_bind(ahead);
  MUST_OR_FAIL(type_bind, ahead, "<type bind>");

  return node_new_binary(TAU_NODE_LET_DECONSTRUCTION, decons_token, identifier, type_bind);
handle_fail:
  if (identifier != NULL) {
    node_free(identifier);
  }

  if (type_bind != NULL) {
    node_free(type_bind);
  }
  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_proc_signature(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_proc_signature: ahead cannot be NULL");

  struct tau_node *formal_args = NULL;
  struct tau_node *type_bind = NULL;

  struct tau_token signature_token = *ahead;
  formal_args = parse_formal_args(ahead);
  MUST_OR_FAIL(formal_args, ahead, "<formal args>");

  type_bind = parse_type_bind(ahead);
  MUST_OR_FAIL(type_bind, ahead, "<type bind>");

  return node_new_binary(TAU_NODE_PROC_SIGNATURE, signature_token, formal_args, type_bind);
handle_fail:
  if (formal_args != NULL) {
    node_free(formal_args);
  }

  if (type_bind != NULL) {
    node_free(type_bind);
  }
  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_formal_args(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_formal_args: ahead cannot be NULL");
  struct tau_node *initial = NULL;
  struct tau_node *attach_to = NULL;
  struct tau_token formal_args_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_LPAR, TAU_KEYWORD_NONE)) {
    for (;;) {
      struct tau_node *arg = parse_formal_arg(ahead);
      if (arg != NULL) {
        if (initial == NULL) {
          initial = arg;
          attach_to = arg;
        } else {
          attach_to->right = arg;
          attach_to = attach_to->right;
        }

        if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_COMMA, TAU_KEYWORD_NONE)) {
          continue;
        }
      }

      break;
    }
    MUST_OR_FAIL(match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_RPAR, TAU_KEYWORD_NONE), ahead,
                 "<closing `)`>");

    return node_new_unary(TAU_NODE_FORMAL_ARGS, formal_args_token, initial);
  }

handle_fail:
  if (initial != NULL) {
    node_free(initial);
  }

  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_formal_arg(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_formal_arg: ahead cannot be NULL");
  return node_new_unary(TAU_NODE_FORMAL_ARG, *ahead, parse_arg_bind(ahead));
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_arg_bind(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_arg_bind: ahead cannot be NULL");
  struct tau_node *identifier = NULL;
  struct tau_node *type_bind = NULL;

  struct tau_token formal_arg_token = *ahead;
  identifier = parse_atom(ahead);
  if (identifier) {
    type_bind = parse_type_bind(ahead);
    MUST_OR_FAIL(type_bind, ahead, "<type bind>");

    return node_new_binary(TAU_NODE_ARG_BIND, formal_arg_token, identifier, type_bind);
  }

  return NULL;
handle_fail:
  if (identifier != NULL) {
    node_free(identifier);
  }

  if (type_bind != NULL) {
    node_free(type_bind);
  }
  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_proc_deconstruction(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_proc_deconstruction: ahead cannot be NULL");
  struct tau_node *identifier = NULL;
  struct tau_node *signature = NULL;

  struct tau_token decons_token = *ahead;
  identifier = parse_atom(ahead);
  MUST_OR_FAIL(identifier, ahead, "<identifier>");
  MUST_OR_FAIL(identifier->token.type == TAU_TOKEN_TYPE_IDENTIFIER, ahead, "<identifier>");

  signature = parse_proc_signature(ahead);
  MUST_OR_FAIL(signature, ahead, "<proc signature>");

  return node_new_binary(TAU_NODE_PROC_DECONSTRUCTION, decons_token, identifier, signature);
handle_fail:
  if (identifier != NULL) {
    node_free(identifier);
  }

  if (signature != NULL) {
    node_free(signature);
  }
  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_type_deconstruction(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_type_deconstruction: ahead cannot be NULL");
  struct tau_node *identifier = NULL;

  struct tau_token decons_token = *ahead;
  identifier = parse_atom(ahead);
  MUST_OR_RETURN_NULL(identifier, ahead, "<identifier>");
  MUST_OR_RETURN_NULL(identifier->token.type == TAU_TOKEN_TYPE_IDENTIFIER, ahead, "<identifier>");

  return node_new_unary(TAU_NODE_TYPE_DECONSTRUCTION, decons_token, identifier);
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_module_decl(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_module_decl: ahead cannot be NULL");
  struct tau_token module_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_KEYWORD, TAU_PUNCT_NONE, TAU_KEYWORD_MODULE)) {
    struct tau_node *static_lookup_expr = parse_static_lookup_expr(ahead);
    MUST_OR_RETURN_NULL(static_lookup_expr, ahead, "<static lookup>");
    return node_new_unary(TAU_NODE_MODULE_DECL, module_token, static_lookup_expr);
  }

  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_prototype_suffix(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_module_decl: ahead cannot be NULL");
  struct tau_token prototype_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_KEYWORD, TAU_PUNCT_NONE, TAU_KEYWORD_PROTOTYPE)) {
    return node_new_empty(TAU_NODE_PROTOTYPE_SUFFIX, prototype_token);
  }

  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_let_decl(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_let_decl: ahead cannot be NULL");
  struct tau_node *deconstruction = NULL;
  struct tau_node *data_bind = NULL;

  struct tau_token let_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_KEYWORD, TAU_PUNCT_NONE, TAU_KEYWORD_LET)) {
    deconstruction = parse_let_deconstruction(ahead);
    data_bind = parse_prototype_suffix(ahead);
    if (data_bind == NULL) {
      data_bind = parse_data_bind(ahead);
    }
    MUST_OR_FAIL(data_bind, ahead, "<prototype or data bind>");

    return node_new_binary(TAU_NODE_LET_DECL, let_token, deconstruction, data_bind);
  }

  return NULL;
handle_fail:
  if (deconstruction != NULL) {
    node_free(deconstruction);
  }

  if (data_bind != NULL) {
    node_free(data_bind);
  }

  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_proc_decl(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_proc_decl: ahead cannot be NULL");
  struct tau_node *deconstruction = NULL;
  struct tau_node *data_bind_or_block = NULL;

  struct tau_token proc_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_KEYWORD, TAU_PUNCT_NONE, TAU_KEYWORD_PROC)) {
    deconstruction = parse_proc_deconstruction(ahead);
    data_bind_or_block = parse_prototype_suffix(ahead);
    if (data_bind_or_block == NULL) {
      data_bind_or_block = parse_data_bind(ahead);
    }

    if (data_bind_or_block == NULL) {
      data_bind_or_block = parse_block(ahead);
    }

    MUST_OR_FAIL(data_bind_or_block, ahead, "<prototype, block or data bind>");

    return node_new_binary(TAU_NODE_PROC_DECL, proc_token, deconstruction, data_bind_or_block);
  }

  return NULL;
handle_fail:
  if (deconstruction != NULL) {
    node_free(deconstruction);
  }

  if (data_bind_or_block != NULL) {
    node_free(data_bind_or_block);
  }

  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_type_decl(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_type_decl: ahead cannot be NULL");
  struct tau_node *deconstruction = NULL;
  struct tau_node *data_bind = NULL;

  struct tau_token type_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_KEYWORD, TAU_PUNCT_NONE, TAU_KEYWORD_TYPE)) {
    deconstruction = parse_type_deconstruction(ahead);
    data_bind = parse_prototype_suffix(ahead);
    if (data_bind == NULL) {
      data_bind = parse_data_bind(ahead);
    }
    MUST_OR_FAIL(data_bind, ahead, "<prototype or data bind>");

    return node_new_binary(TAU_NODE_TYPE_DECL, type_token, deconstruction, data_bind);
  }

  return NULL;
handle_fail:
  if (deconstruction != NULL) {
    node_free(deconstruction);
  }

  if (data_bind != NULL) {
    node_free(data_bind);
  }

  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_extern_decl(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_extern_decl: ahead cannot be NULL");
  struct tau_token extern_token = *ahead;
  struct tau_node *decl = NULL;

  if (match_and_consume(ahead, TAU_TOKEN_TYPE_KEYWORD, TAU_PUNCT_NONE, TAU_KEYWORD_EXTERN)) {
    decl = parse_let_decl(ahead);
    if (decl != NULL) {
      return node_new_unary(TAU_NODE_EXTERN_DECL, extern_token, decl);
    }

    decl = parse_proc_decl(ahead);
    if (decl != NULL) {
      return node_new_unary(TAU_NODE_EXTERN_DECL, extern_token, decl);
    }

    decl = parse_type_decl(ahead);
    if (decl != NULL) {
      return node_new_unary(TAU_NODE_EXTERN_DECL, extern_token, decl);
    }

    MUST_OR_RETURN_NULL(decl, ahead, "<let, proc or type decl>");
  }

  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_decl(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_decl: ahead cannot be NULL");
  struct tau_node *node = NULL;
  static const parser_func_t try_parsers[] = {parse_let_decl, parse_proc_decl, parse_type_decl, parse_extern_decl,
                                              NULL};

  for (int i = 0; try_parsers[i] != NULL; i++) {
    node = try_parsers[i](ahead);
    if (node != NULL) {
      return node_new_unary(TAU_NODE_DECL, node->token, node);
    }
  }

  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_decls(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_decls: ahead cannot be NULL");
  struct tau_token start_token = *ahead;
  struct tau_node *root = NULL;
  struct tau_node *attach_to = NULL;

  root = node_new_empty(TAU_NODE_DECLS, start_token);
  for (;;) {
    struct tau_node *decl = parse_decl(ahead);
    if (decl != NULL) {
      if (attach_to == NULL) {
        root->left = decl;
        attach_to = root->left;
      } else {
        attach_to->right = decl;
        attach_to = attach_to->right;
      }

      MUST_OR_FAIL(match_and_consume(ahead, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE), ahead,
                   "<end of line>");
      continue;
    }

    break;
  }

  return root;
handle_fail:
  if (root != NULL) {
    node_free(root);
  }

  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
struct tau_node *parse_compilation_unit(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_compilation_unit: ahead cannot be NULL");
  struct tau_node *module_decl = NULL;
  struct tau_node *decls =  NULL;

  struct tau_token start_token = *ahead;
  module_decl = parse_module_decl(ahead);
  MUST_OR_FAIL(module_decl, ahead, "<module decl>");
  MUST_OR_FAIL(match_and_consume(ahead, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE), ahead,
               "<end of line>");
  decls = parse_decls(ahead);
  return node_new_binary(TAU_NODE_COMPILATION_UNIT, start_token, module_decl, decls);
handle_fail:
  if (module_decl != NULL) {
    node_free(module_decl);
  }

  if (decls != NULL) {
    node_free(decls);
  }

  return NULL;
}
