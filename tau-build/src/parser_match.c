//
// Created  on 1/5/23.
//

#include "parser_match.h"

#include <assert.h>
#include <stdbool.h>

bool match(struct tau_token *ahead, enum tau_token_type type, enum tau_punct punct, enum tau_keyword keyword) {
  assert(ahead != NULL && "match: ahead cannot be null");
  assert(type != TAU_TOKEN_TYPE_NONE && "match: type cannot be TAU_TOKEN_TYPE_NONE");
  assert(type != TAU_TOKEN_TYPE_COUNT && "match: type cannot be TAU_TOKEN_TYPE_COUNT");

  if (type == TAU_TOKEN_TYPE_PUNCT) {
    return ahead->type == type && (punct == TAU_PUNCT_NONE || ahead->punct == punct);
  }

  if (type == TAU_TOKEN_TYPE_KEYWORD) {
    return ahead->type == type && (keyword == TAU_KEYWORD_NONE || ahead->keyword == keyword);
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