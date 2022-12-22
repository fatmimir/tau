//
// Created on 12/21/22.
//

#include "parser.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "utf8.h"

#define ASSERT_PARSER(p)                                                     \
  do {                                                                       \
    assert((p) != NULL && "parser: cannot be null");                         \
    assert((p)->buf_size > 0 && "parser->buf_len: cannot be zero");          \
    assert((p)->cur_offset != NULL && "parser->cur_offset: cannot be null"); \
    assert((p)->cur_offset <= (p)->buf_base + (p)->buf_size + 1 &&           \
           "parser->cur_offset: cannot be outside of range of buffer");      \
  } while (0)
#define CHECK_BOUNDS_OR_FALSE(p)                            \
  do {                                                      \
    if ((p)->cur_offset - (p)->buf_base >= (p)->buf_size) { \
      return false;                                         \
    }                                                       \
  } while (0)
#define CHECK_BOUNDS_OR_NULL(p)                             \
  do {                                                      \
    if ((p)->cur_offset - (p)->buf_base >= (p)->buf_size) { \
      return NULL;                                          \
    }                                                       \
  } while (0)
#define CHECK_BOUNDS_OR_RETURN(p)                           \
  do {                                                      \
    if ((p)->cur_offset - (p)->buf_base >= (p)->buf_size) { \
      return;                                               \
    }                                                       \
  } while (0)
#define MUST(v, p, e)                                                                                               \
  do {                                                                                                              \
    if (!(v)) {                                                                                                     \
      if (*p->cur_offset == '\n') {                                                                                 \
        tau_log(TAU_LOG_LEVEL_ERROR, (p)->loc, "unexpected character <new line>, was expecting %s", (e));           \
      } else if (*p->cur_offset == '\t') {                                                                          \
        tau_log(TAU_LOG_LEVEL_ERROR, (p)->loc, "unexpected character <tab>, was expecting %s", (e));                \
      } else if (*p->cur_offset == ' ') {                                                                           \
        tau_log(TAU_LOG_LEVEL_ERROR, (p)->loc, "unexpected character <space>, was expecting %s", (e));              \
      } else if (*p->cur_offset == '\0') {                                                                          \
        tau_log(TAU_LOG_LEVEL_ERROR, (p)->loc, "unexpected character <end of file>, was expecting %s", (e));        \
      } else {                                                                                                      \
        tau_log(TAU_LOG_LEVEL_ERROR, (p)->loc, "unexpected character `%c`, was expecting %s", *p->cur_offset, (e)); \
      }                                                                                                             \
      return false;                                                                                                 \
    }                                                                                                               \
  } while (0)

#define EXP_SEQ_COUNT 3
#define SIMPLE_ESC_SEQ_COUNT 9

// TERMINALS
// eof = ? end of file ?;
bool match_eof(const struct tau_parser *parser) {
  assert(parser != NULL && "parser: cannot be null");
  assert(parser->buf_size > 0 && "parser->buf_len: cannot be zero");
  assert(parser->cur_offset != NULL && "parser->cur_offset: cannot be null");
  if (parser->cur_offset - parser->buf_base > parser->buf_size) {
    return true;
  }

  return *parser->cur_offset == '\0';
}

// letter = ? any alpha letter ?;
bool match_letter(const struct tau_parser *parser, size_t *uc_len) {
  ASSERT_PARSER(parser);
  CHECK_BOUNDS_OR_FALSE(parser);
  assert(uc_len != NULL && "uc_len: cannot be null");

  uint32_t uc = 0;
  *uc_len = tau_dec_bytes_to_cp(parser->cur_offset, &uc);

  // TODO(fatmimir): review which ranges are valid letters for identifiers
  return (uc >= 0x0041 && uc <= 0x005A) || (uc >= 0x0061 && uc <= 0x007A);
}

// space = ? any white space character ?;
bool match_space(const struct tau_parser *parser, size_t *uc_len) {
  ASSERT_PARSER(parser);
  CHECK_BOUNDS_OR_FALSE(parser);
  assert(uc_len != NULL && "uc_len: cannot be null");

  uint32_t uc = 0;
  *uc_len = tau_dec_bytes_to_cp(parser->cur_offset, &uc);

  // TODO(fatmimir): review which ranges are valid spaces
  return uc == 0x0020 || uc == 0x0009 || uc == 0x000A || uc == 0x000B || uc == 0x000D;
}

// space = ? any white space character ?;
bool match_unicode(const struct tau_parser *parser, size_t *uc_len) {
  ASSERT_PARSER(parser);
  CHECK_BOUNDS_OR_FALSE(parser);
  assert(uc_len != NULL && "uc_len: cannot be null");

  uint32_t _discard = 0;
  *uc_len = tau_dec_bytes_to_cp(parser->cur_offset, &_discard);
  return *uc_len > 0 && *uc_len <= 4;
}

// new_line = ? any new line character ?;
bool match_newline(const struct tau_parser *parser, size_t *uc_len) {
  ASSERT_PARSER(parser);
  CHECK_BOUNDS_OR_FALSE(parser);

  uint32_t uc = 0;
  if (uc_len != NULL) {
    *uc_len = tau_dec_bytes_to_cp(parser->cur_offset, &uc);
  } else {
    tau_dec_bytes_to_cp(parser->cur_offset, &uc);
  }

  // TODO(fatmimir): review which ranges are valid new lines
  return uc == 0x000A || uc == 0x000B || uc == 0x000D;
}

// bin_digit = ? 0 .. 1 ?;
bool match_bin_digit(const struct tau_parser *parser, size_t *uc_len) {
  ASSERT_PARSER(parser);
  CHECK_BOUNDS_OR_FALSE(parser);
  assert(uc_len != NULL && "uc_len: cannot be null");

  uint32_t uc = 0;
  *uc_len = tau_dec_bytes_to_cp(parser->cur_offset, &uc);
  return uc == 0x0030 || uc == 0x0031;
}

// oct_digit = ? 0 .. 7 ?;
bool match_oct_digit(const struct tau_parser *parser, size_t *uc_len) {
  ASSERT_PARSER(parser);
  CHECK_BOUNDS_OR_FALSE(parser);
  assert(uc_len != NULL && "uc_len: cannot be null");

  uint32_t uc = 0;
  *uc_len = tau_dec_bytes_to_cp(parser->cur_offset, &uc);
  return uc >= 0x0030 && uc <= 0x0037;
}

// dec_digit = ? 0 .. 9 ?;
bool match_dec_digit(const struct tau_parser *parser, size_t *uc_len) {
  ASSERT_PARSER(parser);
  CHECK_BOUNDS_OR_FALSE(parser);
  assert(uc_len != NULL && "uc_len: cannot be null");

  uint32_t uc = 0;
  *uc_len = tau_dec_bytes_to_cp(parser->cur_offset, &uc);
  return uc >= 0x0030 && uc <= 0x0039;
}

// hex_digit = ? 0 .. 9 | A .. F | a .. f ?;
bool match_hex_digit(const struct tau_parser *parser, size_t *uc_len) {
  ASSERT_PARSER(parser);
  CHECK_BOUNDS_OR_FALSE(parser);
  assert(uc_len != NULL && "uc_len: cannot be null");

  uint32_t uc = 0;
  *uc_len = tau_dec_bytes_to_cp(parser->cur_offset, &uc);
  return (uc >= 0x0030 && uc <= 0x0039) || (uc >= 0x0041 && uc <= 0x005A) || (uc >= 0061 && uc <= 0x007A);
}

// eol = ";" | 0x037E*;
bool match_eol(const struct tau_parser *parser, size_t *uc_len) {
  ASSERT_PARSER(parser);
  CHECK_BOUNDS_OR_FALSE(parser);
  assert(uc_len != NULL && "uc_len: cannot be null");

  uint32_t uc = 0;
  *uc_len = tau_dec_bytes_to_cp(parser->cur_offset, &uc);

  // TODO(fatmimir): warn about greek question mark
  return uc == 0x003B || uc == 0x037E;
}

// UTILITIES

// utility to match a single sequence
bool match_str(const struct tau_parser *parser, const char *seq, size_t *match_len) {
  ASSERT_PARSER(parser);
  CHECK_BOUNDS_OR_FALSE(parser);
  size_t seq_len = strlen(seq);
  if (strncmp(parser->cur_offset, seq, seq_len) == 0) {
    if (match_len != NULL) {
      *match_len = seq_len;
    }

    return true;
  }

  return false;
}

// utility to match full sets of keywords or operands
bool match_any(const struct tau_parser *parser, const char **seqs, size_t seq_count, size_t *match_len) {
  ASSERT_PARSER(parser);
  CHECK_BOUNDS_OR_FALSE(parser);

  for (size_t i = 0; i < seq_count; i++) {
    size_t seq_len = strlen(seqs[i]);
    if (strncmp(parser->cur_offset, seqs[i], seq_len) == 0) {
      if (match_len != NULL) {
        *match_len = seq_len;
      }

      return true;
    }
  }
  return false;
}

// utility to consume a size
void consume(struct tau_parser *parser, size_t len) {
  ASSERT_PARSER(parser);
  CHECK_BOUNDS_OR_RETURN(parser);
  parser->cur_offset += len;
  parser->loc.col += len;
}

bool match_and_consume_str(struct tau_parser *parser, const char *seq) {
  size_t len = 0;
  if (match_str(parser, seq, &len)) {
    consume(parser, len);
    return true;
  }

  return false;
}

// SPACES

// maybe_spaces = [{ space }];
bool tau_parse_spaces(struct tau_parser *parser) {
  ASSERT_PARSER(parser);
  CHECK_BOUNDS_OR_FALSE(parser);
  size_t uc_len = 0;
  while (match_space(parser, &uc_len)) {
    if (match_newline(parser, NULL)) {
      parser->loc.row += 1;
      parser->loc.col = 0;
    }

    consume(parser, uc_len);
  }

  return uc_len > 0;
}

// spaces = { space };
bool must_tau_parse_spaces(struct tau_parser *parser) {
  ASSERT_PARSER(parser);
  CHECK_BOUNDS_OR_FALSE(parser);
  MUST(tau_parse_spaces(parser), parser, "<any space character>");
  return true;
}

// COMMENTS

// line_comment = maybe_spaces, "//", { unicode - new_line }, maybe_spaces;
bool tau_parse_line_comment(struct tau_parser *parser) {
  ASSERT_PARSER(parser);
  CHECK_BOUNDS_OR_FALSE(parser);

  size_t len = 0;
  tau_parse_spaces(parser);
  if (match_str(parser, "//", &len)) {
    do {
      if (match_newline(parser, NULL) || match_eof(parser)) {
        break;
      }

      consume(parser, len);
    } while (match_unicode(parser, &len));

    return true;
  }

  return false;
}

// block_comment = maybe_spaces, "/*", { unicode }, "*/", maybe_spaces;
bool tau_parse_block_comment(struct tau_parser *parser) {
  ASSERT_PARSER(parser);
  CHECK_BOUNDS_OR_FALSE(parser);

  size_t len = 0;
  tau_parse_spaces(parser);
  if (match_str(parser, "/*", &len)) {
    do {
      if (match_eof(parser) || match_str(parser, "*/", NULL)) {
        break;
      }

      consume(parser, len);
    } while (match_unicode(parser, &len));

    MUST(match_and_consume_str(parser, "*/"), parser, "<block comment closure `*/`>");
    tau_parse_spaces(parser);
    return true;
  }

  return false;
}

// comment = line | block_comment;
// TODO(fatmimir): Maybe produce a comment node?
bool tau_parse_comment(struct tau_parser *parser) {
  ASSERT_PARSER(parser);
  CHECK_BOUNDS_OR_FALSE(parser);

  if (tau_parse_line_comment(parser) || tau_parse_block_comment(parser)) {
    return true;
  }

  return false;
}

// LITERALS

/*
 num_literal	= "0b", { bin_digit }
                | "0o", { oct_digit }
                | "0x", { hex_digit }
                | { dec_digit } [".", { dec_digit }, [ ("e" | "e+" | "e-"), { dec_digit } ]];
 */
struct tau_literal *tau_parse_num_literal(struct tau_parser *parser) {
  ASSERT_PARSER(parser);
  CHECK_BOUNDS_OR_NULL(parser);

  const char *exp_seqs[] = {"e", "e+", "e-"};
  static_assert(EXP_SEQ_COUNT == sizeof(exp_seqs) / sizeof(const char *) && "outdated sequence count");

  size_t len = 0;
  struct tau_literal *literal = NULL;
  const char *literal_start = NULL;
  if (match_str(parser, "0b", &len)) {
    // "0b", { bin_digit }
    literal = tau_literal_new();
    literal->loc = parser->loc;
    literal_start = parser->cur_offset + 2;
    do {
      consume(parser, len);
    } while (match_bin_digit(parser, &len));

    literal->type = TAU_LITERAL_NUM_INT;
    literal->as_int = strtol(literal_start, NULL, 2);
    return literal;
  } else if (match_str(parser, "0o", &len)) {
    // "0o", { oct_digit }
    literal = tau_literal_new();
    literal->loc = parser->loc;
    literal_start = parser->cur_offset + 2;
    do {
      consume(parser, len);
    } while (match_oct_digit(parser, &len));

    literal->type = TAU_LITERAL_NUM_INT;
    literal->as_int = strtol(literal_start, NULL, 8);
    return literal;
  } else if (match_str(parser, "0x", &len)) {
    // "0x", { hex_digit }
    literal = tau_literal_new();
    literal->loc = parser->loc;
    literal_start = parser->cur_offset + 2;
    do {
      consume(parser, len);
    } while (match_hex_digit(parser, &len));

    literal->type = TAU_LITERAL_NUM_INT;
    literal->as_int = strtol(literal_start, NULL, 16);
    return literal;
  } else if (match_dec_digit(parser, &len)) {
    // { dec_digit }
    literal = tau_literal_new();
    literal->loc = parser->loc;
    literal_start = parser->cur_offset;
    do {
      consume(parser, len);
    } while (match_dec_digit(parser, &len));

    // [".", { dec_digit }, [ ("e" | "e+" | "e-"), { dec_digit } ]]
    if (match_str(parser, ".", &len)) {
      do {
        consume(parser, len);
      } while (match_dec_digit(parser, &len));

      // [ ("e" | "e+" | "e-"), { dec_digit } ]
      if (match_any(parser, exp_seqs, EXP_SEQ_COUNT, &len)) {
        do {
          consume(parser, len);
        } while (match_dec_digit(parser, &len));
      }

      literal->type = TAU_LITERAL_NUM_FLT;
      literal->as_flt = strtod(literal_start, NULL);
    } else {
      literal->type = TAU_LITERAL_NUM_INT;
      literal->as_int = strtol(literal_start, NULL, 10);
    }

    return literal;
  }

  return literal;
}

/*
 esc_seq : "\'" | "\"" | "\\" | "\n" | "\r" | "\t" | "\b" | "\f" | "\v" | "\0"
               | "\x", hex_digit, hex_digit
               | "\u", hex_digit, hex_digit, hex_digit, hex_digit
               | "\U", hex_digit, hex_digit, hex_digit, hex_digit, hex_digit, hex_digit, hex_digit, hex_digit;
 */
bool match_esc_seq(struct tau_parser *parser, size_t *len) {
  ASSERT_PARSER(parser);
  CHECK_BOUNDS_OR_FALSE(parser);

  const char *simple_esc_seqs[] = {"\\'", "\\\"", "\\n", "\\r", "\\t", "\\b", "\\f", "\\v", "\\0"};
  static_assert(SIMPLE_ESC_SEQ_COUNT == sizeof(simple_esc_seqs) / sizeof(const char *) && "outdated sequence count");
  if (match_any(parser, simple_esc_seqs, SIMPLE_ESC_SEQ_COUNT, len)) {
    return true;
  } else if (match_str(parser, "\\x", len)) {
    size_t inc = 0;
    for (size_t i = 0; i < 2; i++) {
      MUST(match_hex_digit(parser, &inc), parser, "<hex digit>");
      *len += inc;
    }
  } else if (match_str(parser, "\\u", len)) {
    size_t inc = 0;
    for (size_t i = 0; i < 4; i++) {
      MUST(match_hex_digit(parser, &inc), parser, "<hex digit>");
      *len += inc;
    }
  } else if (match_str(parser, "\\U", len)) {
    size_t inc = 0;
    for (size_t i = 0; i < 8; i++) {
      MUST(match_hex_digit(parser, &inc), parser, "<hex digit>");
      *len += inc;
    }
  }

  return false;
}

// str_literal = """, { (unicode - new_line) | esc_seq }, """;
struct tau_literal *tau_parse_str_literal(struct tau_parser *parser) {
  ASSERT_PARSER(parser);
  CHECK_BOUNDS_OR_NULL(parser);

  size_t len = 0;
  if (match_str(parser, "\"", &len)) {
    struct tau_literal *literal = tau_literal_new();
    const char *literal_start = NULL;

    literal->loc = parser->loc;
    literal_start = parser->cur_offset;
    consume(parser, len);

    do {
      if (match_str(parser, "\"", NULL)) {
        break;
      }

      if (match_esc_seq(parser, &len)) {
        consume(parser, len);
        continue;
      }

      consume(parser, len);
    } while (match_unicode(parser, &len));

    // closing string
    MUST(match_and_consume_str(parser, "\""), parser, "<closing string '\"'>");

    // Create the literal
    size_t literal_size = parser->cur_offset - literal_start;
    literal->type = TAU_LITERAL_STR;
    literal->as_str = calloc(literal_size, sizeof(char *));
    strncpy(literal->as_str, literal_start, literal_size);
    return literal;
  }

  return NULL;
}

/*
  bol_literal = "true" | "false";
  nil_literal = "nil";
  unit_literal = "unit";
 */
struct tau_literal *tau_parse_bol_nil_or_unit_literal(struct tau_parser *parser) {
  ASSERT_PARSER(parser);
  CHECK_BOUNDS_OR_NULL(parser);

  struct tau_literal *literal = NULL;
  if (match_and_consume_str(parser, "true")) {
    literal = tau_literal_new();
    literal->type = TAU_LITERAL_BOL;
    literal->as_bol = true;
  } else if (match_and_consume_str(parser, "false")) {
    literal = tau_literal_new();
    literal->type = TAU_LITERAL_BOL;
    literal->as_bol = false;
  } else if (match_and_consume_str(parser, "nil")) {
    literal = tau_literal_new();
    literal->type = TAU_LITERAL_NIL;
  } else if (match_and_consume_str(parser, "unit")) {
    literal = tau_literal_new();
    literal->type = TAU_LITERAL_UNIT;
  }

  return literal;
}

// literal 	= num_literal | str_literal | bol_literal | nil_literal | unit_literal;
struct tau_literal *tau_parse_literal(struct tau_parser *parser) {
  ASSERT_PARSER(parser);
  CHECK_BOUNDS_OR_NULL(parser);

  struct tau_literal *literal = NULL;
  literal = tau_parse_num_literal(parser);
  if (literal != NULL) {
    return literal;
  }

  literal = tau_parse_str_literal(parser);
  if (literal != NULL) {
    return literal;
  }

  literal = tau_parse_bol_nil_or_unit_literal(parser);
  if (literal != NULL) {
    return literal;
  }

  return NULL;
}

// LOOKUPS

// identifier = letter, { letter | dec_digit | "_" | "$" };
struct tau_identifier *tau_parse_identifier(struct tau_parser *parser) { return NULL; }

// PUBLIC FUNCTIONS
struct tau_parser *tau_parser_new(const char *name, const char *data, size_t size) {
  struct tau_parser *parser = calloc(1, sizeof(struct tau_parser));
  parser->loc.buf_name = name;
  parser->loc.col = 0;
  parser->loc.row = 0;
  parser->buf_base = data;
  parser->buf_size = size;
  parser->cur_offset = data;
  return parser;
}

void tau_parser_free(struct tau_parser *parser) {
  assert(parser != NULL && "parser: cannot be null");
  free(parser);
}

struct tau_literal *tau_literal_new() { return calloc(1, sizeof(struct tau_literal)); }

void tau_literal_free(struct tau_literal *literal) {
  assert(literal != NULL && "parser: cannot be null");
  switch (literal->type) {
    case TAU_LITERAL_STR:
      if (literal->as_str != NULL) {
        free(literal->as_str);
      }
      break;
    case TAU_LITERAL_NUM_INT:
    case TAU_LITERAL_BOL:
    case TAU_LITERAL_UNIT:
    case TAU_LITERAL_NIL:
    case TAU_LITERAL_NUM_FLT:
      break;
  }

  free(literal);
}

struct tau_identifier *tau_identifier_new() { return calloc(1, sizeof(struct tau_identifier)); }

void tau_identifier_free(struct tau_identifier *identifier) {
  assert(identifier != NULL && "parser: cannot be null");
  if (identifier->as_str != NULL) {
    free(identifier->as_str);
  }
  free(identifier);
}