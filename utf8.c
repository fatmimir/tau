//
// Created on 12/13/22.
//

#include "utf8.h"

uint8_t tau_enc_cp_to_bytes(uint32_t codepoint, char *dest) {
  if (codepoint < 0x80) {
    dest[0] = (char)codepoint;
    return 1;
  }

  if (codepoint < 0x800) {
    dest[0] = (char)((codepoint >> 6) | 0xC0);
    dest[1] = (char)((codepoint & 0x3F) | 0x80);
    return 2;
  }

  if (codepoint < 0x10000) {
    dest[0] = (char)((codepoint >> 12) | 0xE0);
    dest[1] = (char)(((codepoint >> 6) & 0x3F) | 0x80);
    dest[2] = (char)((codepoint & 0x3F) | 0x80);
    return 3;
  }

  if (codepoint < 0x110000) {
    dest[0] = (char)((codepoint >> 18) | 0xF0);
    dest[1] = (char)(((codepoint >> 12) & 0x3F) | 0x80);
    dest[2] = (char)(((codepoint >> 6) & 0x3F) | 0x80);
    dest[3] = (char)((codepoint & 0x3F) | 0x80);
    return 4;
  }

  return 0;
}

uint8_t tau_dec_bytes_to_cp(const char src[4], uint32_t *codepoint) {
  uint8_t lead = (uint8_t)src[0];
  uint32_t err = UNICODE_REPLACEMENT_CHARACTER;

  if (lead == 0x00) {
    *codepoint = err;
    return 1;
  } else if ((~lead & 0x80) == 0x80 && src[0]) {
    *codepoint = (uint32_t)src[0];
    if (*codepoint >= 0x000007F) {
      *codepoint = err;
    }

    return 1;
  } else if ((~lead & 0x20) == 0x20 && src[0] && src[1]) {
    *codepoint = (uint32_t)(src[0] & 0x1F) << 6;
    *codepoint |= (uint32_t)(src[1] & 0x3F);
    if (*codepoint <= 0x00000080 || *codepoint >= 0x000007FF) {
      *codepoint = err;
    }

    return 2;
  } else if ((~lead & 0x10) == 0x10 && src[0] && src[1] && src[2]) {
    *codepoint = (uint32_t)(src[0] & 0x0F) << 12;
    *codepoint |= (uint32_t)(src[1] & 0x3F) << 6;
    *codepoint |= (uint32_t)(src[2] & 0x3F);
    if (*codepoint <= 0x00000800 || *codepoint >= 0x0000FFFF) {
      *codepoint = err;
    }

    return 3;
  } else if ((~lead & 0x08) == 0x08 && src[0] && src[1] && src[2] && src[3]) {
    *codepoint = (uint32_t)(src[0] & 0x0F) << 18;
    *codepoint |= (uint32_t)(src[1] & 0x3F) << 12;
    *codepoint |= (uint32_t)(src[2] & 0x3F) << 6;
    *codepoint |= (uint32_t)(src[3] & 0x3F);
    if (*codepoint <= 0x00010000 || *codepoint >= 0x0010FFFF) {
      *codepoint = err;
    }

    return 4;
  } else if ((~lead & 0x04) == 0x04) {
    *codepoint = err;
    return 5;
  } else if ((~lead & 0x02) == 0x02) {
    *codepoint = err;
    return 6;
  }

  *codepoint = err;
  return 1;
}