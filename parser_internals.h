//
// Created on 12/29/22.
//

#ifndef TAU_PARSER_INTERNALS_H
#define TAU_PARSER_INTERNALS_H

struct tau_anode *parse_compilation_unit(struct tau_token *ahead);
struct tau_anode *parse_decls(struct tau_token *ahead);
struct tau_anode *parse_decl(struct tau_token *ahead);
struct tau_anode *parse_module_decl(struct tau_token *ahead);
struct tau_anode *parse_let_decl(struct tau_token *ahead);
struct tau_anode *parse_proc_decl(struct tau_token *ahead);
struct tau_anode *parse_extern_decl(struct tau_token *ahead);
struct tau_anode *parse_type_bind(struct tau_token *ahead);
struct tau_anode *parse_data_bind(struct tau_token *ahead);
struct tau_anode *parse_formal_args(struct tau_token *ahead);
struct tau_anode *parse_formal_arg(struct tau_token *ahead);
struct tau_anode *parse_passing_args(struct tau_token *ahead);
struct tau_anode *parse_block(struct tau_token *ahead);
struct tau_anode *parse_return_stmt(struct tau_token *ahead);
struct tau_anode *parse_continue_stmt(struct tau_token *ahead);
struct tau_anode *parse_break_stmt(struct tau_token *ahead);
struct tau_anode *parse_assign_or_call_stmt(struct tau_token *ahead);
struct tau_anode *parse_if_stmt(struct tau_token *ahead);
struct tau_anode *parse_loop_stmt(struct tau_token *ahead);
struct tau_anode *parse_statement(struct tau_token *ahead);
struct tau_anode *parse_expr(struct tau_token *ahead);
struct tau_anode *parse_log_or_expr(struct tau_token *ahead);
struct tau_anode *parse_log_and_expr(struct tau_token *ahead);
struct tau_anode *parse_rel_expr(struct tau_token *ahead);
struct tau_anode *parse_cmp_expr(struct tau_token *ahead);
struct tau_anode *parse_bit_or_expr(struct tau_token *ahead);
struct tau_anode *parse_bit_and_expr(struct tau_token *ahead);
struct tau_anode *parse_bit_shift_expr(struct tau_token *ahead);
struct tau_anode *parse_sum_expr(struct tau_token *ahead);
struct tau_anode *parse_mul_expr(struct tau_token *ahead);
struct tau_anode *parse_unary_expr(struct tau_token *ahead);
struct tau_anode *parse_tag_expr(struct tau_token *ahead);
struct tau_anode *parse_call_expr(struct tau_token *ahead);
struct tau_anode *parse_data_lookup_expr(struct tau_token *ahead);
struct tau_anode *parse_type_lookup_expr(struct tau_token *ahead);
struct tau_anode *parse_atom_expr(struct tau_token *ahead);
struct tau_anode *parse_identifier(struct tau_token *ahead);

#endif  // TAU_PARSER_INTERNALS_H
