//
// Created on 12/29/22.
//

#ifndef TAU_PARSER_TEST_UTILS_H
#define TAU_PARSER_TEST_UTILS_H

#include <cmocka.h>
#include <malloc.h>

#include "log.h"
#include "parser.h"
#include "parser_match.h"

struct test_node {
  struct tau_ptr_stack *items;
  struct tau_token token;
};

static enum tau_anode_type tau_anode_type_from_name(const char *name, size_t len) {
  const char *anode_names[TAU_ANODE_COUNT] = {
      [TAU_ANODE_NONE] = "NONE",
      [TAU_ANODE_COMPILATION_UNIT] = "COMPILATION_UNIT",
      [TAU_ANODE_DECLS] = "DECLS",
      [TAU_ANODE_MODULE_DECL] = "MODULE_DECL",
      [TAU_ANODE_LET_DECL] = "LET_DECL",
      [TAU_ANODE_PROC_DECL] = "PROC_DECL",
      [TAU_ANODE_EXTERN_DECL] = "EXTERN_DECL",
      [TAU_ANODE_TYPE_BIND] = "TYPE_BIND",
      [TAU_ANODE_DATA_BIND] = "DATA_BIND",
      [TAU_ANODE_FORMAL_ARGS] = "FORMAL_ARGS",
      [TAU_ANODE_FORMAL_ARG] = "FORMAL_ARG",
      [TAU_ANODE_PASSING_ARGS] = "PASSING_ARGS",
      [TAU_ANODE_BLOCK] = "BLOCK",
      [TAU_ANODE_RETURN_STMT] = "RETURN_STMT",
      [TAU_ANODE_CONTINUE_STMT] = "CONTINUE_STMT",
      [TAU_ANODE_BREAK_STMT] = "BREAK_STMT",
      [TAU_ANODE_ASSIGN_SET_STMT] = "ASSIGN_SET_STMT",
      [TAU_ANODE_ASSIGN_INC_STMT] = "ASSIGN_INC_STMT",
      [TAU_ANODE_ASSIGN_DEC_STMT] = "ASSIGN_DEC_STMT",
      [TAU_ANODE_ASSIGN_MUL_STMT] = "ASSIGN_MUL_STMT",
      [TAU_ANODE_ASSIGN_DIV_STMT] = "ASSIGN_DIV_STMT",
      [TAU_ANODE_ASSIGN_LSH_STMT] = "ASSIGN_LSH_STMT",
      [TAU_ANODE_ASSIGN_RSH_STMT] = "ASSIGN_RSH_STMT",
      [TAU_ANODE_ASSIGN_BIT_AND_STMT] = "ASSIGN_BIT_AND_STMT",
      [TAU_ANODE_ASSIGN_BIT_OR_STMT] = "ASSIGN_BIT_OR_STMT",
      [TAU_ANODE_ASSIGN_BIT_XOR_STMT] = "ASSIGN_BIT_XOR_STMT",
      [TAU_ANODE_CALL_STMT] = "CALL_STMT",
      [TAU_ANODE_IF_BRANCH_STMT] = "IF_BRANCH_STMT",
      [TAU_ANODE_LOOP_BRANCH_STMT] = "LOOP_BRANCH_STMT",
      [TAU_ANODE_THEN_CASE] = "THEN_CASE",
      [TAU_ANODE_ELIF_CASE] = "ELIF_CASE",
      [TAU_ANODE_ELSE_CASE] = "ELSE_CASE",
      [TAU_ANODE_LOG_OR_EXPR] = "LOG_OR_EXPR",
      [TAU_ANODE_LOG_AND_EXPR] = "LOG_AND_EXPR",
      [TAU_ANODE_REL_EQ_EXPR] = "REL_EQ_EXPR",
      [TAU_ANODE_REL_NE_EXPR] = "REL_NE_EXPR",
      [TAU_ANODE_CMP_GT_EXPR] = "CMP_GT_EXPR",
      [TAU_ANODE_CMP_LT_EXPR] = "CMP_LT_EXPR",
      [TAU_ANODE_CMP_GE_EXPR] = "CMP_GE_EXPR",
      [TAU_ANODE_CMP_LE_EXPR] = "CMP_LE_EXPR",
      [TAU_ANODE_BIT_OR_EXPR] = "BIT_OR_EXPR",
      [TAU_ANODE_BIT_XOR_EXPR] = "BIT_XOR_EXPR",
      [TAU_ANODE_BIT_AND_EXPR] = "BIT_AND_EXPR",
      [TAU_ANODE_BIT_LSH_EXPR] = "BIT_LSH_EXPR",
      [TAU_ANODE_BIT_RSH_EXPR] = "BIT_RSH_EXPR",
      [TAU_ANODE_ADD_EXPR] = "ADD_EXPR",
      [TAU_ANODE_SUB_EXPR] = "SUB_EXPR",
      [TAU_ANODE_MUL_EXPR] = "MUL_EXPR",
      [TAU_ANODE_DIV_EXPR] = "DIV_EXPR",
      [TAU_ANODE_REM_EXPR] = "REM_EXPR",
      [TAU_ANODE_UNARY_PLUS_EXPR] = "UNARY_PLUS_EXPR",
      [TAU_ANODE_UNARY_MINUS_EXPR] = "UNARY_MINUS_EXPR",
      [TAU_ANODE_UNARY_REF_EXPR] = "UNARY_REF_EXPR",
      [TAU_ANODE_UNARY_LOG_NOT_EXPR] = "UNARY_LOG_NOT_EXPR",
      [TAU_ANODE_UNARY_BIT_NOT_EXPR] = "UNARY_BIT_NOT_EXPR",
      [TAU_ANODE_TAG_EXPR] = "TAG_EXPR",
      [TAU_ANODE_CALL_EXPR] = "CALL_EXPR",
      [TAU_ANODE_DATA_LOOKUP_EXPR] = "DATA_LOOKUP_EXPR",
      [TAU_ANODE_TYPE_LOOKUP_EXPR] = "TYPE_LOOKUP_EXPR",
      [TAU_ANODE_LITERAL] = "LITERAL",
      [TAU_ANODE_IDENTIFIER] = "IDENTIFIER",
  };

  enum tau_anode_type i;
  for (i = TAU_ANODE_NONE; i < TAU_ANODE_COUNT; i++) {
    if (strncmp(anode_names[i], name, len) == 0) {
      return i;
    }
  }

  return TAU_ANODE_NONE;
}

static int get_anode_type_arity(enum tau_anode_type type) {
  switch (type) {
    case TAU_ANODE_MODULE_DECL:
    case TAU_ANODE_EXTERN_DECL:
    case TAU_ANODE_UNARY_PLUS_EXPR:
    case TAU_ANODE_UNARY_MINUS_EXPR:
    case TAU_ANODE_UNARY_REF_EXPR:
    case TAU_ANODE_UNARY_LOG_NOT_EXPR:
    case TAU_ANODE_UNARY_BIT_NOT_EXPR:
    case TAU_ANODE_LOOP_BRANCH_STMT:
    case TAU_ANODE_ELSE_CASE:
    case TAU_ANODE_TYPE_BIND:
    case TAU_ANODE_DATA_BIND:
    case TAU_ANODE_FORMAL_ARG:
    case TAU_ANODE_RETURN_STMT:
      return 1;
    case TAU_ANODE_COMPILATION_UNIT:
    case TAU_ANODE_ASSIGN_SET_STMT:
    case TAU_ANODE_ASSIGN_INC_STMT:
    case TAU_ANODE_ASSIGN_DEC_STMT:
    case TAU_ANODE_ASSIGN_MUL_STMT:
    case TAU_ANODE_ASSIGN_DIV_STMT:
    case TAU_ANODE_ASSIGN_LSH_STMT:
    case TAU_ANODE_ASSIGN_RSH_STMT:
    case TAU_ANODE_ASSIGN_BIT_AND_STMT:
    case TAU_ANODE_ASSIGN_BIT_OR_STMT:
    case TAU_ANODE_ASSIGN_BIT_XOR_STMT:
    case TAU_ANODE_THEN_CASE:
    case TAU_ANODE_ELIF_CASE:
    case TAU_ANODE_LOG_OR_EXPR:
    case TAU_ANODE_LOG_AND_EXPR:
    case TAU_ANODE_REL_EQ_EXPR:
    case TAU_ANODE_REL_NE_EXPR:
    case TAU_ANODE_CMP_GT_EXPR:
    case TAU_ANODE_CMP_LT_EXPR:
    case TAU_ANODE_CMP_GE_EXPR:
    case TAU_ANODE_CMP_LE_EXPR:
    case TAU_ANODE_BIT_OR_EXPR:
    case TAU_ANODE_BIT_XOR_EXPR:
    case TAU_ANODE_BIT_AND_EXPR:
    case TAU_ANODE_BIT_LSH_EXPR:
    case TAU_ANODE_BIT_RSH_EXPR:
    case TAU_ANODE_ADD_EXPR:
    case TAU_ANODE_SUB_EXPR:
    case TAU_ANODE_MUL_EXPR:
    case TAU_ANODE_DIV_EXPR:
    case TAU_ANODE_REM_EXPR:
    case TAU_ANODE_TAG_EXPR:
    case TAU_ANODE_DATA_LOOKUP_EXPR:
    case TAU_ANODE_TYPE_LOOKUP_EXPR:
    case TAU_ANODE_CALL_EXPR:
    case TAU_ANODE_CALL_STMT:
      return 2;
    case TAU_ANODE_IF_BRANCH_STMT:
    case TAU_ANODE_BLOCK:
    case TAU_ANODE_PASSING_ARGS:
    case TAU_ANODE_FORMAL_ARGS:
    case TAU_ANODE_PROC_DECL:
    case TAU_ANODE_LET_DECL:
    case TAU_ANODE_DECLS:
      return 3;
    case TAU_ANODE_LITERAL:
    case TAU_ANODE_IDENTIFIER:
    case TAU_ANODE_BREAK_STMT:
    case TAU_ANODE_CONTINUE_STMT:
      return 0;
    case TAU_ANODE_NONE:
    case TAU_ANODE_COUNT:
      assert(false && "tau_anode_free: unreachable code");
  }
}

static void test_node_free(void *maybe_test_node) {
  struct test_node *test_node = (struct test_node *)maybe_test_node;
  if (test_node->items != NULL) {
    tau_ptr_stack_free(test_node->items);
  }
  free(test_node);
}

// NOLINTNEXTLINE(misc-no-recursion)
static struct test_node *parse_test_expr(struct tau_token *ahead) {
  struct tau_token list_token = *ahead;
  struct test_node *node = NULL;

  if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_LPAR, TAU_KEYWORD_NONE)) {
    node = calloc(1, sizeof(struct test_node));
    node->items = tau_ptr_stack_new();
    node->token = list_token;

    while (true) {
      struct test_node *item = parse_test_expr(ahead);
      if (item != NULL) {
        tau_ptr_stack_push(node->items, item, test_node_free);
        continue;
      }

      break;
    }

    MUST_OR_FAIL(match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_RPAR, TAU_KEYWORD_NONE), ahead,
                 "<closing `)` for test expression>");
    return node;
  } else if (match_and_consume(ahead, TAU_TOKEN_TYPE_IDENTIFIER, TAU_PUNCT_NONE, TAU_KEYWORD_NONE) ||
             match_and_consume(ahead, TAU_TOKEN_TYPE_BOL_LIT, TAU_PUNCT_NONE, TAU_KEYWORD_NONE) ||
             match_and_consume(ahead, TAU_TOKEN_TYPE_INT_LIT, TAU_PUNCT_NONE, TAU_KEYWORD_NONE) ||
             match_and_consume(ahead, TAU_TOKEN_TYPE_FLT_LIT, TAU_PUNCT_NONE, TAU_KEYWORD_NONE) ||
             match_and_consume(ahead, TAU_TOKEN_TYPE_STR_LIT, TAU_PUNCT_NONE, TAU_KEYWORD_NONE) ||
             match_and_consume(ahead, TAU_TOKEN_TYPE_UNI_LIT, TAU_PUNCT_NONE, TAU_KEYWORD_NONE) ||
             match_and_consume(ahead, TAU_TOKEN_TYPE_NIL_LIT, TAU_PUNCT_NONE, TAU_KEYWORD_NONE)) {
    node = calloc(1, sizeof(struct test_node));
    node->token = list_token;
    return node;
  }

  return NULL;
handle_fail:
  if (node != NULL) {
    test_node_free(node);
  }

  return NULL;
}

struct test_node *parse_test_node(const char *test_name, const char *test_data) {
  struct tau_token start = tau_token_start(test_name, test_data, strlen(test_data));
  struct tau_token token = tau_token_next(start);
  struct test_node *node = parse_test_expr(&token);
  MUST_OR_RETURN(node, &token, "<test expression>", NULL);
  return node;
}

// NOLINTNEXTLINE(misc-no-recursion)
static bool assert_node_topology(struct tau_anode *given_node, struct test_node *expected_node) {
  struct tau_token expected_token = expected_node->token;
  struct tau_token given_token = given_node->token;

  char expected_data[1024] = {0};
  char given_data[1024] = {0};

  if (expected_node->items == NULL && expected_token.type == TAU_TOKEN_TYPE_IDENTIFIER ||
      expected_token.type == TAU_TOKEN_TYPE_BOL_LIT || expected_token.type == TAU_TOKEN_TYPE_INT_LIT ||
      expected_token.type == TAU_TOKEN_TYPE_FLT_LIT || expected_token.type == TAU_TOKEN_TYPE_STR_LIT ||
      expected_token.type == TAU_TOKEN_TYPE_UNI_LIT || expected_token.type == TAU_TOKEN_TYPE_NIL_LIT) {
    assert_int_equal(given_token.type, expected_token.type);
    strncpy(expected_data, expected_token.buf, expected_token.len);
    strncpy(given_data, given_token.buf, given_token.len);
    assert_string_equal(given_data, expected_data);
    return true;
  } else if (expected_node->items != NULL) {
    struct test_node *operator_node = tau_ptr_stack_get(expected_node->items, 0);
    enum tau_anode_type operator_type = tau_anode_type_from_name(operator_node->token.buf, operator_node->token.len);
    assert_int_equal(given_node->type, operator_type);
    int expected_arity = get_anode_type_arity(operator_type);
    if (expected_arity == 1) {
      // UNARY
      struct test_node *operand_node = tau_ptr_stack_get(expected_node->items, 1);
      assert_non_null(operand_node);
      assert_non_null(given_node->left);
      return assert_node_topology(given_node->left, operand_node);
    } else if (expected_arity == 2) {
      // BINARY
      struct test_node *left = tau_ptr_stack_get(expected_node->items, 1);
      struct test_node *right = tau_ptr_stack_get(expected_node->items, 2);
      // assert_int_equal(given_node->type, operator_type);
      assert_non_null(given_node->left);
      assert_non_null(given_node->right);
      return assert_node_topology(given_node->left, left) && assert_node_topology(given_node->right, right);
    } else if (expected_arity > 2) {
      // LIST
      assert_int_equal(given_node->stack->head, expected_node->items->head - 1);
      for (int64_t i = 1; i < expected_node->items->head; i++) {
        struct tau_anode *given_child = tau_ptr_stack_get(given_node->stack, i - 1);
        struct test_node *expected_child = tau_ptr_stack_get(expected_node->items, i);
        if (!assert_node_topology(given_child, expected_child)) {
          return false;
        }
      }
    }

    return true;
  }

  return false;
}

void assert_topology(struct tau_anode *given, const char *expected) {
  char test_name[1024] = {0};
  strcpy(test_name, "test: ");
  strcpy(test_name + 6, expected);
  struct test_node *test_node = parse_test_node(test_name, expected);
  assert_non_null(test_node);
  assert_true(assert_node_topology(given, test_node));
  test_node_free(test_node);
}

#endif  // TAU_PARSER_TEST_UTILS_H
