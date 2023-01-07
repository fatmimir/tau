//
// Created on 1/5/23.
//

#ifndef TAU_TOPOLOGY_HELPER_H
#define TAU_TOPOLOGY_HELPER_H

#include <malloc.h>
#include <string.h>

#include "../src/log.h"
#include "../src/parser_internal.h"
#include "../src/parser_match.h"
#define HANDLED_IDENTIFIER_TO_NODE_TYPE 33

static enum tau_node_type identifier_to_node_type(const char *name, size_t len) {
  const char *anode_names[TAU_NODE_COUNT] = {
      [TAU_NODE_NONE] = "NONE",
      [TAU_NODE_PASSING_ARG] = "PASSING_ARG",
      [TAU_NODE_PASSING_INDEX] = "PASSING_INDEX",
      [TAU_NODE_CAST_EXPR] = "CAST_EXPR",
      [TAU_NODE_LOG_OR_EXPR] = "LOG_OR_EXPR",
      [TAU_NODE_LOG_AND_EXPR] = "LOG_AND_EXPR",
      [TAU_NODE_EQ_EXPR] = "EQ_EXPR",
      [TAU_NODE_NE_EXPR] = "NE_EXPR",
      [TAU_NODE_LT_EXPR] = "LT_EXPR",
      [TAU_NODE_LE_EXPR] = "LE_EXPR",
      [TAU_NODE_GT_EXPR] = "GT_EXPR",
      [TAU_NODE_GE_EXPR] = "GE_EXPR",
      [TAU_NODE_BIT_OR_EXPR] = "BIT_OR_EXPR",
      [TAU_NODE_BIT_XOR_EXPR] = "BIT_XOR_EXPR",
      [TAU_NODE_BIT_AND_EXPR] = "BIT_AND_EXPR",
      [TAU_NODE_LSH_EXPR] = "LSH_EXPR",
      [TAU_NODE_RSH_EXPR] = "RSH_EXPR",
      [TAU_NODE_ADD_EXPR] = "ADD_EXPR",
      [TAU_NODE_SUB_EXPR] = "SUB_EXPR",
      [TAU_NODE_MUL_EXPR] = "MUL_EXPR",
      [TAU_NODE_DIV_EXPR] = "DIV_EXPR",
      [TAU_NODE_REM_EXPR] = "REM_EXPR",
      [TAU_NODE_U_REF_EXPR] = "U_REF_EXPR",
      [TAU_NODE_PROOF_EXPR] = "PROOF_EXPR",
      [TAU_NODE_VALUE_LOOKUP_EXPR] = "VALUE_LOOKUP_EXPR",
      [TAU_NODE_STATIC_LOOKUP_EXPR] = "STATIC_LOOKUP_EXPR",
      [TAU_NODE_U_POS_EXPR] = "U_POS_EXPR",
      [TAU_NODE_U_NEG_EXPR] = "U_NEG_EXPR",
      [TAU_NODE_U_LOG_NOT_EXPR] = "U_LOG_NOT_EXPR",
      [TAU_NODE_U_BIT_NOT_EXPR] = "U_BIT_NOT_EXPR",
      [TAU_NODE_CALL_EXPR] = "CALL_EXPR",
      [TAU_NODE_INDEX_EXPR] = "INDEX_EXPR",
      [TAU_NODE_ATOM] = "ATOM",
  };
  static_assert(HANDLED_IDENTIFIER_TO_NODE_TYPE == TAU_NODE_COUNT);

  enum tau_node_type i;
  for (i = TAU_NODE_NONE; i < TAU_NODE_COUNT; i++) {
    size_t comparing_len = strlen(anode_names[i]);
    if (strncmp(anode_names[i], name, len) == 0 && len == comparing_len) {
      return i;
    }
  }

  return TAU_NODE_NONE;
}

// NOLINTNEXTLINE(misc-no-recursion)
static struct tau_node *parse_topology_expr(struct tau_token *ahead) {
  struct tau_node *node = NULL;
  struct tau_node *left = NULL;
  struct tau_node *right = NULL;
  struct tau_token node_start = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_LPAR, TAU_KEYWORD_NONE)) {
    struct tau_node *identifier = parse_atom(ahead);
    MUST_OR_FAIL(identifier && identifier->type == TAU_NODE_ATOM, ahead, "<topology identifier>");
    enum tau_node_type target_type = identifier_to_node_type(identifier->token.buf, identifier->token.len);
    if (!match(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_RPAR, TAU_KEYWORD_NONE)) {
      left = parse_topology_expr(ahead);
    }

    if (!match(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_RPAR, TAU_KEYWORD_NONE)) {
      right = parse_topology_expr(ahead);
    }

    node = node_new_binary(target_type, node_start, left, right);
    MUST_OR_FAIL(match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_RPAR, TAU_KEYWORD_NONE), ahead,
                 "<closing topology `)`>");
    return node;
  }

  struct tau_token atom_start = *ahead;
  struct tau_node *atomic = parse_atom(ahead);
  MUST_OR_FAIL(atomic, ahead, "<atomic topology expression>");
  return node_new_empty(TAU_NODE_ATOM, atom_start);
handle_fail:
  if (node != NULL) {
    node_free(node);
  }

  if (left != NULL) {
    node_free(left);
  }

  if (right != NULL) {
    node_free(right);
  }

  return NULL;
}

// NOLINTNEXTLINE(misc-no-recursion)
static void assert_nodes_equal(struct tau_node *given, struct tau_node *expected) {
  assert(given != NULL && "assert_nodes_equal: given cannot be null");
  assert(expected != NULL && "assert_nodes_equal: expected cannot be null");

  assert_int_equal(given->type, expected->type);
  if (given->type == TAU_NODE_ATOM) {
    char given_str[250] = {0};
    char expected_str[250] = {0};
    strncpy(given_str, given->token.buf, given->token.len);
    strncpy(expected_str, expected->token.buf, expected->token.len);
    assert_string_equal(given_str, expected_str);
  }

  if (expected->left != NULL) {
    assert_non_null(given->left);
    assert_nodes_equal(given->left, expected->left);
  }

  if (expected->right != NULL) {
    assert_non_null(given->right);
    assert_nodes_equal(given->right, expected->right);
  }
}

static void assert_node_topology(struct tau_node *given_node, const char *expected_str) {
  assert(given_node != NULL && "assert_node_topology: given_node cannot be null");
  assert(expected_str != NULL && "assert_node_topology: expected_str cannot be null");

  size_t expected_len = strlen(expected_str);
  char *topology_name = calloc(expected_len + 1, sizeof(char));
  strcpy(topology_name, expected_str);
  struct tau_token start = tau_token_start(topology_name, expected_str, expected_len);
  struct tau_token head = tau_token_next(start);
  struct tau_node *expected_node = parse_topology_expr(&head);
  assert(expected_node != NULL && "assert_node_topology: could not parse expected_str topology program");
  assert_nodes_equal(given_node, expected_node);
  free(topology_name);
  node_free(expected_node);
}

#endif  // TAU_TOPOLOGY_HELPER_H