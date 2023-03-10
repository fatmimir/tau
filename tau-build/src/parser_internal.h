//
// Created on 1/5/23.
//

#ifndef TAU_PARSER_INTERNAL_H
#define TAU_PARSER_INTERNAL_H

#include "lexer.h"

enum tau_node_type {
  TAU_NODE_NONE,
  TAU_NODE_CALLING_ARGS,
  TAU_NODE_CALLING_ARG,
  TAU_NODE_INDEXING_ARGS,
  TAU_NODE_INDEXING_ARG,
  TAU_NODE_CAST_EXPR,
  TAU_NODE_LOG_OR_EXPR,
  TAU_NODE_LOG_AND_EXPR,
  TAU_NODE_EQ_EXPR,
  TAU_NODE_NE_EXPR,
  TAU_NODE_LT_EXPR,
  TAU_NODE_LE_EXPR,
  TAU_NODE_GT_EXPR,
  TAU_NODE_GE_EXPR,
  TAU_NODE_BIT_OR_EXPR,
  TAU_NODE_BIT_XOR_EXPR,
  TAU_NODE_BIT_AND_EXPR,
  TAU_NODE_LSH_EXPR,
  TAU_NODE_RSH_EXPR,
  TAU_NODE_ADD_EXPR,
  TAU_NODE_SUB_EXPR,
  TAU_NODE_MUL_EXPR,
  TAU_NODE_DIV_EXPR,
  TAU_NODE_REM_EXPR,
  TAU_NODE_U_REF_EXPR,
  TAU_NODE_PROOF_EXPR,
  TAU_NODE_STATIC_LOOKUP_EXPR,
  TAU_NODE_VALUE_LOOKUP_EXPR,
  TAU_NODE_U_POS_EXPR,
  TAU_NODE_U_NEG_EXPR,
  TAU_NODE_U_LOG_NOT_EXPR,
  TAU_NODE_U_BIT_NOT_EXPR,
  TAU_NODE_SUBSCRIPTION_EXPR,
  TAU_NODE_ATOM,
  TAU_NODE_RETURN_STMT,
  TAU_NODE_CONTINUE_STMT,
  TAU_NODE_BREAK_STMT,
  TAU_NODE_IF_STMT,
  TAU_NODE_MAIN_BRANCH,
  TAU_NODE_ELIF_BRANCH,
  TAU_NODE_ELSE_BRANCH,
  TAU_NODE_WHILE_STMT,
  TAU_NODE_ASSIGN_STMT,
  TAU_NODE_ACCUM_ADD_STMT,
  TAU_NODE_ACCUM_SUB_STMT,
  TAU_NODE_ACCUM_MUL_STMT,
  TAU_NODE_ACCUM_DIV_STMT,
  TAU_NODE_ACCUM_REM_STMT,
  TAU_NODE_ACCUM_BIT_AND_STMT,
  TAU_NODE_ACCUM_BIT_OR_STMT,
  TAU_NODE_ACCUM_BIT_XOR_STMT,
  TAU_NODE_ACCUM_RSH_STMT,
  TAU_NODE_ACCUM_LSH_STMT,
  TAU_NODE_STATEMENT_OR_DECL,
  TAU_NODE_BLOCK,
  TAU_NODE_EXPR_WITH_BLOCK,
  TAU_NODE_TYPE_BIND,
  TAU_NODE_DATA_BIND,
  TAU_NODE_LET_DECONSTRUCTION,
  TAU_NODE_PROC_SIGNATURE,
  TAU_NODE_FORMAL_ARGS,
  TAU_NODE_FORMAL_ARG,
  TAU_NODE_ARG_BIND,
  TAU_NODE_PROC_DECONSTRUCTION,
  TAU_NODE_TYPE_DECONSTRUCTION,
  TAU_NODE_MODULE_DECL,
  TAU_NODE_PROTOTYPE_SUFFIX,
  TAU_NODE_LET_DECL,
  TAU_NODE_PROC_DECL,
  TAU_NODE_TYPE_DECL,
  TAU_NODE_EXTERN_DECL,
  TAU_NODE_DECL,
  TAU_NODE_DECLS,
  TAU_NODE_COMPILATION_UNIT,
  TAU_NODE_COUNT,
};

struct tau_node {
  struct tau_token token;
  struct tau_node *left;
  struct tau_node *right;
  enum tau_node_type type;
};

typedef struct tau_node *(*parser_func_t)(struct tau_token *);

void node_free(struct tau_node *node);
struct tau_node *node_new_empty(enum tau_node_type type, struct tau_token token);
struct tau_node *node_new_unary(enum tau_node_type type, struct tau_token token, struct tau_node *operand);
struct tau_node *node_new_binary(enum tau_node_type type, struct tau_token token, struct tau_node *left,
                                 struct tau_node *right);

struct tau_node *parse_expr(struct tau_token *ahead);
struct tau_node *parse_cast_expr(struct tau_token *ahead);
struct tau_node *parse_log_or_expr(struct tau_token *ahead);
struct tau_node *parse_log_and_expr(struct tau_token *ahead);
struct tau_node *parse_rel_expr(struct tau_token *ahead);
struct tau_node *parse_cmp_expr(struct tau_token *ahead);
struct tau_node *parse_bit_or_expr(struct tau_token *ahead);
struct tau_node *parse_bit_and_expr(struct tau_token *ahead);
struct tau_node *parse_bit_shift_expr(struct tau_token *ahead);
struct tau_node *parse_term_expr(struct tau_token *ahead);
struct tau_node *parse_fact_expr(struct tau_token *ahead);
struct tau_node *parse_ref_expr(struct tau_token *ahead);
struct tau_node *parse_proof_expr(struct tau_token *ahead);
struct tau_node *parse_unary_expr(struct tau_token *ahead);
struct tau_node *parse_subscription_expr(struct tau_token *ahead);
struct tau_node *parse_value_lookup_expr(struct tau_token *ahead);
struct tau_node *parse_static_lookup_expr(struct tau_token *ahead);
struct tau_node *parse_primary_expr(struct tau_token *ahead);
struct tau_node *parse_atom(struct tau_token *ahead);

struct tau_node *parse_calling_args(struct tau_token *ahead);
struct tau_node *parse_indexing_args(struct tau_token *ahead);

struct tau_node *parse_return_stmt(struct tau_token *ahead);
struct tau_node *parse_continue_stmt(struct tau_token *ahead);
struct tau_node *parse_break_stmt(struct tau_token *ahead);
struct tau_node *parse_if_stmt(struct tau_token *ahead);
struct tau_node *parse_main_branch(struct tau_token *ahead);
struct tau_node *parse_elif_branch(struct tau_token *ahead);
struct tau_node *parse_else_branch(struct tau_token *ahead);
struct tau_node *parse_while_stmt(struct tau_token *ahead);
struct tau_node *parse_assign_stmt(struct tau_token *ahead);
struct tau_node *parse_subscription_stmt(struct tau_token *ahead);

struct tau_node *parse_statement_or_decl(struct tau_token *ahead);
struct tau_node *parse_block(struct tau_token *ahead);
struct tau_node *parse_expr_with_block(struct tau_token *ahead);

struct tau_node *parse_type_bind(struct tau_token *ahead);
struct tau_node *parse_data_bind(struct tau_token *ahead);

struct tau_node *parse_let_deconstruction(struct tau_token *ahead);
struct tau_node *parse_proc_signature(struct tau_token *ahead);
struct tau_node *parse_formal_args(struct tau_token *ahead);
struct tau_node *parse_formal_arg(struct tau_token *ahead);
struct tau_node *parse_arg_bind(struct tau_token *ahead);
struct tau_node *parse_proc_deconstruction(struct tau_token *ahead);
struct tau_node *parse_type_deconstruction(struct tau_token *ahead);

struct tau_node *parse_module_decl(struct tau_token *ahead);

struct tau_node *parse_prototype_suffix(struct tau_token *ahead);
struct tau_node *parse_let_decl(struct tau_token *ahead);
struct tau_node *parse_proc_decl(struct tau_token *ahead);
struct tau_node *parse_type_decl(struct tau_token *ahead);
struct tau_node *parse_extern_decl(struct tau_token *ahead);
struct tau_node *parse_decl(struct tau_token *ahead);
struct tau_node *parse_decls(struct tau_token *ahead);

struct tau_node *parse_compilation_unit(struct tau_token *ahead);

#endif  // TAU_PARSER_INTERNAL_H
