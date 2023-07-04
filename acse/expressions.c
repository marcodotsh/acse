/// @file expressions.c

#include <limits.h>
#include "expressions.h"
#include "gencode.h"
#include "errors.h"
#include "utils.h"


t_expressionValue constantExpressionValue(int value)
{
  t_expressionValue exp;
  exp.type = CONSTANT;
  exp.immediate = value;
  exp.registerId = REG_INVALID;
  return exp;
}

t_expressionValue registerExpressionValue(int registerId)
{
  t_expressionValue exp;
  exp.type = REGISTER;
  exp.immediate = 0;
  exp.registerId = registerId;
  return exp;
}

int genConvertExpValueToRegister(t_program *program, t_expressionValue exp)
{
  if (exp.type == REGISTER)
    return exp.registerId;
  int res = getNewRegister(program);
  genLIInstruction(program, res, exp.immediate);
  return res;
}

t_expressionValue genNormalizeBooleanExpValue(t_program *program, t_expressionValue exp)
{
  if (exp.type == CONSTANT)
    return constantExpressionValue(exp.immediate ? 1 : 0);
  int res = getNewRegister(program);
  genSLTUInstruction(program, res, REG_0, exp.registerId);
  return registerExpressionValue(res);
}
