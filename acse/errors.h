/// @file errors.h
/// @brief Error logging utilities.

#ifndef ERRORS_H
#define ERRORS_H

#include <stddef.h>

/// Structure that represents a location in a file.
typedef struct {
  char *file; ///< The name of the file
  int row;    ///< The zero-based index of a line in the file
} t_fileLocation;

/// A global constant that represents an unknown file location
static const t_fileLocation nullFileLocation = {NULL, -1};

/// The number of errors logged by emitError up to now.
extern int numErrors;

/** Prints an error message depending on the given code.
 * Does not terminate the program.
 * @param loc       The file location where the error originated, or
 *                  `nullFileLocation' if not applicable
 * @param errorcode The error code. */
void emitError(t_fileLocation loc, const char *fmt, ...);

/** Prints the specified error message and terminates the program.
 * @param errorcode The error code. */
__attribute__((noreturn)) void fatalError(const char *format, ...);

#endif
