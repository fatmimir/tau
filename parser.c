//
// Created on 12/21/22.
//

#include "parser.h"

#include <assert.h>
#include <malloc.h>

#include "log.h"

////////////////////////////////////////////////////////////////////// MATCHING FUNCTIONS / UTILITIES
#include "parser_alloc.h"
#include "parser_internals.h"
#include "parser_match.h"

////////////////////////////////////////////////////////////////////// PARSING FUNCTIONS (IMPL)

// compilation_unit = module_decl, decls eof;
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_compilation_unit(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_compilation_unit: ahead cannot be null");
  struct tau_token compilation_unit_token;
  struct tau_anode *module_decl = NULL;
  struct tau_anode *decls = NULL;

  compilation_unit_token = *ahead;
  module_decl = parse_module_decl(ahead);
  MUST_OR_FAIL(module_decl, ahead, "<module decl>");

  decls = parse_decls(ahead);
  MUST_OR_FAIL(decls, ahead, "<declarations>");

  MUST_OR_FAIL(match(ahead, TAU_TOKEN_TYPE_EOF, TAU_PUNCT_NONE, TAU_KEYWORD_NONE), ahead, "<end of file>");
  return new_anode_binary(TAU_ANODE_COMPILATION_UNIT, compilation_unit_token, module_decl, decls);
handle_fail:
  if (module_decl != NULL) {
    tau_anode_free(module_decl);
  }

  if (decls != NULL) {
    tau_anode_free(decls);
  }

  return NULL;
}

// decls = { decl, eol };
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_decls(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_decls: ahead cannot be null");
  struct tau_anode *decls = NULL;
  struct tau_anode *node = NULL;

  decls = new_anode_stack(TAU_ANODE_DECLS, *ahead);
  while (true) {
    node = parse_decl(ahead);
    if (node != NULL) {
      tau_ptr_stack_push(decls->stack, node, tau_anode_free);
      continue;
    }

    break;
  }

  return decls;
}

// decl = let_decl | proc_decl | extern_decl;
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_decl(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_decl: ahead cannot be null");
  struct tau_anode *node = NULL;

  node = parse_let_decl(ahead);
  if (node != NULL) {
    return node;
  }

  node = parse_proc_decl(ahead);
  if (node != NULL) {
    return node;
  }

  node = parse_extern_decl(ahead);
  if (node != NULL) {
    return node;
  }

  return NULL;
}

// module_decl = "module", type_lookup_expr, eol;
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_module_decl(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_module_decl: ahead cannot be null");
  struct tau_token module_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_KEYWORD, TAU_PUNCT_NONE, TAU_KEYWORD_MODULE)) {
    struct tau_anode *identifier = parse_type_lookup_expr(ahead);
    MUST_OR_RETURN(identifier, ahead, "<type lookup>", NULL);
    return new_anode_unary(TAU_ANODE_MODULE_DECL, module_token, identifier);
  }

  return NULL;
}

// let_decl = "let", identifier, type_bind, (data_bind | "prototype");
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_let_decl(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_let_decl: ahead cannot be null");
  struct tau_anode *let_decl = NULL;
  struct tau_anode *identifier = NULL;
  struct tau_anode *type_bind = NULL;
  struct tau_anode *data_bind = NULL;
  struct tau_token let_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_KEYWORD, TAU_PUNCT_NONE, TAU_KEYWORD_LET)) {
    let_decl = new_anode_stack(TAU_ANODE_LET_DECL, let_token);

    identifier = parse_identifier(ahead);
    MUST_OR_FAIL(identifier, ahead, "<identifier>");
    tau_ptr_stack_push(let_decl->stack, identifier, tau_anode_free);

    type_bind = parse_type_bind(ahead);
    MUST_OR_FAIL(type_bind, ahead, "<type bind>");
    tau_ptr_stack_push(let_decl->stack, type_bind, tau_anode_free);

    if (!match_and_consume(ahead, TAU_TOKEN_TYPE_KEYWORD, TAU_PUNCT_NONE, TAU_KEYWORD_PROTOTYPE)) {
      data_bind = parse_data_bind(ahead);
      MUST_OR_FAIL(type_bind, ahead, "<data bind>");
      tau_ptr_stack_push(let_decl->stack, data_bind, tau_anode_free);
    }

    return let_decl;
  }

  return NULL;
handle_fail:
  if (let_decl != NULL) {
    tau_anode_free(let_decl);
  }

  if (identifier != NULL) {
    tau_anode_free(identifier);
  }

  if (type_bind != NULL) {
    tau_anode_free(type_bind);
  }

  if (data_bind != NULL) {
    tau_anode_free(data_bind);
  }

  return NULL;
}

// proc_decl = "proc", identifier, formal_args, type_bind, (block | data_bind | "prototype");
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_proc_decl(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_proc_decl: ahead cannot be null");
  struct tau_anode *proc_decl = NULL;
  struct tau_token proc_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_KEYWORD, TAU_PUNCT_NONE, TAU_KEYWORD_PROC)) {
    proc_decl = new_anode_stack(TAU_ANODE_PROC_DECL, proc_token);

    struct tau_anode *identifier = parse_identifier(ahead);
    MUST_OR_FAIL(identifier, ahead, "<identifier>");
    tau_ptr_stack_push(proc_decl->stack, identifier, tau_anode_free);

    struct tau_anode *formal_args = parse_formal_args(ahead);
    MUST_OR_FAIL(formal_args, ahead, "<formal args>");
    tau_ptr_stack_push(proc_decl->stack, formal_args, tau_anode_free);

    struct tau_anode *type_bind = parse_type_bind(ahead);
    MUST_OR_FAIL(type_bind, ahead, "<type bind>");
    tau_ptr_stack_push(proc_decl->stack, type_bind, tau_anode_free);

    struct tau_anode *block_or_data_bind = parse_data_bind(ahead);
    if (block_or_data_bind != NULL) {
      tau_ptr_stack_push(proc_decl->stack, block_or_data_bind, tau_anode_free);
      return proc_decl;
    }

    block_or_data_bind = parse_block(ahead);
    if (block_or_data_bind != NULL) {
      tau_ptr_stack_push(proc_decl->stack, block_or_data_bind, tau_anode_free);
      return proc_decl;
    }

    MUST_OR_FAIL(match_and_consume(ahead, TAU_TOKEN_TYPE_KEYWORD, TAU_PUNCT_NONE, TAU_KEYWORD_PROTOTYPE), ahead,
                 "<data bind> <block> or <prototype>");
    return proc_decl;
  }

  return NULL;
handle_fail:
  if (proc_decl != NULL) {
    tau_anode_free(proc_decl);
  }

  return NULL;
}

// extern_decl = "extern", ( let_decl | proc_decl );
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_extern_decl(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_extern_decl: ahead cannot be null");
  struct tau_anode *decl = NULL;
  struct tau_token extern_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_KEYWORD, TAU_PUNCT_NONE, TAU_KEYWORD_EXTERN)) {
    decl = parse_let_decl(ahead);
    if (decl != NULL) {
      return new_anode_unary(TAU_ANODE_EXTERN_DECL, extern_token, decl);
    }

    decl = parse_proc_decl(ahead);
    MUST_OR_FAIL(decl, ahead, "<let decl> or <proc decl>");
    return new_anode_unary(TAU_ANODE_EXTERN_DECL, extern_token, decl);
  }

  return NULL;
handle_fail:
  if (decl != NULL) {
    tau_anode_free(decl);
  }

  return NULL;
}

// type_bind = ":", expr;
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_type_bind(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_type_bind: ahead cannot be null");
  struct tau_anode *bind = NULL;
  struct tau_token bind_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_COLON, TAU_KEYWORD_NONE)) {
    bind = parse_expr(ahead);
    MUST_OR_FAIL(bind, ahead, "<expression>");
    return new_anode_unary(TAU_ANODE_TYPE_BIND, bind_token, bind);
  }

  return bind;
handle_fail:
  if (bind != NULL) {
    tau_anode_free(bind);
  }

  return NULL;
}

// data_bind = "=", expr;
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_data_bind(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_data_bind: ahead cannot be null");
  struct tau_anode *bind = NULL;
  struct tau_token bind_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_EQ, TAU_KEYWORD_NONE)) {
    bind = parse_expr(ahead);
    MUST_OR_FAIL(bind, ahead, "<expression>");
    return new_anode_unary(TAU_ANODE_DATA_BIND, bind_token, bind);
  }

  return bind;
handle_fail:
  if (bind != NULL) {
    tau_anode_free(bind);
  }

  return NULL;
}

// formal_args = "(", [ formal_arg | { formal_arg "," } ], ")";
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_formal_args(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_formal_args: ahead cannot be null");
  struct tau_anode *formal_args = NULL;
  struct tau_token formal_args_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_LPAR, TAU_KEYWORD_NONE)) {
    formal_args = new_anode_stack(TAU_ANODE_FORMAL_ARGS, formal_args_token);

    while (true) {
      if (match(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_RPAR, TAU_KEYWORD_NONE)) {
        break;
      }

      struct tau_anode *arg = parse_formal_arg(ahead);
      if (arg != NULL) {
        tau_ptr_stack_push(arg->stack, arg, tau_ptr_stack_free);
        if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_COMMA, TAU_KEYWORD_NONE)) {
          continue;
        }
      }

      break;
    }

    MUST_OR_FAIL(match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_RPAR, TAU_KEYWORD_NONE), ahead,
                 "<closing `)`>");
    return formal_args;
  }

  return formal_args;
handle_fail:
  if (formal_args != NULL) {
    tau_anode_free(formal_args);
  }

  return NULL;
}

// formal_arg = identifier, type_bind;
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_formal_arg(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_formal_arg: ahead cannot be null");
  struct tau_anode *identifier = NULL;
  struct tau_anode *type_bind = NULL;
  struct tau_token formal_arg_token = *ahead;
  identifier = parse_identifier(ahead);
  MUST_OR_FAIL(identifier, ahead, "<identifier>");

  type_bind = parse_type_bind(ahead);
  MUST_OR_FAIL(type_bind, ahead, "<type bind>");

  return new_anode_binary(TAU_ANODE_FORMAL_ARG, formal_arg_token, identifier, type_bind);
handle_fail:
  if (identifier != NULL) {
    tau_anode_free(identifier);
  }

  if (type_bind != NULL) {
    tau_anode_free(type_bind);
  }

  return NULL;
}

// passing_args = "(", [ expr | { expr, "," } ], ")";
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_passing_args(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_passing_args: ahead cannot be null");
  struct tau_anode *passing_args = NULL;
  struct tau_token passing_args_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_LPAR, TAU_KEYWORD_NONE)) {
    passing_args = new_anode_stack(TAU_ANODE_PASSING_ARGS, passing_args_token);

    while (true) {
      if (match(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_RPAR, TAU_KEYWORD_NONE)) {
        break;
      }

      struct tau_anode *arg = parse_expr(ahead);
      if (arg != NULL) {
        tau_ptr_stack_push(arg->stack, arg, tau_ptr_stack_free);
        if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_COMMA, TAU_KEYWORD_NONE)) {
          continue;
        }
      }

      break;
    }

    MUST_OR_FAIL(match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_RPAR, TAU_KEYWORD_NONE), ahead,
                 "<closing `)`>");
    return passing_args;
  }
handle_fail:
  if (passing_args != NULL) {
    tau_anode_free(passing_args);
  }

  return NULL;
}

// block = "{", [eol], { statement eol }, "}";
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_block(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_block: ahead cannot be null");
  struct tau_anode *block = NULL;
  struct tau_anode *statement_or_decl = NULL;
  struct tau_token block_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_LCBR, TAU_KEYWORD_NONE)) {
    block = new_anode_stack(TAU_ANODE_BLOCK, block_token);
    while (true) {
      statement_or_decl = parse_statement(ahead);
      if (statement_or_decl != NULL) {
        tau_ptr_stack_push(statement_or_decl->stack, statement_or_decl, tau_ptr_stack_free);
        if (match_and_consume(ahead, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE)) {
          continue;
        }
      }

      statement_or_decl = parse_decl(ahead);
      if (statement_or_decl != NULL) {
        tau_ptr_stack_push(statement_or_decl->stack, statement_or_decl, tau_ptr_stack_free);
        if (match_and_consume(ahead, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE)) {
          continue;
        }
      }

      break;
    }

    MUST_OR_FAIL(match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_RCBR, TAU_KEYWORD_NONE), ahead,
                 "<closing `}`>");
    return block;
  }

  return NULL;
handle_fail:
  if (block != NULL) {
    tau_anode_free(block);
  }

  return NULL;
}

// return_stmt = "return", expr;
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_return_stmt(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_return_stmt: ahead cannot be null");
  struct tau_token return_stmt_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_KEYWORD, TAU_PUNCT_NONE, TAU_KEYWORD_RETURN)) {
    struct tau_anode *expr = parse_expr(ahead);
    return new_anode_unary(TAU_ANODE_RETURN_STMT, return_stmt_token, expr);
  }

  return NULL;
}

// continue_stmt = "continue";
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_continue_stmt(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_continue_stmt: ahead cannot be null");
  struct tau_token continue_stmt_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_KEYWORD, TAU_PUNCT_NONE, TAU_KEYWORD_CONTINUE)) {
    return new_anode(TAU_ANODE_CONTINUE_STMT, continue_stmt_token);
  }

  return NULL;
}

// break_stmt = "break";
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_break_stmt(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_break_stmt: ahead cannot be null");
  struct tau_token break_stmt_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_KEYWORD, TAU_PUNCT_NONE, TAU_KEYWORD_BREAK)) {
    return new_anode(TAU_ANODE_BREAK_STMT, break_stmt_token);
  }

  return NULL;
}

// assign_or_call_stmt = (data_lookup_expr, ("=" | "+=" | "-=" | "*=" | "/=" | ">>=" | "<<=" | "&=" | "|=" | "^="),
// expr) | call_expr;
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_assign_or_call_stmt(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_assign_or_call_stmt: ahead cannot be null");
  struct tau_token stmt_token;
  struct tau_anode *node = parse_call_expr(ahead);
  struct tau_anode *expr = NULL;
  if (node != NULL) {
    if (node->type == TAU_ANODE_CALL_EXPR) {
      node->type = TAU_ANODE_CALL_STMT;
      return node;
    } else if (node->type == TAU_ANODE_DATA_LOOKUP_EXPR || node->type == TAU_ANODE_TYPE_LOOKUP_EXPR ||
               node->type == TAU_ANODE_ATOM_EXPR) {
      stmt_token = *ahead;
      if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_EQ, TAU_KEYWORD_NONE)) {
        expr = parse_expr(ahead);
        MUST_OR_FAIL(expr, ahead, "<expression>");
        return new_anode_binary(TAU_ANODE_ASSIGN_SET_STMT, stmt_token, node, expr);
      } else if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_PLUS_EQ, TAU_KEYWORD_NONE)) {
        expr = parse_expr(ahead);
        MUST_OR_FAIL(expr, ahead, "<expression>");
        return new_anode_binary(TAU_ANODE_ASSIGN_INC_STMT, stmt_token, node, expr);
      } else if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_HYPHEN_EQ, TAU_KEYWORD_NONE)) {
        expr = parse_expr(ahead);
        MUST_OR_FAIL(expr, ahead, "<expression>");
        return new_anode_binary(TAU_ANODE_ASSIGN_DEC_STMT, stmt_token, node, expr);
      } else if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_AST_EQ, TAU_KEYWORD_NONE)) {
        expr = parse_expr(ahead);
        MUST_OR_FAIL(expr, ahead, "<expression>");
        return new_anode_binary(TAU_ANODE_ASSIGN_MUL_STMT, stmt_token, node, expr);
      } else if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_SLASH_EQ, TAU_KEYWORD_NONE)) {
        expr = parse_expr(ahead);
        MUST_OR_FAIL(expr, ahead, "<expression>");
        return new_anode_binary(TAU_ANODE_ASSIGN_DIV_STMT, stmt_token, node, expr);
      } else if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_D_GT_EQ, TAU_KEYWORD_NONE)) {
        expr = parse_expr(ahead);
        MUST_OR_FAIL(expr, ahead, "<expression>");
        return new_anode_binary(TAU_ANODE_ASSIGN_RSH_STMT, stmt_token, node, expr);
      } else if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_D_LT_EQ, TAU_KEYWORD_NONE)) {
        expr = parse_expr(ahead);
        MUST_OR_FAIL(expr, ahead, "<expression>");
        return new_anode_binary(TAU_ANODE_ASSIGN_LSH_STMT, stmt_token, node, expr);
      } else if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_AMP_EQ, TAU_KEYWORD_NONE)) {
        expr = parse_expr(ahead);
        MUST_OR_FAIL(expr, ahead, "<expression>");
        return new_anode_binary(TAU_ANODE_ASSIGN_BIT_AND_STMT, stmt_token, node, expr);
      } else if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_PIPE_EQ, TAU_KEYWORD_NONE)) {
        expr = parse_expr(ahead);
        MUST_OR_FAIL(expr, ahead, "<expression>");
        return new_anode_binary(TAU_ANODE_ASSIGN_BIT_OR_STMT, stmt_token, node, expr);
      } else if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_CIRC_EQ, TAU_KEYWORD_NONE)) {
        expr = parse_expr(ahead);
        MUST_OR_FAIL(expr, ahead, "<expression>");
        return new_anode_binary(TAU_ANODE_ASSIGN_BIT_XOR_STMT, stmt_token, node, expr);
      } else {
        tau_log(TAU_LOG_LEVEL_ERROR, ahead->loc,
                "unexpected `%*.s`, was expecting <call statement> or <assign statement>", ahead->len, ahead->buf);
        goto handle_fail;
      }
    }
  }

  return NULL;
handle_fail:
  if (node != NULL) {
    tau_anode_free(node);
  }

  if (expr != NULL) {
    tau_anode_free(expr);
  }

  return NULL;
}

// if_stmt = "if", expr, block, { "elif", expr, block }, [ "else" block ];
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_if_stmt(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_if_stmt: ahead cannot be null");
  struct tau_anode *if_stmt = NULL;
  struct tau_anode *aux_branch = NULL;
  struct tau_token if_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_KEYWORD, TAU_PUNCT_NONE, TAU_KEYWORD_IF)) {
    if_stmt = new_anode_stack(TAU_ANODE_IF_BRANCH_STMT, if_token);

    // Add main if branch
    struct tau_anode *main_cond = parse_expr(ahead);
    MUST_OR_FAIL(main_cond, ahead, "<expression>");
    struct tau_anode *main_block = parse_block(ahead);
    MUST_OR_FAIL(main_block, ahead, "<block>");

    aux_branch = new_anode_binary(TAU_ANODE_THEN_CASE, if_token, main_cond, main_block);
    tau_ptr_stack_push(if_stmt->stack, aux_branch, tau_anode_free);

    // Add all elif-s if present
    while (true) {
      struct tau_token elif_token = *ahead;
      if (match_and_consume(ahead, TAU_TOKEN_TYPE_KEYWORD, TAU_PUNCT_NONE, TAU_KEYWORD_ELIF)) {
        struct tau_anode *elif_cond = parse_expr(ahead);
        MUST_OR_FAIL(elif_cond, ahead, "<expression>");
        struct tau_anode *elif_block = parse_expr(ahead);
        MUST_OR_FAIL(elif_block, ahead, "<block>");
        aux_branch = new_anode_binary(TAU_ANODE_ELIF_CASE, elif_token, elif_cond, elif_block);
        tau_ptr_stack_push(if_stmt->stack, aux_branch, tau_anode_free);
        continue;
      }

      break;
    }

    // Add else if present
    struct tau_anode *else_block = parse_block(ahead);
    if (else_block != NULL) {
      aux_branch = new_anode_unary(TAU_ANODE_ELSE_CASE, if_token, else_block);
      tau_ptr_stack_push(if_stmt->stack, aux_branch, tau_anode_free);
    }

    return if_stmt;
  }

  return NULL;
handle_fail:
  if (if_stmt != NULL) {
    tau_anode_free(if_stmt);
  }

  return NULL;
}

// loop_stmt = "loop" block;
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_loop_stmt(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_loop_stmt: ahead cannot be null");
  struct tau_token loop_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_KEYWORD, TAU_PUNCT_NONE, TAU_KEYWORD_LOOP)) {
    struct tau_anode *block = parse_block(ahead);
    MUST_OR_RETURN(block, ahead, "<block>", NULL);
    return new_anode_unary(TAU_ANODE_LOOP_BRANCH_STMT, loop_token, block);
  }

  return NULL;
}

// statement = break_stmt | continue_stmt | return_stmt | assign_or_call_stmt | if_stmt | loop_stmt;
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_statement(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_statement: ahead cannot be null");
  struct tau_anode *node = NULL;

  node = parse_break_stmt(ahead);
  if (node != NULL) {
    return node;
  }

  node = parse_continue_stmt(ahead);
  if (node != NULL) {
    return node;
  }

  node = parse_return_stmt(ahead);
  if (node != NULL) {
    return node;
  }

  node = parse_assign_or_call_stmt(ahead);
  if (node != NULL) {
    return node;
  }

  node = parse_if_stmt(ahead);
  if (node != NULL) {
    return node;
  }

  node = parse_loop_stmt(ahead);
  if (node != NULL) {
    return node;
  }

  return NULL;
}

// expr = log_or_expr;
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_expr: ahead cannot be null");
  return parse_log_or_expr(ahead);
}

// log_or_expr = log_and_expr, { "||", log_and_expr };
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_log_or_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_log_or_expr: ahead cannot be null");
  struct tau_anode *node = parse_log_and_expr(ahead);
  while (true) {
    struct tau_token operator_token = *ahead;
    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_D_PIPE, TAU_KEYWORD_NONE)) {
      struct tau_anode *right = parse_log_and_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      node = new_anode_binary(TAU_ANODE_LOG_OR_EXPR, operator_token, node, right);
      continue;
    }

    break;
  }

  return node;
handle_fail:
  if (node != NULL) {
    tau_anode_free(node);
  }

  return NULL;
}

// log_and_expr = rel_expr, { "&&", rel_expr };
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_log_and_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_log_and_expr: ahead cannot be null");
  struct tau_anode *node = parse_rel_expr(ahead);
  while (true) {
    struct tau_token operator_token = *ahead;
    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_D_AMP, TAU_KEYWORD_NONE)) {
      struct tau_anode *right = parse_rel_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      node = new_anode_binary(TAU_ANODE_LOG_AND_EXPR, operator_token, node, right);
      continue;
    }

    break;
  }

  return node;
handle_fail:
  if (node != NULL) {
    tau_anode_free(node);
  }

  return NULL;
}

// rel_expr = cmp_expr, { "==", cmp_expr | "!=", cmp_expr };
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_rel_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_rel_expr: ahead cannot be null");
  struct tau_anode *node = parse_cmp_expr(ahead);
  while (true) {
    struct tau_token operator_token = *ahead;
    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_D_EQ, TAU_KEYWORD_NONE)) {
      struct tau_anode *right = parse_cmp_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      node = new_anode_binary(TAU_ANODE_REL_EQ_EXPR, operator_token, node, right);
      continue;
    }

    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_BANG_EQ, TAU_KEYWORD_NONE)) {
      struct tau_anode *right = parse_cmp_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      node = new_anode_binary(TAU_ANODE_REL_NE_EXPR, operator_token, node, right);
      continue;
    }

    break;
  }

  return node;
handle_fail:
  if (node != NULL) {
    tau_anode_free(node);
  }

  return NULL;
}

// cmp_expr = bit_or_expr, { ">", bit_or_expr | "<", bit_or_expr | ">=", bit_or_expr | "<=", bit_or_expr};
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_cmp_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_cmp_expr: ahead cannot be null");
  struct tau_anode *node = parse_bit_or_expr(ahead);
  while (true) {
    struct tau_token operator_token = *ahead;
    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_GT, TAU_KEYWORD_NONE)) {
      struct tau_anode *right = parse_bit_or_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      node = new_anode_binary(TAU_ANODE_CMP_GT_EXPR, operator_token, node, right);
      continue;
    }

    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_LT, TAU_KEYWORD_NONE)) {
      struct tau_anode *right = parse_bit_or_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      node = new_anode_binary(TAU_ANODE_CMP_LT_EXPR, operator_token, node, right);
      continue;
    }

    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_GT_EQ, TAU_KEYWORD_NONE)) {
      struct tau_anode *right = parse_bit_or_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      node = new_anode_binary(TAU_ANODE_CMP_GE_EXPR, operator_token, node, right);
      continue;
    }

    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_LT_EQ, TAU_KEYWORD_NONE)) {
      struct tau_anode *right = parse_bit_or_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      node = new_anode_binary(TAU_ANODE_CMP_LE_EXPR, operator_token, node, right);
      continue;
    }

    break;
  }

  return node;
handle_fail:
  if (node != NULL) {
    tau_anode_free(node);
  }

  return NULL;
}

// bit_or_expr = bit_and_expr, { "|", bit_and_expr | "^", bit_and_expr };
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_bit_or_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_bit_or_expr: ahead cannot be null");
  struct tau_anode *node = parse_bit_and_expr(ahead);
  while (true) {
    struct tau_token operator_token = *ahead;
    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_PIPE, TAU_KEYWORD_NONE)) {
      struct tau_anode *right = parse_bit_and_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      node = new_anode_binary(TAU_ANODE_BIT_OR_EXPR, operator_token, node, right);
      continue;
    }

    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_CIRC, TAU_KEYWORD_NONE)) {
      struct tau_anode *right = parse_bit_and_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      node = new_anode_binary(TAU_ANODE_BIT_XOR_EXPR, operator_token, node, right);
      continue;
    }

    break;
  }

  return node;
handle_fail:
  if (node != NULL) {
    tau_anode_free(node);
  }

  return NULL;
}

// bit_and_expr = bit_shift_expr, { "&", bit_shift_expr };
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_bit_and_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_bit_and_expr: ahead cannot be null");
  struct tau_anode *node = parse_bit_shift_expr(ahead);
  while (true) {
    struct tau_token operator_token = *ahead;
    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_AMP, TAU_KEYWORD_NONE)) {
      struct tau_anode *right = parse_bit_shift_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      node = new_anode_binary(TAU_ANODE_BIT_AND_EXPR, operator_token, node, right);
      continue;
    }

    break;
  }

  return node;
handle_fail:
  if (node != NULL) {
    tau_anode_free(node);
  }

  return NULL;
}

// bit_shift_expr = sum_expr, { ">>", sum_expr | "<<", sum_expr };
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_bit_shift_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_bit_shift_expr: ahead cannot be null");
  struct tau_anode *node = parse_sum_expr(ahead);
  while (true) {
    struct tau_token operator_token = *ahead;
    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_D_GT, TAU_KEYWORD_NONE)) {
      struct tau_anode *right = parse_sum_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      node = new_anode_binary(TAU_ANODE_BIT_RSH_EXPR, operator_token, node, right);
      continue;
    }

    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_D_LT, TAU_KEYWORD_NONE)) {
      struct tau_anode *right = parse_sum_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      node = new_anode_binary(TAU_ANODE_BIT_LSH_EXPR, operator_token, node, right);
      continue;
    }

    break;
  }

  return node;
handle_fail:
  if (node != NULL) {
    tau_anode_free(node);
  }

  return NULL;
}

// sum_expr = mul_expr, { "+", mul_expr | "-", mul_expr  };
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_sum_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_sum_expr: ahead cannot be null");
  struct tau_anode *node = parse_mul_expr(ahead);
  while (true) {
    struct tau_token operator_token = *ahead;
    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_PLUS, TAU_KEYWORD_NONE)) {
      struct tau_anode *right = parse_mul_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      node = new_anode_binary(TAU_ANODE_ADD_EXPR, operator_token, node, right);
      continue;
    }

    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_HYPHEN, TAU_KEYWORD_NONE)) {
      struct tau_anode *right = parse_mul_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      node = new_anode_binary(TAU_ANODE_SUB_EXPR, operator_token, node, right);
      continue;
    }

    break;
  }

  return node;
handle_fail:
  if (node != NULL) {
    tau_anode_free(node);
  }

  return NULL;
}

// mul_expr = unary_expr, { "*", unary_expr | "/", unary_expr | "%", unary_expr};
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_mul_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_mul_expr: ahead cannot be null");
  struct tau_anode *node = parse_unary_expr(ahead);
  while (true) {
    struct tau_token operator_token = *ahead;
    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_AST, TAU_KEYWORD_NONE)) {
      struct tau_anode *right = parse_unary_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      node = new_anode_binary(TAU_ANODE_MUL_EXPR, operator_token, node, right);
      continue;
    }

    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_SLASH, TAU_KEYWORD_NONE)) {
      struct tau_anode *right = parse_unary_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      node = new_anode_binary(TAU_ANODE_DIV_EXPR, operator_token, node, right);
      continue;
    }

    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_PCT, TAU_KEYWORD_NONE)) {
      struct tau_anode *right = parse_unary_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      node = new_anode_binary(TAU_ANODE_REM_EXPR, operator_token, node, right);
      continue;
    }

    break;
  }

  return node;
handle_fail:
  if (node != NULL) {
    tau_anode_free(node);
  }

  return NULL;
}

// unary_expr = { "+" | "-" | "&" | "!" | "~" }, tag_expr;
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_unary_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_unary_expr: ahead cannot be null");
  struct tau_anode *node = NULL;
  struct tau_anode *root = NULL;
  while (true) {
    struct tau_token unary_token = *ahead;
    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_PLUS, TAU_KEYWORD_NONE)) {
      if (node == NULL) {
        node = new_anode_unary(TAU_ANODE_UNARY_PLUS_EXPR, unary_token, NULL);
        root = node;
      } else {
        node->left = new_anode_unary(TAU_ANODE_UNARY_PLUS_EXPR, unary_token, NULL);
        node = node->left;
      }
      continue;
    }

    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_HYPHEN, TAU_KEYWORD_NONE)) {
      if (node == NULL) {
        node = new_anode_unary(TAU_ANODE_UNARY_MINUS_EXPR, unary_token, NULL);
        root = node;
      } else {
        node->left = new_anode_unary(TAU_ANODE_UNARY_MINUS_EXPR, unary_token, NULL);
        node = node->left;
      }
      continue;
    }

    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_AMP, TAU_KEYWORD_NONE)) {
      if (node == NULL) {
        node = new_anode_unary(TAU_ANODE_UNARY_REF_EXPR, unary_token, NULL);
        root = node;
      } else {
        node->left = new_anode_unary(TAU_ANODE_UNARY_REF_EXPR, unary_token, NULL);
        node = node->left;
      }
      continue;
    }

    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_BANG, TAU_KEYWORD_NONE)) {
      if (node == NULL) {
        node = new_anode_unary(TAU_ANODE_UNARY_LOG_NOT_EXPR, unary_token, NULL);
        root = node;
      } else {
        node->left = new_anode_unary(TAU_ANODE_UNARY_LOG_NOT_EXPR, unary_token, NULL);
        node = node->left;
      }
      continue;
    }

    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_TILDE, TAU_KEYWORD_NONE)) {
      if (node == NULL) {
        node = new_anode_unary(TAU_ANODE_UNARY_BIT_NOT_EXPR, unary_token, NULL);
        root = node;
      } else {
        node->left = new_anode_unary(TAU_ANODE_UNARY_BIT_NOT_EXPR, unary_token, NULL);
        node = node->left;
      }
      continue;
    }

    break;
  }

  if (node != NULL) {
    struct tau_anode *operand = parse_tag_expr(ahead);
    MUST_OR_FAIL(operand, ahead, "<expression>");
    node->left = operand;
    return root;
  }

  return parse_tag_expr(ahead);
handle_fail:
  if (root != NULL) {
    tau_anode_free(root);
  }

  return NULL;
}

// tag_expr = call_expr, { "'", call_expr };
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_tag_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_tag_expr: ahead cannot be null");
  struct tau_anode *node = parse_call_expr(ahead);
  while (true) {
    struct tau_token operator_token = *ahead;
    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_APOS, TAU_KEYWORD_NONE)) {
      struct tau_anode *right = parse_call_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      node = new_anode_binary(TAU_ANODE_TAG_EXPR, operator_token, node, right);
      continue;
    }

    break;
  }

  return node;
handle_fail:
  if (node != NULL) {
    tau_anode_free(node);
  }

  return NULL;
}

// call_expr = data_lookup_expr, { passing_args };
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_call_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_call_expr: ahead cannot be null");
  struct tau_anode *node = parse_data_lookup_expr(ahead);
  while (true) {
    struct tau_token call_token = *ahead;
    struct tau_anode *arg = parse_passing_args(ahead);
    if (arg != NULL) {
      node = new_anode_binary(TAU_ANODE_CALL_EXPR, call_token, node, arg);
      continue;
    }

    break;
  }

  return node;
}

// data_lookup_expr = type_lookup_expr, { ".", type_lookup_expr };
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_data_lookup_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_data_lookup_expr: ahead cannot be null");
  struct tau_anode *node = parse_type_lookup_expr(ahead);
  while (true) {
    struct tau_token operator_token = *ahead;
    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_DOT, TAU_KEYWORD_NONE)) {
      struct tau_anode *right = parse_type_lookup_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      node = new_anode_binary(TAU_ANODE_DATA_LOOKUP_EXPR, operator_token, node, right);
      continue;
    }

    break;
  }

  return node;
handle_fail:
  if (node != NULL) {
    tau_anode_free(node);
  }

  return NULL;
}

// type_lookup_expr = atom_expr { "::", atom_expr };
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_type_lookup_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_type_lookup_expr: ahead cannot be null");
  struct tau_anode *node = parse_atom_expr(ahead);
  while (true) {
    struct tau_token operator_token = *ahead;
    if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_D_COLON, TAU_KEYWORD_NONE)) {
      struct tau_anode *right = parse_atom_expr(ahead);
      MUST_OR_FAIL(right, ahead, "<expression>");
      node = new_anode_binary(TAU_ANODE_TYPE_LOOKUP_EXPR, operator_token, node, right);
      continue;
    }

    break;
  }

  return node;
handle_fail:
  if (node != NULL) {
    tau_anode_free(node);
  }

  return NULL;
}

// atom_expr = "(" expr ")" | identifier | literal;
// NOLINTNEXTLINE(misc-no-recursion)
struct tau_anode *parse_atom_expr(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_atom_expr: ahead cannot be null");
  struct tau_anode *node = NULL;

  struct tau_token literal_token = *ahead;
  bool is_literal = (match_and_consume(ahead, TAU_TOKEN_TYPE_INT_LIT, TAU_PUNCT_NONE, TAU_KEYWORD_NONE) ||
                     match_and_consume(ahead, TAU_TOKEN_TYPE_FLT_LIT, TAU_PUNCT_NONE, TAU_KEYWORD_NONE) ||
                     match_and_consume(ahead, TAU_TOKEN_TYPE_STR_LIT, TAU_PUNCT_NONE, TAU_KEYWORD_NONE) ||
                     match_and_consume(ahead, TAU_TOKEN_TYPE_UNI_LIT, TAU_PUNCT_NONE, TAU_KEYWORD_NONE) ||
                     match_and_consume(ahead, TAU_TOKEN_TYPE_NIL_LIT, TAU_PUNCT_NONE, TAU_KEYWORD_NONE) ||
                     match_and_consume(ahead, TAU_TOKEN_TYPE_BOL_LIT, TAU_PUNCT_NONE, TAU_KEYWORD_NONE));

  if (match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_LPAR, TAU_KEYWORD_NONE)) {
    node = parse_expr(ahead);
    MUST_OR_FAIL(match_and_consume(ahead, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_RPAR, TAU_KEYWORD_NONE), ahead,
                 "<closing `)`>");
    return node;
  } else if (is_literal) {
    return new_anode(TAU_ANODE_LITERAL, literal_token);
  }

  return parse_identifier(ahead);
handle_fail:
  if (node != NULL) {
    tau_anode_free(node);
  }

  return NULL;
}

struct tau_anode *parse_identifier(struct tau_token *ahead) {
  assert(ahead != NULL && "parse_atom_expr: ahead cannot be null");
  struct tau_token identifier_token = *ahead;
  if (match_and_consume(ahead, TAU_TOKEN_TYPE_IDENTIFIER, TAU_PUNCT_NONE, TAU_KEYWORD_NONE)) {
    return new_anode(TAU_ANODE_IDENTIFIER, identifier_token);
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////// PUBLIC FUNCTIONS
struct tau_anode *tau_parse(const char *buf_name, const char *buf_data, size_t buf_size) {
  assert(buf_data != NULL && "tau_parse: buf_data cannot be null");
  struct tau_token starting = tau_token_start(buf_name, buf_data, buf_size);
  struct tau_token token = tau_token_next(starting);

  struct tau_anode *compilation_unit = parse_compilation_unit(&token);
  if (compilation_unit != NULL) {
    tau_log(TAU_LOG_LEVEL_ERROR, token.loc, "unexpected `%.*s`, was expecting <compilation unit>");
    return NULL;
  }

  return compilation_unit;
}

void tau_anode_free(void *maybe_anode) {
  struct tau_anode *anode = (struct tau_anode *)maybe_anode;
  // TODO(fatmimir): properly release memory hold by this node
  free(anode);
}
