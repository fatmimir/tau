//
// Created on 12/13/22.
//

#include "log.h"

#include <stdarg.h>
#include <stdio.h>

void tau_log(enum tau_log_level level, struct tau_loc loc, const char *fmt, ...) {
  FILE *target = stdout;
  if (level == TAU_LOG_ERROR) {
    target = stderr;
  }

  va_list list;
  va_start(list, fmt);

  fprintf(target, "%s:%ld:%ld ", loc.buf_name, loc.row, loc.col);
  switch (level) {
    case TAU_LOG_TRACE:
      fprintf(target, "trace: ");
      break;
    case TAU_LOG_DEBUG:
      fprintf(target, "debug: ");
      break;
    case TAU_LOG_INFO:
      fprintf(target, "info: ");
      break;
    case TAU_LOG_WARN:
      fprintf(target, "warn: ");
      break;
    case TAU_LOG_ERROR:
      fprintf(target, "error: ");
      break;
  }

  vfprintf(target, fmt, list);
  fprintf(target, "\n");
  va_end(list);
}
