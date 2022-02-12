/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * expressions.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 * Support functions for t_axe_expressions.
 */

#ifndef _AXE_EXPRESSIONS_H
#define _AXE_EXPRESSIONS_H

#include "engine.h"

/* EXPRESSION TYPES */
#define IMMEDIATE 0
#define REGISTER 1

typedef struct t_axe_expression {
   int type;         /* REGISTER or IMMEDIATE */
   int immediate;    /* an immediate value (only when type is IMMEDIATE) */
   int registerId;   /* a register ID (only when type is REGISTER) */
} t_axe_expression;


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
extern t_axe_expression getImmediateExpression(int value);

/* create a register expression */
extern t_axe_expression getRegisterExpression(int registerId);

/* This function generats instructions for binary numeric
 * operations.  It takes as input two expressions and a binary
 * operation identifier, and it returns a new expression that
 * represents the result of the specified binary operation
 * applied to `exp1' and `exp2'.
 * If the two expressions are both IMMEDIATE, no instructions are generated
 * and an IMMEDIATE expression is returned.
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
extern t_axe_expression handleBinaryOperator(t_program_infos *program,
      t_axe_expression exp1, t_axe_expression exp2, int operator);

#endif
