#ifndef ERRORS_H
#define ERRORS_H

typedef struct {
  char *file;
  int row;
} t_fileLocation;

static const t_fileLocation nullFileLocation = {NULL, -1};

extern int num_error;

/** Prints an error message depending on the given code.
 * Does not terminate the program.
 * @param errorcode The error code. */
void emitError(t_fileLocation loc, const char *fmt, ...);

/** Prints the specified error message and terminates the program.
 * @param errorcode The error code. */
__attribute__((noreturn)) void fatalError(const char *format, ...);

#endif
