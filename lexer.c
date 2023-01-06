//
// Created on 12/24/22.
//

#include "lexer.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "log.h"
#include "uc_names.h"
#include "utf8.h"

#define DIGITS_BASE_PREFIX_LEN 2
#define EXPONENT_WITH_SIGN_PREFIX_LEN 2
#define ESCAPE_SEQUENCE_PREFIX_LEN 2
#define EXHAUSTIVE_PUNCT_TABLE_COUNT 44
#define EXHAUSTIVE_KEYWORD_TABLE_COUNT 17
#define EXHAUSTIVE_TOKEN_NAME_TABLE_COUNT 12

// TABLES
const char *punct_table[] = {
    [TAU_PUNCT_D_GT_EQ] = ">>=", [TAU_PUNCT_D_LT_EQ] = "<<=",  [TAU_PUNCT_D_EQ] = "==",    [TAU_PUNCT_D_GT] = ">>",
    [TAU_PUNCT_D_LT] = "<<",     [TAU_PUNCT_D_AMP] = "&&",     [TAU_PUNCT_D_PIPE] = "||",  [TAU_PUNCT_D_COLON] = "::",
    [TAU_PUNCT_COLON_EQ] = ":=", [TAU_PUNCT_BANG_EQ] = "!=",   [TAU_PUNCT_GT_EQ] = ">=",   [TAU_PUNCT_LT_EQ] = "<=",
    [TAU_PUNCT_PLUS_EQ] = "+=",  [TAU_PUNCT_HYPHEN_EQ] = "-=", [TAU_PUNCT_AST_EQ] = "*=",  [TAU_PUNCT_SLASH_EQ] = "/=",
    [TAU_PUNCT_PCT_EQ] = "%=",   [TAU_PUNCT_AMP_EQ] = "&=",    [TAU_PUNCT_PIPE_EQ] = "|=", [TAU_PUNCT_CIRC_EQ] = "^=",
    [TAU_PUNCT_LPAR] = "(",      [TAU_PUNCT_RPAR] = ")",       [TAU_PUNCT_LSBR] = "[",     [TAU_PUNCT_RSBR] = "]",
    [TAU_PUNCT_LCBR] = "{",      [TAU_PUNCT_RCBR] = "}",       [TAU_PUNCT_COLON] = ":",    [TAU_PUNCT_DOT] = ".",
    [TAU_PUNCT_COMMA] = ",",     [TAU_PUNCT_EQ] = "=",         [TAU_PUNCT_BANG] = "!",     [TAU_PUNCT_LT] = "<",
    [TAU_PUNCT_GT] = ">",        [TAU_PUNCT_PLUS] = "+",       [TAU_PUNCT_HYPHEN] = "-",   [TAU_PUNCT_AST] = "*",
    [TAU_PUNCT_SLASH] = "/",     [TAU_PUNCT_PCT] = "%",        [TAU_PUNCT_PIPE] = "|",     [TAU_PUNCT_AMP] = "&",
    [TAU_PUNCT_CIRC] = "^",      [TAU_PUNCT_TILDE] = "~",      [TAU_PUNCT_APOS] = "'",
};
static_assert(TAU_PUNCT_COUNT == EXHAUSTIVE_PUNCT_TABLE_COUNT && "outdated exhaustive punct table");

const char *keyword_table[] = {
    [TAU_KEYWORD_NIL] = "nil",       [TAU_KEYWORD_UNIT] = "unit",     [TAU_KEYWORD_TRUE] = "true",
    [TAU_KEYWORD_FALSE] = "false",   [TAU_KEYWORD_MODULE] = "module", [TAU_KEYWORD_EXTERN] = "extern",
    [TAU_KEYWORD_PROC] = "proc",     [TAU_KEYWORD_LET] = "let",       [TAU_KEYWORD_PROTOTYPE] = "prototype",
    [TAU_KEYWORD_IF] = "if",         [TAU_KEYWORD_ELIF] = "elif",     [TAU_KEYWORD_ELSE] = "else",
    [TAU_KEYWORD_WHILE] = "while",     [TAU_KEYWORD_BREAK] = "break",   [TAU_KEYWORD_CONTINUE] = "continue",
    [TAU_KEYWORD_RETURN] = "return",
};
static_assert(TAU_KEYWORD_COUNT == EXHAUSTIVE_KEYWORD_TABLE_COUNT && "outdated exhaustive keyword table");

const char *token_name_table[] = {
    [TAU_TOKEN_TYPE_NONE] = "(none)",
    [TAU_TOKEN_TYPE_EOF] = "<EOF>",
    [TAU_TOKEN_TYPE_EOL] = "<EOL>",
    [TAU_TOKEN_TYPE_INT_LIT] = "<int literal>",
    [TAU_TOKEN_TYPE_FLT_LIT] = "<float literal>",
    [TAU_TOKEN_TYPE_STR_LIT] = "<string literal>",
    [TAU_TOKEN_TYPE_BOL_LIT] = "<boolean literal>",
    [TAU_TOKEN_TYPE_NIL_LIT] = "<nil literal>",
    [TAU_TOKEN_TYPE_UNI_LIT] = "<unit literal>",
    [TAU_TOKEN_TYPE_PUNCT] = "<punct symbol>",
    [TAU_TOKEN_TYPE_KEYWORD] = "<keyword>",
    [TAU_TOKEN_TYPE_IDENTIFIER] = "<identifier>",
    [TAU_TOKEN_TYPE_COUNT] = "(invalid)",
};
static_assert(TAU_TOKEN_TYPE_COUNT == EXHAUSTIVE_TOKEN_NAME_TABLE_COUNT && "outdated exhaustive token name table");

bool is_space(const uint32_t uc) { return uc == UC_TAB || uc == UC_SPACE; }

bool is_newline(const uint32_t uc) { return uc == UC_NEW_LINE || uc == UC_CARRIAGE_RETURN; }

bool is_digit(const uint32_t uc, enum tau_num_base base) {
  switch (base) {
    case TAU_NUM_BASE_DEC:
      return uc >= UC_DIGIT_ZERO && uc <= UC_DIGIT_NINE;
    case TAU_NUM_BASE_BIN:
      return uc == UC_DIGIT_ZERO || uc == UC_DIGIT_ONE;
    case TAU_NUM_BASE_OCT:
      return uc >= UC_DIGIT_ZERO && uc <= UC_DIGIT_SEVEN;
    case TAU_NUM_BASE_HEX:
      return (uc >= UC_DIGIT_ZERO && uc <= UC_DIGIT_NINE) ||
             (uc >= UC_LATIN_CAPITAL_LETTER_A && uc <= UC_LATIN_CAPITAL_LETTER_F) ||
             (uc >= UC_LATIN_SMALL_LETTER_A && uc <= UC_LATIN_SMALL_LETTER_F);
  }
}

bool is_ident_start(const uint32_t uc) {
  return uc == UC_DOLLAR_SIGN || uc == UC_LOW_LINE ||
         (uc >= UC_LATIN_CAPITAL_LETTER_A && uc <= UC_LATIN_CAPITAL_LETTER_Z) ||
         (uc >= UC_LATIN_SMALL_LETTER_A && uc <= UC_LATIN_SMALL_LETTER_Z);
}

bool is_ident_body(const uint32_t uc) {
  return uc == UC_DOLLAR_SIGN || uc == UC_LOW_LINE || (uc >= UC_DIGIT_ZERO && uc <= UC_DIGIT_NINE) ||
         (uc >= UC_LATIN_CAPITAL_LETTER_A && uc <= UC_LATIN_CAPITAL_LETTER_Z) ||
         (uc >= UC_LATIN_SMALL_LETTER_A && uc <= UC_LATIN_SMALL_LETTER_Z);
}

void move_len_ahead(struct tau_token *cur, const char *ahead) {
  assert(ahead > cur->buf && "move_len_ahead: ahead is behind buffer base");
  size_t diff = ahead - cur->buf;
  assert(diff <= cur->rem && "move_len_ahead: not enough bytes to move ahead");
  cur->len = diff;
  cur->rem -= diff;
}

void move_base_ahead(struct tau_token *cur, const char *ahead) {
  assert(ahead > cur->buf && "move_len_ahead: ahead is behind buffer base");
  size_t taken = ahead - cur->buf;
  cur->buf = ahead;
  cur->rem -= taken;
}

bool should_omit_newlines(struct tau_token *cur) { return cur->par_balance > 0 || cur->sbr_balance > 0; }

void skip_spaces(struct tau_token *cur) {
  assert(cur != NULL && "skip_spaces: cur token cannot be NULL");
  assert(cur->buf != NULL && "skip_spaces: cur buffer cannot be NULL");

  uint32_t uc = 0;
  uint8_t uc_len;
  size_t col_count = cur->loc.col;
  size_t row_count = cur->loc.row;
  const char *ahead = cur->buf;

  uc_len = tau_dec_bytes_to_cp(ahead, &uc);
  if (is_space(uc) || (is_newline(uc) && should_omit_newlines(cur))) {
    do {
      if (is_newline(uc)) {
        col_count = 0;
        row_count += 1;
      } else {
        col_count += 1;
      }

      ahead += uc_len;
      uc_len = tau_dec_bytes_to_cp(ahead, &uc);
    } while (is_space(uc));

    move_base_ahead(cur, ahead);
    cur->loc.col = col_count;
    cur->loc.row = row_count;
  }
}

bool tokenize_as_eol(struct tau_token *cur) {
  assert(cur != NULL && "tokenize_as_eol: cur token cannot be NULL");
  assert(cur->buf != NULL && "tokenize_as_eol: cur buffer cannot be NULL");

  uint32_t uc = 0;
  uint8_t uc_len;
  size_t col_count = cur->loc.col;
  size_t row_count = cur->loc.row;
  const char *ahead = cur->buf;

  uc_len = tau_dec_bytes_to_cp(ahead, &uc);
  if (is_newline(uc) || uc == UC_SEMICOLON) {
    do {
      if (is_newline(uc)) {
        col_count = 0;
        row_count += 1;
      } else {
        col_count += 1;
      }

      ahead += uc_len;
      uc_len = tau_dec_bytes_to_cp(ahead, &uc);
    } while (is_space(uc) || is_newline(uc) || uc == UC_SEMICOLON);

    move_len_ahead(cur, ahead);
    cur->type = TAU_TOKEN_TYPE_EOL;
    cur->loc.col = col_count;
    cur->loc.row = row_count;
    return true;
  }

  return false;
}

bool tokenize_as_num_lit(struct tau_token *cur) {
  assert(cur != NULL && "tokenize_as_number: cur token cannot be NULL");
  assert(cur->buf != NULL && "tokenize_as_number: cur buffer cannot be NULL");

  uint32_t uc = 0;
  uint8_t uc_len;
  enum tau_num_base base = TAU_NUM_BASE_DEC;
  const char *ahead = cur->buf;

  uc_len = tau_dec_bytes_to_cp(ahead, &uc);
  if (is_digit(uc, base)) {
    if (cur->rem > DIGITS_BASE_PREFIX_LEN && strncmp(ahead, "0b", DIGITS_BASE_PREFIX_LEN) == 0) {
      base = TAU_NUM_BASE_BIN;
      ahead += DIGITS_BASE_PREFIX_LEN;
    } else if (cur->rem > DIGITS_BASE_PREFIX_LEN && strncmp(ahead, "0o", DIGITS_BASE_PREFIX_LEN) == 0) {
      base = TAU_NUM_BASE_OCT;
      ahead += DIGITS_BASE_PREFIX_LEN;
    } else if (cur->rem > DIGITS_BASE_PREFIX_LEN && strncmp(ahead, "0x", DIGITS_BASE_PREFIX_LEN) == 0) {
      base = TAU_NUM_BASE_HEX;
      ahead += DIGITS_BASE_PREFIX_LEN;
    }

    do {
      ahead += uc_len;
      uc_len = tau_dec_bytes_to_cp(ahead, &uc);
    } while (is_digit(uc, base));

    if (base == TAU_NUM_BASE_DEC && *ahead == '.') {
      do {
        ahead += uc_len;
        uc_len = tau_dec_bytes_to_cp(ahead, &uc);
      } while (is_digit(uc, base));

      if (strncmp(ahead, "e+", EXPONENT_WITH_SIGN_PREFIX_LEN) == 0 ||
          strncmp(ahead, "e-", EXPONENT_WITH_SIGN_PREFIX_LEN) == 0) {
        uc_len = 2;
        do {
          ahead += uc_len;
          uc_len = tau_dec_bytes_to_cp(ahead, &uc);
        } while (is_digit(uc, base));
      }

      move_len_ahead(cur, ahead);
      cur->type = TAU_TOKEN_TYPE_FLT_LIT;
      cur->num_base = base;
      return true;
    }

    move_len_ahead(cur, ahead);
    cur->type = TAU_TOKEN_TYPE_INT_LIT;
    cur->num_base = base;
    return true;
  }

  return false;
}
// "\'" | "\"" | "\\" | "\n" | "\r" | "\t" | "\b" | "\f" | "\v" | "\0"
bool skip_escape_seq(const char *ahead, uint8_t *len) {
  if (strncmp(ahead, "\\'", ESCAPE_SEQUENCE_PREFIX_LEN) == 0 ||
      strncmp(ahead, "\\n", ESCAPE_SEQUENCE_PREFIX_LEN) == 0 ||
      strncmp(ahead, "\\r", ESCAPE_SEQUENCE_PREFIX_LEN) == 0 ||
      strncmp(ahead, "\\t", ESCAPE_SEQUENCE_PREFIX_LEN) == 0 ||
      strncmp(ahead, "\\b", ESCAPE_SEQUENCE_PREFIX_LEN) == 0 ||
      strncmp(ahead, "\\f", ESCAPE_SEQUENCE_PREFIX_LEN) == 0 ||
      strncmp(ahead, "\\v", ESCAPE_SEQUENCE_PREFIX_LEN) == 0 ||
      strncmp(ahead, "\\0", ESCAPE_SEQUENCE_PREFIX_LEN) == 0 ||
      strncmp(ahead, "\\\"", ESCAPE_SEQUENCE_PREFIX_LEN) == 0 ||
      strncmp(ahead, "\\\\", ESCAPE_SEQUENCE_PREFIX_LEN) == 0) {
    *len += 2;
    return true;
  } else if (strncmp(ahead, "\\x", ESCAPE_SEQUENCE_PREFIX_LEN) == 0) {
    *len += 4;
    return true;
  } else if (strncmp(ahead, "\\u", ESCAPE_SEQUENCE_PREFIX_LEN) == 0) {
    *len += 6;
    return true;
  } else if (strncmp(ahead, "\\U", ESCAPE_SEQUENCE_PREFIX_LEN) == 0) {
    *len += 10;
    return true;
  }

  return false;
}

bool tokenize_as_str_lit(struct tau_token *cur) {
  assert(cur != NULL && "tokenize_as_number: cur token cannot be NULL");
  assert(cur->buf != NULL && "tokenize_as_number: cur buffer cannot be NULL");

  uint32_t uc = 0;
  uint8_t uc_len;
  const char *ahead = cur->buf;

  uc_len = tau_dec_bytes_to_cp(ahead, &uc);
  if (uc == UC_QUOTATION_MARK) {
    // We accept the current double quote
    ahead += uc_len;
    uc_len = tau_dec_bytes_to_cp(ahead, &uc);

    // Consume all unicode characters
    do {
      // look ahead for escape sequences
      if (!skip_escape_seq(ahead, &uc_len)) {
        // stop if a new line or other double quote is found
        if (uc == UC_QUOTATION_MARK || is_newline(uc)) {
          break;
        }
      }

      ahead += uc_len;
      uc_len = tau_dec_bytes_to_cp(ahead, &uc);
    } while (uc_len > 0 && uc_len <= 4);

    if (uc == UC_QUOTATION_MARK) {
      ahead += uc_len;
      move_len_ahead(cur, ahead);
      cur->type = TAU_TOKEN_TYPE_STR_LIT;
    } else {
      move_len_ahead(cur, ahead);
      tau_log(TAU_LOG_LEVEL_ERROR, cur->loc,
              "unclosed string literal, new line (U+000A) found before quotation mark (\" U+0022)");
      cur->type = TAU_TOKEN_TYPE_NONE;
    }

    return true;
  }

  return false;
}

bool tokenize_as_punct(struct tau_token *cur) {
  assert(cur != NULL && "tokenize_as_punct: cur token cannot be NULL");
  assert(cur->buf != NULL && "tokenize_as_punct: cur buffer cannot be NULL");
  for (enum tau_punct i = TAU_PUNCT_NONE + 1; i < TAU_PUNCT_COUNT; i++) {
    const char *comparing_punct = punct_table[i];
    size_t comparing_punct_len = strlen(comparing_punct);
    if (strncmp(cur->buf, comparing_punct, comparing_punct_len) == 0) {
      move_len_ahead(cur, cur->buf + comparing_punct_len);
      cur->type = TAU_TOKEN_TYPE_PUNCT;
      cur->punct = i;
      return true;
    }
  }
  return false;
}

bool tokenize_as_word(struct tau_token *cur) {
  assert(cur != NULL && "tokenize_as_ident: cur token cannot be NULL");
  assert(cur->buf != NULL && "tokenize_as_ident: cur buffer cannot be NULL");

  uint32_t uc = 0;
  uint8_t uc_len;
  const char *start = cur->buf;
  const char *ahead = cur->buf;

  uc_len = tau_dec_bytes_to_cp(ahead, &uc);
  if (is_ident_start(uc)) {
    do {
      ahead += uc_len;
      uc_len = tau_dec_bytes_to_cp(ahead, &uc);
    } while (is_ident_body(uc));

    cur->keyword = TAU_KEYWORD_NONE;
    for (enum tau_keyword i = TAU_KEYWORD_NONE + 1; i < TAU_KEYWORD_COUNT; i++) {
      const char *comparing_keyword = keyword_table[i];
      size_t comparing_keyword_len = strlen(comparing_keyword);
      if (strncmp(start, comparing_keyword, comparing_keyword_len) == 0) {
        cur->keyword = i;
        break;
      }
    }

    move_len_ahead(cur, ahead);
    if (cur->keyword == TAU_KEYWORD_TRUE || cur->keyword == TAU_KEYWORD_FALSE) {
      cur->type = TAU_TOKEN_TYPE_BOL_LIT;
    } else if (cur->keyword == TAU_KEYWORD_NIL) {
      cur->type = TAU_TOKEN_TYPE_NIL_LIT;
    } else if (cur->keyword == TAU_KEYWORD_UNIT) {
      cur->type = TAU_TOKEN_TYPE_UNI_LIT;
    } else if (cur->keyword != TAU_KEYWORD_NONE) {
      cur->type = TAU_TOKEN_TYPE_KEYWORD;
    } else {
      cur->type = TAU_TOKEN_TYPE_IDENTIFIER;
    }

    return true;
  }

  return false;
}

void apply_bracket_balance(struct tau_token *cur) {
  if (cur->punct == TAU_PUNCT_LPAR) {
    cur->par_balance++;
  } else if (cur->punct == TAU_PUNCT_RPAR) {
    cur->par_balance--;
  }

  if (cur->punct == TAU_PUNCT_LSBR) {
    cur->sbr_balance++;
  } else if (cur->punct == TAU_PUNCT_RSBR) {
    cur->sbr_balance--;
  }

  if (cur->punct == TAU_PUNCT_LCBR) {
    cur->cbr_balance++;
  } else if (cur->punct == TAU_PUNCT_RCBR) {
    cur->cbr_balance--;
  }
}

const char *tau_token_get_name(enum tau_token_type type) { return token_name_table[type]; }

const char *tau_token_get_punct_name(enum tau_punct punct) {
  if (punct > TAU_PUNCT_NONE && punct < TAU_PUNCT_COUNT) {
    return punct_table[punct];
  }

  return "(invalid)";
}
const char *tau_token_get_keyword_name(enum tau_keyword keyword) {
  if (keyword > TAU_KEYWORD_NONE && keyword < TAU_KEYWORD_COUNT) {
    return keyword_table[keyword];
  }

  return "(invalid)";
}

struct tau_token tau_token_start(const char *name, const char *buf_data, size_t buf_size) {
  return (struct tau_token){
      .buf = buf_data,
      .rem = buf_size,
      .len = 0,
      .par_balance = 0,
      .sbr_balance = 0,
      .cbr_balance = 0,
      .loc.buf_name = name,
      .loc.row = 0,
      .loc.col = 0,
      .type = TAU_TOKEN_TYPE_NONE,
      .punct = TAU_PUNCT_NONE,
      .keyword = TAU_KEYWORD_NONE,
      .num_base = TAU_NUM_BASE_DEC,
  };
}

struct tau_token tau_token_next(const struct tau_token prev) {
  struct tau_token cur = {
      .buf = prev.buf + prev.len,
      .rem = prev.rem,
      .len = 0,
      .par_balance = prev.par_balance,
      .sbr_balance = prev.sbr_balance,
      .cbr_balance = prev.cbr_balance,
      .loc.buf_name = prev.loc.buf_name,
      .loc.row = prev.loc.row,
      .loc.col = prev.loc.col + prev.len,
      .type = TAU_TOKEN_TYPE_EOF,
      .punct = TAU_PUNCT_NONE,
      .keyword = TAU_KEYWORD_NONE,
      .num_base = TAU_NUM_BASE_DEC,
  };
  assert(cur.buf != NULL && "tau_token_next: buf cannot be NULL");

  if (*cur.buf == '\0' || cur.rem == 0) {
    return cur;
  }

  // Skip spaces and count offsets for token begin, and row/col
  skip_spaces(&cur);

  // If new line found, tokenize as EOL
  if (tokenize_as_eol(&cur)) {
    return cur;
  }

  // Try to tokenize as numeric values
  if (tokenize_as_num_lit(&cur)) {
    return cur;
  }

  // Try to tokenize as string value
  if (tokenize_as_str_lit(&cur)) {
    return cur;
  }

  // Try to tokenize as punctuation
  if (tokenize_as_punct(&cur)) {
    apply_bracket_balance(&cur);
    return cur;
  }

  // Try to tokenize as:
  //  1 - Keyword
  //  2 - "nil" or "unit" literal
  //  3 - Identifier otherwise
  if (tokenize_as_word(&cur)) {
    return cur;
  }

  // Token it's not recognized, but we still have to skip it to avoid infinite loops
  uint32_t unknown_uc = 0;
  uint8_t unknown_uc_len = tau_dec_bytes_to_cp(cur.buf, &unknown_uc);
  char unknown_uc_enc[10] = {0};
  tau_enc_cp_to_bytes(unknown_uc, unknown_uc_enc);
  tau_log(TAU_LOG_LEVEL_ERROR, cur.loc, "unknown unicode character `%s` (U+%ld)", unknown_uc_enc, unknown_uc);
  move_len_ahead(&cur, cur.buf + unknown_uc_len);
  cur.type = TAU_TOKEN_TYPE_NONE;
  return cur;
}
