#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "errors.h"

int num_error;

static void printMessage(
    t_fileLocation loc, const char *category, const char *fmt, va_list arg)
{
  if (loc.file && loc.row >= 0)
    fprintf(stderr, "%s:%d: %s: ", loc.file, loc.row + 1, category);
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
  num_error++;
}

void fatalError(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  printMessage(nullFileLocation, "fatal error", format, args);
  va_end(args);
  exit(1);
}
