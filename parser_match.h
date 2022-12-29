//
// Created on 12/29/22.
//

#ifndef TAU_PARSER_MATCH_H
#define TAU_PARSER_MATCH_H

#include <assert.h>
#include <stdbool.h>

#include "lexer.h"

bool match(struct tau_token *ahead, enum tau_token_type type, enum tau_punct punct, enum tau_keyword keyword);
void consume(struct tau_token *ahead);
bool match_and_consume(struct tau_token *ahead, enum tau_token_type type, enum tau_punct punct,
                       enum tau_keyword keyword);

#define MUST_OR_FAIL(v, t, e)                                                                               \
  do {                                                                                                      \
    if (!(v)) {                                                                                             \
      tau_log(TAU_LOG_LEVEL_ERROR, (t)->loc, "unexpected `%.*s`, was expecting %s", (t)->len, (t)->buf, e); \
      goto handle_fail;                                                                                     \
    }                                                                                                       \
  } while (0)

#define MUST_OR_RETURN(v, t, e, r)                                                                          \
  do {                                                                                                      \
    if (!(v)) {                                                                                             \
      tau_log(TAU_LOG_LEVEL_ERROR, (t)->loc, "unexpected `%.*s`, was expecting %s", (t)->len, (t)->buf, e); \
      return (r);                                                                                           \
    }                                                                                                       \
  } while (0)

#endif  // TAU_PARSER_MATCH_H
