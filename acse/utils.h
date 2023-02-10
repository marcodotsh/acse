/// @file utils.h
/// @brief Contains important functions to access the list of symbols and other
///        utility functions and macros.

#ifndef UTILS_H
#define UTILS_H

#include "program.h"
#include "list.h"

/** Maximum between two values. */
#define MAX(x, y) ((x) > (y) ? (x) : (y))

/** Minimum between two values. */
#define MIN(x, y) ((x) > (y) ? (y) : (x))

/** Utility structure used to store information about a while statement. */
typedef struct t_whileStatement {
  t_label *label_condition; /* this label points to the expression
                             * that is used as loop condition */
  t_label *label_end;       /* this label points to the instruction
                             * that follows the while construct */
} t_whileStatement;


/** In debug builds (NDEBUG not defined), prints a message on the standard
 * output like `printf'. Otherwise, does nothing and returns zero. */
int debugPrintf(const char *fmt, ...);

#endif
