//
// Created on 12/29/22.
//

#ifndef TAU_PARSER_MATCH_H
#define TAU_PARSER_MATCH_H

#include "lexer.h"
#include <assert.h>

bool match(struct tau_token *ahead, enum tau_token_type type, enum tau_punct punct, enum tau_keyword keyword) {
  assert(ahead != NULL && "match: ahead cannot be null");
  assert(type != TAU_TOKEN_TYPE_NONE && "match: type cannot be TAU_TOKEN_TYPE_NONE");
  assert(type != TAU_TOKEN_TYPE_COUNT && "match: type cannot be TAU_TOKEN_TYPE_COUNT");

  if (type == TAU_TOKEN_TYPE_PUNCT || type == TAU_TOKEN_TYPE_KEYWORD) {
    bool matches_punct = punct == TAU_PUNCT_NONE || ahead->punct == punct;
    bool matches_keyword = keyword == TAU_KEYWORD_NONE || ahead->keyword == keyword;
    return ahead->type == type && (matches_punct || matches_keyword);
  }

  return ahead->type == type;
}

void consume(struct tau_token *ahead) {
  assert(ahead != NULL && "match: ahead cannot be null");
  *ahead = tau_token_next(*ahead);
}

bool match_and_consume(struct tau_token *ahead, enum tau_token_type type, enum tau_punct punct,
                       enum tau_keyword keyword) {
  if (match(ahead, type, punct, keyword)) {
    consume(ahead);
    return true;
  }

  return false;
}

#define MUST_OR_FAIL(v, t, e)                                                                               \
  do {                                                                                                      \
    if (!(v)) {                                                                                             \
      tau_log(TAU_LOG_LEVEL_ERROR, (t)->loc, "unexpected `%*.s`, was expecting %s", (t)->len, (t)->buf, e); \
      goto handle_fail;                                                                                     \
    }                                                                                                       \
  } while (0)

#define MUST_OR_RETURN(v, t, e, r)                                                                          \
  do {                                                                                                      \
    if (!(v)) {                                                                                             \
      tau_log(TAU_LOG_LEVEL_ERROR, (t)->loc, "unexpected `%*.s`, was expecting %s", (t)->len, (t)->buf, e); \
      return (r);                                                                                           \
    }                                                                                                       \
  } while (0)


#endif  // TAU_PARSER_MATCH_H
