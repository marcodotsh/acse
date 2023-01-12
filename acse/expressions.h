/// @file expressions.h
/// @brief Support functions for t_axe_expressions.

#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H

#include "program.h"

/* EXPRESSION TYPES */
#define CONSTANT 0
#define REGISTER 1

typedef struct t_expressionValue {
   int type;         /* REGISTER or CONSTANT */
   int immediate;    /* an immediate value (only when type is CONSTANT) */
   int registerId;   /* a register ID (only when type is REGISTER) */
} t_expressionValue;


/* mathematical operator constants */
#define OP_ADD     0
#define OP_ANDB    1
#define OP_ANDL    2
#define OP_ORB     3
#define OP_ORL     4
#define OP_XORB    5
#define OP_SUB     7
#define OP_MUL     8
#define OP_SHL     9
#define OP_SHR    10
#define OP_DIV    11

/* binary comparison operator constants */
#define OP_LT     12
#define OP_GT     13
#define OP_EQ     14
#define OP_NOTEQ  15
#define OP_LTEQ   16
#define OP_GTEQ   17


/* create an immediate (constant) expression */
extern t_expressionValue getConstantExprValue(int value);

/* create a register expression */
extern t_expressionValue getRegisterExprValue(int registerId);

/* This function generats instructions for binary numeric
 * operations.  It takes as input two expressions and a binary
 * operation identifier, and it returns a new expression that
 * represents the result of the specified binary operation
 * applied to `exp1' and `exp2'.
 * If the two expressions are both CONSTANT, no instructions are generated
 * and an CONSTANT expression is returned.
 *
 * Valid values for `operator' are:
 * OP_ADD 
 * OP_ANDB
 * OP_ANDL
 * OP_ORB 
 * OP_ORL 
 * OP_XORB
 * OP_SUB 
 * OP_MUL 
 * OP_SHL 
 * OP_SHR 
 * OP_DIV
 * OP_LT     (used to test if the value of `exp1' is less than
 *           the value of `exp2')
 * OP_GT     (used to test if the value of `exp1' is greater than
 *           the value of `exp2')
 * OP_EQ     (used to test if the value of `exp1' is equal to
 *           the value of `exp2')
 * OP_NOTEQ  (used to test if the value of `exp1' is not equal to
 *           the value of `exp2')
 * OP_LTEQ   (used to test if the value of `exp1' is less than
 *           or equal to the value of `exp2')
 * OP_GTEQ   (used to test if the value of `exp1' is greater than
 *           the value of `exp2')
 */
extern t_expressionValue handleBinaryOperator(t_program *program,
      t_expressionValue exp1, t_expressionValue exp2, int operator);

#endif
