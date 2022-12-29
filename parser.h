//
// Created on 12/21/22.
//

#ifndef TAU_PARSER_H
#define TAU_PARSER_H

#include "lexer.h"
#include "ptr_stack.h"

enum tau_anode_type {
  TAU_ANODE_NONE,
  TAU_ANODE_COMPILATION_UNIT,
  TAU_ANODE_DECLS,
  TAU_ANODE_MODULE_DECL,
  TAU_ANODE_LET_DECL,
  TAU_ANODE_PROC_DECL,
  TAU_ANODE_EXTERN_DECL,
  TAU_ANODE_TYPE_BIND,
  TAU_ANODE_DATA_BIND,
  TAU_ANODE_FORMAL_ARGS,
  TAU_ANODE_FORMAL_ARG,
  TAU_ANODE_PASSING_ARGS,
  TAU_ANODE_BLOCK,
  TAU_ANODE_RETURN_STMT,
  TAU_ANODE_CONTINUE_STMT,
  TAU_ANODE_BREAK_STMT,
  TAU_ANODE_ASSIGN_SET_STMT,
  TAU_ANODE_ASSIGN_INC_STMT,
  TAU_ANODE_ASSIGN_DEC_STMT,
  TAU_ANODE_ASSIGN_MUL_STMT,
  TAU_ANODE_ASSIGN_DIV_STMT,
  TAU_ANODE_ASSIGN_LSH_STMT,
  TAU_ANODE_ASSIGN_RSH_STMT,
  TAU_ANODE_ASSIGN_BIT_AND_STMT,
  TAU_ANODE_ASSIGN_BIT_OR_STMT,
  TAU_ANODE_ASSIGN_BIT_XOR_STMT,
  TAU_ANODE_CALL_STMT,
  TAU_ANODE_IF_BRANCH_STMT,
  TAU_ANODE_LOOP_BRANCH_STMT,
  TAU_ANODE_THEN_CASE,
  TAU_ANODE_ELIF_CASE,
  TAU_ANODE_ELSE_CASE,
  TAU_ANODE_LOG_OR_EXPR,
  TAU_ANODE_LOG_AND_EXPR,
  TAU_ANODE_REL_EQ_EXPR,
  TAU_ANODE_REL_NE_EXPR,
  TAU_ANODE_CMP_GT_EXPR,
  TAU_ANODE_CMP_LT_EXPR,
  TAU_ANODE_CMP_GE_EXPR,
  TAU_ANODE_CMP_LE_EXPR,
  TAU_ANODE_BIT_OR_EXPR,
  TAU_ANODE_BIT_XOR_EXPR,
  TAU_ANODE_BIT_AND_EXPR,
  TAU_ANODE_BIT_LSH_EXPR,
  TAU_ANODE_BIT_RSH_EXPR,
  TAU_ANODE_ADD_EXPR,
  TAU_ANODE_SUB_EXPR,
  TAU_ANODE_MUL_EXPR,
  TAU_ANODE_DIV_EXPR,
  TAU_ANODE_REM_EXPR,
  TAU_ANODE_UNARY_PLUS_EXPR,
  TAU_ANODE_UNARY_MINUS_EXPR,
  TAU_ANODE_UNARY_REF_EXPR,
  TAU_ANODE_UNARY_LOG_NOT_EXPR,
  TAU_ANODE_UNARY_BIT_NOT_EXPR,
  TAU_ANODE_TAG_EXPR,
  TAU_ANODE_CALL_EXPR,
  TAU_ANODE_DATA_LOOKUP_EXPR,
  TAU_ANODE_TYPE_LOOKUP_EXPR,
  TAU_ANODE_ATOM_EXPR,
  TAU_ANODE_LITERAL,
  TAU_ANODE_IDENTIFIER,
  TAU_ANODE_COUNT,
};

struct tau_anode {
  union {
    struct {
      struct tau_anode *left;
      struct tau_anode *right;
    };

    struct tau_ptr_stack *stack;
  };
  struct tau_token token;
  enum tau_anode_type type;
};

struct tau_anode *tau_parse(const char *buf_name, const char *buf_data, size_t buf_size);
void tau_anode_free(void *maybe_anode);

#endif  // TAU_PARSER_H
