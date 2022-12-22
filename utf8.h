//
// Created on 12/13/22.
//

#ifndef TAU_UTF8_H
#define TAU_UTF8_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define UNICODE_REPLACEMENT_CHARACTER 0xFFFD

uint8_t tau_enc_cp_to_bytes(uint32_t codepoint, char *dest);
uint8_t tau_dec_bytes_to_cp(const char src[4], uint32_t *codepoint);

#endif  // TAU_UTF8_H
