//
// Created on 12/13/22.
//

#include "log.h"

#include <stdarg.h>
#include <stdio.h>

void tau_log(enum tau_log_level level,  const char *file, size_t row, size_t col, const char *fmt, ...) {
  FILE *target = stdout;
  if (level == TAU_LOG_LEVEL_ERROR) {
    target = stderr;
  }

  va_list list;
  va_start(list, fmt);

  fprintf(target, "%s:%ld:%ld ", file, row, col);
  switch(level) {
  case TAU_LOG_LEVEL_TRACE:
    fprintf(target, "trace: ");
    break;
  case TAU_LOG_LEVEL_DEBUG:
    fprintf(target, "debug: ");
    break;
  case TAU_LOG_LEVEL_INFO:
    fprintf(target, "info: ");
    break;
  case TAU_LOG_LEVEL_WARN:
    fprintf(target, "warn: ");
    break;
  case TAU_LOG_LEVEL_ERROR:
    fprintf(target, "error: ");
    break;
  }

  vfprintf(target, fmt, list);
  fprintf(target, "\n");
  va_end(list);

}

