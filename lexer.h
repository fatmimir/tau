//
// Created on 12/24/22.
//

#ifndef TAU_LEXER_H
#define TAU_LEXER_H

#include <stdint.h>

#include "common.h"

enum tau_token_type {
  TAU_TOKEN_TYPE_NONE,
  TAU_TOKEN_TYPE_EOF,
  TAU_TOKEN_TYPE_EOL,
  TAU_TOKEN_TYPE_INT_LIT,
  TAU_TOKEN_TYPE_FLT_LIT,
  TAU_TOKEN_TYPE_STR_LIT,
  TAU_TOKEN_TYPE_BOL_LIT,
  TAU_TOKEN_TYPE_NIL_LIT,
  TAU_TOKEN_TYPE_UNI_LIT,
  TAU_TOKEN_TYPE_PUNCT,
  TAU_TOKEN_TYPE_KEYWORD,
  TAU_TOKEN_TYPE_IDENTIFIER,
  TAU_TOKEN_TYPE_COUNT,
};

enum tau_punct {
  TAU_PUNCT_NONE,
  TAU_PUNCT_D_GT_EQ,    // >>=
  TAU_PUNCT_D_LT_EQ,    // <<=
  TAU_PUNCT_D_EQ,       // ==
  TAU_PUNCT_D_GT,       // >>
  TAU_PUNCT_D_LT,       // <<
  TAU_PUNCT_D_AMP,      // &&
  TAU_PUNCT_D_PIPE,     // ||
  TAU_PUNCT_D_COLON,    // ::
  TAU_PUNCT_COLON_EQ,   // :=
  TAU_PUNCT_BANG_EQ,    // !=
  TAU_PUNCT_GT_EQ,      // >=
  TAU_PUNCT_LT_EQ,      // <=
  TAU_PUNCT_PLUS_EQ,    // +=
  TAU_PUNCT_HYPHEN_EQ,  // -=
  TAU_PUNCT_AST_EQ,     // *=
  TAU_PUNCT_SLASH_EQ,   // /=
  TAU_PUNCT_PCT_EQ,     // %=
  TAU_PUNCT_AMP_EQ,     // &=
  TAU_PUNCT_PIPE_EQ,    // |=
  TAU_PUNCT_CIRC_EQ,    // ^=
  TAU_PUNCT_LPAR,       // (
  TAU_PUNCT_RPAR,       // )
  TAU_PUNCT_LSBR,       // [
  TAU_PUNCT_RSBR,       // ]
  TAU_PUNCT_LCBR,       // {
  TAU_PUNCT_RCBR,       // {
  TAU_PUNCT_COLON,      // :
  TAU_PUNCT_DOT,        // .
  TAU_PUNCT_COMMA,      // ,
  TAU_PUNCT_EQ,         // =
  TAU_PUNCT_BANG,       // !
  TAU_PUNCT_LT,         // <
  TAU_PUNCT_GT,         // >
  TAU_PUNCT_PLUS,       // +
  TAU_PUNCT_HYPHEN,     // -
  TAU_PUNCT_AST,        // *
  TAU_PUNCT_SLASH,      // /
  TAU_PUNCT_PCT,        // %
  TAU_PUNCT_PIPE,       // |
  TAU_PUNCT_AMP,        // &
  TAU_PUNCT_CIRC,       // ^
  TAU_PUNCT_TILDE,      // ~
  TAU_PUNCT_APOS,       // '
  TAU_PUNCT_COUNT,
};

enum tau_keyword {
  TAU_KEYWORD_NONE,
  TAU_KEYWORD_NIL,        // nil
  TAU_KEYWORD_UNIT,       // unit
  TAU_KEYWORD_TRUE,       // true
  TAU_KEYWORD_FALSE,      // false
  TAU_KEYWORD_MODULE,     // module
  TAU_KEYWORD_EXTERN,     // extern
  TAU_KEYWORD_PROC,       // proc
  TAU_KEYWORD_LET,        // let
  TAU_KEYWORD_PROTOTYPE,  // prototype
  TAU_KEYWORD_IF,         // if
  TAU_KEYWORD_ELIF,       // elif
  TAU_KEYWORD_ELSE,       // else
  TAU_KEYWORD_LOOP,       // loop
  TAU_KEYWORD_BREAK,      // break
  TAU_KEYWORD_CONTINUE,   // continue
  TAU_KEYWORD_RETURN,     // return
  TAU_KEYWORD_COUNT,
};

enum tau_num_base {
  TAU_NUM_BASE_DEC,
  TAU_NUM_BASE_BIN,
  TAU_NUM_BASE_OCT,
  TAU_NUM_BASE_HEX,
};

struct tau_token {
  const char *buf;
  size_t rem;
  size_t len;
  int32_t par_balance;
  int32_t sbr_balance;
  int32_t cbr_balance;
  struct tau_loc loc;
  enum tau_token_type type;
  enum tau_punct punct;
  enum tau_keyword keyword;
  enum tau_num_base num_base;
};

const char *tau_token_get_name(enum tau_token_type type);
const char *tau_token_get_punct_name(enum tau_punct punct);
const char *tau_token_get_keyword_name(enum tau_keyword keyword);

struct tau_token tau_token_start(const char *name, const char *buf_data, size_t buf_size);
struct tau_token tau_token_next(struct tau_token prev);

#endif  // TAU_LEXER_H
