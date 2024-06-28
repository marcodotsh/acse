/// @file utils.h
/// @brief Contains important functions to access the list of symbols and other
///        utility functions and macros.

#ifndef UTILS_H
#define UTILS_H

#include "program.h"
#include "list.h"

/**
 * @defgroup utils Utilities
 * @brief Generally useful definitions
 * @{
 */

/** Maximum between two values. */
#define MAX(x, y) ((x) > (y) ? (x) : (y))

/** Minimum between two values. */
#define MIN(x, y) ((x) > (y) ? (y) : (x))

/** Arithmetic shift to the right.
 *  The C language does not guarantee the right shift of a signed value is an
 *  arithmetic shift, so we need to use this macro. */
#define SHIFT_RIGHT_ARITH(x, y) \
    (((x) >> (y)) | ((x) < 0 ? (((1 << (y)) - 1) << MAX(32 - (y), 0)) : 0))

/** In debug builds (NDEBUG not defined), prints a message on the standard
 * output like `printf'. Otherwise, does nothing and returns zero. */
int debugPrintf(const char *fmt, ...);

/**
 * @}
 */

#endif
