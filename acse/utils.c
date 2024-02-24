/// @file utils.c

#include <stdio.h>
#include <inttypes.h>
#include <stdarg.h>
#include "utils.h"
#include "gencode.h"
#include "cflow_graph.h"
#include "reg_alloc.h"
#include "acse.h"
#include "target_info.h"


t_expValue constantExpValue(int value)
{
  t_expValue exp;
  exp.type = CONSTANT;
  exp.constant = value;
  exp.registerId = REG_INVALID;
  return exp;
}

t_expValue registerExpValue(t_regID registerId)
{
  t_expValue exp;
  exp.type = REGISTER;
  exp.constant = 0;
  exp.registerId = registerId;
  return exp;
}

t_regID genExpValueToRegister(t_program *program, t_expValue exp)
{
  if (exp.type == REGISTER)
    return exp.registerId;
  t_regID res = getNewRegister(program);
  genLI(program, res, exp.constant);
  return res;
}

t_expValue genNormalizeBoolExpValue(t_program *program, t_expValue exp)
{
  if (exp.type == CONSTANT)
    return constantExpValue(exp.constant ? 1 : 0);
  t_regID res = getNewRegister(program);
  genSLTU(program, res, REG_0, exp.registerId);
  return registerExpValue(res);
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
