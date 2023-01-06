//
// Created on 1/5/23.
//

#ifndef TAU_PARSER_H
#define TAU_PARSER_H

#include <stddef.h>

struct tau_node;

void tau_node_free(struct tau_node *node);
struct tau_node *tau_parse_buffer(const char *buf_name, const char *buf_data, size_t buf_len);

#endif  // TAU_PARSER_H
