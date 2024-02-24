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


/**
 * @defgroup expval Expression Values
 * @brief Generalized abstraction for a value that may or may not be constant
 * @{
 */

/* EXPRESSION TYPES */
#define CONSTANT 0
#define REGISTER 1

typedef struct {
  int type;       /* REGISTER or CONSTANT */
  int immediate;  /* an immediate value (only when type is CONSTANT) */
  t_regID registerId; /* a register ID (only when type is REGISTER) */
} t_expressionValue;


/* create an immediate (constant) expression */
t_expressionValue constantExpressionValue(int value);

/* create a register expression */
t_expressionValue registerExpressionValue(t_regID registerId);

t_regID genConvertExpValueToRegister(t_program *program, t_expressionValue exp);

t_expressionValue genNormalizeBooleanExpValue(t_program *program, t_expressionValue exp);

/**
 * @}
 */

#endif
