#ifndef ERRORS_H
#define ERRORS_H

typedef struct {
  char *file;
  int row;
  int column;
} t_fileLocation;

static const t_fileLocation nullFileLocation = {NULL, -1, -1};

void emitError(t_fileLocation loc, const char *fmt, ...);
void emitWarning(t_fileLocation loc, const char *fmt, ...);

__attribute__((noreturn)) void fatalError(const char *fmt, ...);

#endif
