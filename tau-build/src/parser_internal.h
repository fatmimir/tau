//
// Created on 1/5/23.
//

#ifndef TAU_PARSER_INTERNAL_H
#define TAU_PARSER_INTERNAL_H

#include "lexer.h"

enum tau_node_type {
  TAU_NODE_NONE,
  TAU_NODE_CAST_EXPR,
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
  TAU_NODE_CALL_EXPR,
  TAU_NODE_INDEX_EXPR,
  TAU_NODE_ATOM,
  TAU_NODE_COUNT,
};

struct tau_node {
  struct tau_token token;
  struct tau_node *left;
  struct tau_node *right;
  enum tau_node_type type;
};

void node_free(struct tau_node *node);
struct tau_node *node_new_empty(enum tau_node_type type, struct tau_token token);
struct tau_node *node_new_unary(enum tau_node_type type, struct tau_token token, struct tau_node *operand);
struct tau_node *node_new_binary(enum tau_node_type type, struct tau_token token, struct tau_node *left, struct tau_node *right);

struct tau_node *parse_expr(struct tau_token *ahead);
struct tau_node *parse_cast_expr(struct tau_token *ahead);
struct tau_node *parse_log_or_expr(struct tau_token *ahead);
struct tau_node *parse_log_and_expr(struct tau_token *ahead);
struct tau_node *parse_rel_expr(struct tau_token *ahead);
struct tau_node *parse_bit_or_expr(struct tau_token *ahead);
struct tau_node *parse_bit_and_expr(struct tau_token *ahead);
struct tau_node *parse_bit_shift_expr(struct tau_token *ahead);
struct tau_node *parse_term_expr(struct tau_token *ahead);
struct tau_node *parse_fact_expr(struct tau_token *ahead);
struct tau_node *parse_ref_expr(struct tau_token *ahead);
struct tau_node *parse_proof_expr(struct tau_token *ahead);
struct tau_node *parse_value_lookup_expr(struct tau_token *ahead);
struct tau_node *parse_static_lookup_expr(struct tau_token *ahead);
struct tau_node *parse_unary_expr(struct tau_token *ahead);
struct tau_node *parse_call_expr(struct tau_token *ahead);
struct tau_node *parse_index_expr(struct tau_token *ahead);
struct tau_node *parse_primary_expr(struct tau_token *ahead);
struct tau_node *parse_atom(struct tau_token *ahead);

struct tau_node *passing_args(struct tau_token *ahead);
struct tau_node *index_lookup(struct tau_token *ahead);

struct tau_node *parse_return_stmt(struct tau_token *ahead);
struct tau_node *parse_continue_stmt(struct tau_token *ahead);
struct tau_node *parse_break_stmt(struct tau_token *ahead);
struct tau_node *parse_if_stmt(struct tau_token *ahead);
struct tau_node *parse_main_case(struct tau_token *ahead);
struct tau_node *parse_elif_case(struct tau_token *ahead);
struct tau_node *parse_else_case(struct tau_token *ahead);
struct tau_node *parse_while_stmt(struct tau_token *ahead);

struct tau_node *parse_type_bind(struct tau_token *ahead);
struct tau_node *parse_data_bind(struct tau_token *ahead);

struct tau_node *parse_statement_or_decl(struct tau_token *ahead);
struct tau_node *parse_block(struct tau_token *ahead);

struct tau_node *parse_let_deconstruction(struct tau_token *ahead);
struct tau_node *parse_proc_signature(struct tau_token *ahead);
struct tau_node *parse_formal_arg_list(struct tau_token *ahead);
struct tau_node *parse_formal_arg_item(struct tau_token *ahead);
struct tau_node *parse_proc_deconstruction(struct tau_token *ahead);
struct tau_node *parse_type_deconstruction(struct tau_token *ahead);

struct tau_node *parse_module_decl(struct tau_token *ahead);

struct tau_node *parse_let_decl(struct tau_token *ahead);
struct tau_node *parse_proc_decl(struct tau_token *ahead);
struct tau_node *parse_type_decl(struct tau_token *ahead);
struct tau_node *parse_decl(struct tau_token *ahead);

struct tau_node *parse_compilation_unit(struct tau_token *ahead);


#endif  // TAU_PARSER_INTERNAL_H
