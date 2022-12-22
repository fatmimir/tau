//
// Created on 12/13/22.
//

#ifndef TAU_LOG_H
#define TAU_LOG_H

#include <stddef.h>

#include "common.h"

enum tau_log_level {
  TAU_LOG_LEVEL_TRACE,
  TAU_LOG_LEVEL_DEBUG,
  TAU_LOG_LEVEL_INFO,
  TAU_LOG_LEVEL_WARN,
  TAU_LOG_LEVEL_ERROR,
};

void tau_log(enum tau_log_level level, struct tau_loc loc, const char *fmt, ...);

#endif  // TAU_LOG_H
