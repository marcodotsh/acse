#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "errors.h"

static void printMessage(
    t_fileLocation loc, const char *category, const char *fmt, va_list arg)
{
  if (loc.file && loc.row >= 0 && loc.column >= 0)
    fprintf(stderr, "%s:%d:%d: %s: ", loc.file, loc.row + 1, loc.column + 1,
        category);
  else
    fprintf(stderr, "%s: ", category);
  vfprintf(stderr, fmt, arg);
  fputc('\n', stderr);
}

void emitError(t_fileLocation loc, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  printMessage(loc, "error", fmt, args);
  va_end(args);
}

void emitWarning(t_fileLocation loc, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  printMessage(loc, "warning", fmt, args);
  va_end(args);
}

__attribute__((noreturn)) void fatalError(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  printMessage(nullFileLocation, "fatal error", fmt, args);
  va_end(args);
  exit(1);
}
