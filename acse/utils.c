/// @file utils.c

#include <stdio.h>
#include <inttypes.h>
#include <stdarg.h>
#include "utils.h"
#include "gencode.h"
#include "cflow_graph.h"
#include "reg_alloc.h"
#include "acse.h"
#include "target_info.h"

int debugPrintf(const char *fmt, ...)
{
#ifndef NDEBUG
  int res;
  va_list args;

  va_start(args, fmt);
  res = vprintf(fmt, args);
  va_end(args);
  return res;
#else
  return 0;
#endif
}
