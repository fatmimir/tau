//
// Created on 12/21/22.
//

#ifndef TAU_PARSER_H
#define TAU_PARSER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "common.h"
#include "lexer.h"
#include "ptr_stack.h"

struct tau_parser {
  struct tau_token head;
  struct tau_token prev;
  bool strict_mode;
};

struct tau_ast_identifier {
  struct tau_loc loc;
  struct tau_token data;
};

struct tau_ast_static_lookup {
  struct tau_ptr_stack *identifiers;
  struct tau_loc loc;
};

struct tau_ast_module {
  struct tau_ast_static_lookup *name;
  struct tau_loc loc;
};

struct tau_ast_compilation_unit {
  struct tau_ast_module *module_decl;
  struct tau_ptr_stack *decls;
  struct tau_loc loc;
};

struct tau_parser *tau_parser_new(const char *buf_name, const char *buf_data, size_t buf_size);
struct tau_ast_compilation_unit *tau_parse_compilation_unit(struct tau_parser *parser);
void tau_parser_free(void *maybe_parser);

void tau_ast_compilation_unit_free(void *maybe_ast_compilation_unit);

#endif  // TAU_PARSER_H
