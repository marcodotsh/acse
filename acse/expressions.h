/// @file expressions.h
/// @brief Support functions for t_axe_expressions.

#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

#include "program.h"

/* EXPRESSION TYPES */
#define CONSTANT 0
#define REGISTER 1

typedef struct t_expressionValue {
  int type;       /* REGISTER or CONSTANT */
  int immediate;  /* an immediate value (only when type is CONSTANT) */
  int registerId; /* a register ID (only when type is REGISTER) */
} t_expressionValue;


/* create an immediate (constant) expression */
t_expressionValue constantExpressionValue(int value);

/* create a register expression */
t_expressionValue registerExpressionValue(int registerId);

int genConvertExpValueToRegister(t_program *program, t_expressionValue exp);

t_expressionValue genNormalizeBooleanExpValue(t_program *program, t_expressionValue exp);


#endif
