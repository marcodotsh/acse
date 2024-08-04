#ifndef ERRORS_H
#define ERRORS_H

#include "lexer.h"

void emitError(t_fileLocation loc, const char *fmt, ...);
void emitWarning(t_fileLocation loc, const char *fmt, ...);

__attribute__((noreturn)) void fatalError(const char *fmt, ...);

#endif
