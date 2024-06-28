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

/** In debug builds (NDEBUG not defined), prints a message on the standard
 * output like `printf'. Otherwise, does nothing and returns zero. */
int debugPrintf(const char *fmt, ...);

/**
 * @}
 */

#endif
