/// @file utils.c

#include <assert.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdarg.h>
#include "utils.h"
#include "gencode.h"
#include "cflow_graph.h"
#include "reg_alloc.h"
#include "options.h"
#include "errors.h"
#include "target_info.h"
#include "symbols.h"


t_expressionValue constantExpressionValue(int value)
{
  t_expressionValue exp;
  exp.type = CONSTANT;
  exp.immediate = value;
  exp.registerId = REG_INVALID;
  return exp;
}

t_expressionValue registerExpressionValue(t_regID registerId)
{
  t_expressionValue exp;
  exp.type = REGISTER;
  exp.immediate = 0;
  exp.registerId = registerId;
  return exp;
}

t_regID genConvertExpValueToRegister(t_program *program, t_expressionValue exp)
{
  if (exp.type == REGISTER)
    return exp.registerId;
  t_regID res = getNewRegister(program);
  genLIInstruction(program, res, exp.immediate);
  return res;
}

t_expressionValue genNormalizeBooleanExpValue(t_program *program, t_expressionValue exp)
{
  if (exp.type == CONSTANT)
    return constantExpressionValue(exp.immediate ? 1 : 0);
  t_regID res = getNewRegister(program);
  genSLTUInstruction(program, res, REG_0, exp.registerId);
  return registerExpressionValue(res);
}


int debugPrintf(const char *fmt, ...)
{
#ifndef NDEBUG
  int res;
  va_list args;

  va_start(args, fmt);
  res = vprintf(fmt, args);
  va_end(args);
  return res;
#else
  return 0;
#endif
}
