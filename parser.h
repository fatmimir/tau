//
// Created on 12/21/22.
//

#ifndef TAU_PARSER_H
#define TAU_PARSER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "common.h"

struct tau_parser {
  const char *cur_offset;
  const char *buf_base;
  struct tau_loc loc;
  size_t buf_size;
};

enum tau_literal_type {
  TAU_LITERAL_NUM_INT,
  TAU_LITERAL_NUM_FLT,
  TAU_LITERAL_STR,
  TAU_LITERAL_BOL,
  TAU_LITERAL_NIL,
  TAU_LITERAL_UNIT,
};

struct tau_literal {
  union {
    uint64_t as_int;
    double as_flt;
    char *as_str;  // copy
    bool as_bol;
  };
  struct tau_loc loc;
  enum tau_literal_type type;
};

struct tau_identifier {
  char *as_str;  // copy
  struct tau_loc loc;
};

// parsers
bool tau_parse_spaces(struct tau_parser *parser);
bool tau_parse_line_comment(struct tau_parser *parser);
bool tau_parse_block_comment(struct tau_parser *parser);
bool tau_parse_comment(struct tau_parser *parser);
struct tau_literal *tau_parse_num_literal(struct tau_parser *parser);
struct tau_literal *tau_parse_str_literal(struct tau_parser *parser);
struct tau_literal *tau_parse_bol_nil_or_unit_literal(struct tau_parser *parser);
struct tau_literal *tau_parse_literal(struct tau_parser *parser);
struct tau_identifier *tau_parse_identifier(struct tau_parser *parser);

// allocators
struct tau_parser *tau_parser_new(const char *name, const char *data, size_t len);
void tau_parser_free(struct tau_parser *parser);

struct tau_literal *tau_literal_new();
void tau_literal_free(struct tau_literal *literal);

struct tau_identifier *tau_identifier_new();
void tau_identifier_free(struct tau_identifier *identifier);

#endif  // TAU_PARSER_H
