//
// Created on 12/21/22.
//

#include "parser.h"

#include <assert.h>
#include <malloc.h>

#include "log.h"

// Local declarations
struct tau_ast_compilation_unit *ast_compilation_unit_new(struct tau_loc loc);

struct tau_ast_module *ast_module_new(struct tau_loc loc);
void ast_module_free(void *maybe_ast_module);

struct tau_ast_static_lookup *ast_static_lookup_new(struct tau_loc loc);
void ast_static_lookup_free(void *maybe_ast_static_lookup);

// Support functions
enum MATCH_FLAG {
  MATCH_TYPE = 1 << 0,
  MATCH_PUNCT = 1 << 1,
  MATCH_KEYWORD = 1 << 2,
};

bool match(const struct tau_parser *parser, const enum tau_token_type token_type, const enum tau_punct punct,
           const enum tau_keyword keyword, const uint16_t flags) {
  struct tau_token cur = parser->head;
  if (flags & MATCH_TYPE && cur.type == token_type) {
    if (flags & MATCH_PUNCT) {
      return cur.punct == punct;
    }

    if (flags & MATCH_KEYWORD) {
      return cur.keyword == keyword;
    }

    return true;
  }
  return false;
}

void consume(struct tau_parser *parser) {
  parser->prev = parser->head;
  parser->head = tau_token_next(parser->head);
}

bool may_match_and_consume(struct tau_parser *parser, const enum tau_token_type token_type, const enum tau_punct punct,
                           const enum tau_keyword keyword, const uint16_t flags) {
  if (match(parser, token_type, punct, keyword, flags)) {
    consume(parser);
    return true;
  }

  return false;
}

bool must_match_and_consume(struct tau_parser *parser, const enum tau_token_type token_type, const enum tau_punct punct,
                            const enum tau_keyword keyword, const uint16_t flags) {
  if (!may_match_and_consume(parser, token_type, punct, keyword, flags)) {
    struct tau_token unexpected = parser->head;
    if (token_type == TAU_TOKEN_TYPE_PUNCT) {
      tau_log(TAU_LOG_LEVEL_ERROR, unexpected.loc, "unexpected token `%.*s`, was expecting: %s `%s`",
              (int)unexpected.len, unexpected.buf, tau_token_get_name(token_type), tau_token_get_punct_name(punct));
    } else if (token_type == TAU_TOKEN_TYPE_KEYWORD) {
      tau_log(TAU_LOG_LEVEL_ERROR, unexpected.loc, "unexpected token `%.*s`, was expecting: %s `%s`",
              (int)unexpected.len, unexpected.buf, tau_token_get_name(token_type), tau_token_get_keyword_name(keyword));
    } else {
      tau_log(TAU_LOG_LEVEL_ERROR, unexpected.loc, "unexpected token `%.*s`, was expecting: %s", (int)unexpected.len,
              unexpected.buf, tau_token_get_name(token_type));
    }
    return false;
  }

  return true;
}

// New/free functions for internal types
struct tau_ast_compilation_unit *ast_compilation_unit_new(struct tau_loc loc) {
  struct tau_ast_compilation_unit *ast_compilation_unit = calloc(1, sizeof(struct tau_ast_compilation_unit));
  ast_compilation_unit->loc = loc;
  return ast_compilation_unit;
}

void tau_ast_compilation_unit_free(void *maybe_ast_compilation_unit) {
  assert(maybe_ast_compilation_unit != NULL &&
         "tau_ast_compilation_unit_free: maybe_ast_compilation_unit cannot be null");
  struct tau_ast_compilation_unit *ast_compilation_unit = (struct tau_ast_compilation_unit *)maybe_ast_compilation_unit;
  if (ast_compilation_unit->decls != NULL) {
    tau_ptr_stack_free(ast_compilation_unit->decls);
  }

  if (ast_compilation_unit->module_decl != NULL) {
    ast_module_free(ast_compilation_unit->module_decl);
  }

  free(ast_compilation_unit);
}

struct tau_ast_module *ast_module_new(struct tau_loc loc) {
  struct tau_ast_module *ast_module = calloc(1, sizeof(struct tau_ast_module));
  ast_module->loc = loc;
  return ast_module;
}

void ast_module_free(void *maybe_ast_module) {
  assert(maybe_ast_module != NULL && "ast_module_free: maybe_ast_module cannot be null");
  struct tau_ast_module *ast_module = (struct tau_ast_module *)maybe_ast_module;
  if (ast_module->name != NULL) {
    ast_static_lookup_free(ast_module->name);
  }

  free(ast_module);
}

struct tau_ast_static_lookup *ast_static_lookup_new(struct tau_loc loc) {
  struct tau_ast_static_lookup *ast_static_lookup = calloc(1, sizeof(struct tau_ast_static_lookup));
  ast_static_lookup->identifiers = tau_ptr_stack_new();
  ast_static_lookup->loc = loc;
  return ast_static_lookup;
}

void ast_static_lookup_free(void *maybe_ast_static_lookup) {
  assert(maybe_ast_static_lookup != NULL && "ast_static_lookup_free: maybe_ast_static_lookup cannot be null");
  struct tau_ast_static_lookup *static_lookup = (struct tau_ast_static_lookup *)maybe_ast_static_lookup;
  if (static_lookup->identifiers != NULL) {
    tau_ptr_stack_free(static_lookup->identifiers);
  }

  free(static_lookup);
}

struct tau_ast_identifier *ast_identifier_new(struct tau_token data, struct tau_loc loc) {
  struct tau_ast_identifier *ast_identifier = calloc(1, sizeof(struct tau_ast_identifier));
  ast_identifier->loc = loc;
  ast_identifier->data = data;
  return ast_identifier;
}

void ast_identifier_free(void *maybe_ast_identifier) {
  assert(maybe_ast_identifier != NULL && "ast_identifier_free: maybe_ast_identifier cannot be null");
  free(maybe_ast_identifier);
}

// Parsing functions

// static_lookup = identifier, { "::", identifier };
struct tau_ast_static_lookup *may_parse_static_lookup(struct tau_parser *parser) {
  assert(parser != NULL && "may_parse_static_lookup: parser cannot be null");

  if (match(parser, TAU_TOKEN_TYPE_IDENTIFIER, TAU_PUNCT_NONE, TAU_KEYWORD_NONE, MATCH_TYPE)) {
    struct tau_ast_static_lookup *static_lookup = ast_static_lookup_new(parser->head.loc);
    do {
      // identifier
      struct tau_token identifier_data = parser->head;
      if (must_match_and_consume(parser, TAU_TOKEN_TYPE_IDENTIFIER, TAU_PUNCT_NONE, TAU_KEYWORD_NONE, MATCH_TYPE)) {
        struct tau_ast_identifier *identifier = ast_identifier_new(identifier_data, identifier_data.loc);
        tau_ptr_stack_push(static_lookup->identifiers, identifier, ast_identifier_free);
      }

      // "::"
    } while (may_match_and_consume(parser, TAU_TOKEN_TYPE_PUNCT, TAU_PUNCT_D_COLON, TAU_KEYWORD_NONE,
                                   MATCH_TYPE | MATCH_PUNCT));
    return static_lookup;
  }

  return NULL;
}

// module_decl = "module", static_lookup, eol;
struct tau_ast_module *must_parse_module_decl(struct tau_parser *parser) {
  assert(parser != NULL && "must_parse_module_decl: parser cannot be null");
  struct tau_loc module_loc = parser->head.loc;
  struct tau_ast_static_lookup *lookup = NULL;

  if (!must_match_and_consume(parser, TAU_TOKEN_TYPE_KEYWORD, TAU_PUNCT_NONE, TAU_KEYWORD_MODULE,
                              MATCH_TYPE | MATCH_KEYWORD)) {
    return NULL;
  }

  lookup = may_parse_static_lookup(parser);
  if (lookup == NULL) {
    tau_log(TAU_LOG_LEVEL_ERROR, parser->head.loc,
            "unexpected `%.*s`, was expecting <static lookup> (i.e identifier, ns::id)", parser->head.len,
            parser->head.buf);
    goto handle_fail;
  }

  if (!must_match_and_consume(parser, TAU_TOKEN_TYPE_EOL, TAU_PUNCT_NONE, TAU_KEYWORD_NONE, MATCH_TYPE)) {
    tau_log(TAU_LOG_LEVEL_ERROR, parser->head.loc, "unexpected `%.*s`, was expecting <end of line>", parser->head.len,
            parser->head.buf);
    goto handle_fail;
  }

  struct tau_ast_module *module = ast_module_new(module_loc);
  module->name = lookup;
  return module;

handle_fail:
  if (lookup != NULL) {
    ast_static_lookup_free(lookup);
  }

  return NULL;
}

// decls = { (extern_decl | let_decl | proc_decl), eol };
struct tau_ptr_stack *must_parse_decls(struct tau_parser *parser) {
  assert(parser != NULL && "must_parse_decls: parser cannot be null");
  return tau_ptr_stack_new();
}

// Public functions
struct tau_parser *tau_parser_new(const char *buf_name, const char *buf_data, size_t buf_size) {
  struct tau_parser *parser = calloc(1, sizeof(struct tau_parser));
  parser->prev = tau_token_start(buf_name, buf_data, buf_size);
  parser->head = tau_token_next(parser->prev);
  parser->strict_mode = true;
  return parser;
}

// compilation_unit = module_decl, decls eof;
struct tau_ast_compilation_unit *tau_parse_compilation_unit(struct tau_parser *parser) {
  assert(parser != NULL && "tau_parse_compilation_unit: parser cannot be null");
  struct tau_loc compilation_unit_loc = parser->head.loc;
  struct tau_ast_module *module = NULL;
  struct tau_ptr_stack *decls = NULL;

  module = must_parse_module_decl(parser);
  if (module == NULL) {
    tau_log(TAU_LOG_LEVEL_ERROR, parser->head.loc, "unexpected `%.*s`, was expecting <module decl>", parser->head.len,
            parser->head.buf);
    goto handle_fail;
  }

  decls = must_parse_decls(parser);
  if (decls == NULL || (parser->strict_mode && tau_ptr_stack_is_empty(decls))) {
    tau_log(TAU_LOG_LEVEL_ERROR, parser->head.loc, "unexpected %s `%.*s`, was expecting at least one declaration",
            tau_token_get_name(parser->head.type), parser->head.len, parser->head.buf);
    goto handle_fail;
  }

  must_match_and_consume(parser, TAU_TOKEN_TYPE_EOF, TAU_PUNCT_NONE, TAU_KEYWORD_NONE, MATCH_TYPE);
  struct tau_ast_compilation_unit *compilation_unit = ast_compilation_unit_new(compilation_unit_loc);
  compilation_unit->module_decl = module;
  compilation_unit->decls = decls;
  return compilation_unit;

handle_fail:
  if (module != NULL) {
    ast_module_free(module);
  }

  if (decls != NULL) {
    tau_ptr_stack_free(decls);
  }

  return NULL;
}

void tau_parser_free(void *maybe_parser) {
  assert(maybe_parser != NULL && "tau_parser_free: maybe_parser cannot be null");
  free(maybe_parser);
}
