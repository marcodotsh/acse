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

typedef enum {
  CONSTANT = 0,
  REGISTER = 1
} t_expType;

typedef struct {
  t_expType type;       /* REGISTER or CONSTANT */
  int constant;  /* an immediate value (only when type is CONSTANT) */
  t_regID registerId; /* a register ID (only when type is REGISTER) */
} t_expValue;


/* create an immediate (constant) expression */
t_expValue constantExpValue(int value);

/* create a register expression */
t_expValue registerExpValue(t_regID registerId);

t_regID genExpValueToRegister(t_program *program, t_expValue exp);

t_expValue genNormalizeBoolExpValue(t_program *program, t_expValue exp);

/**
 * @}
 */

#endif
