/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_expressions.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#include <limits.h>
#include "axe_expressions.h"
#include "axe_gencode.h"
#include "axe_errors.h"
#include "axe_utils.h"


t_axe_expression getImmediateExpression(int value)
{
   t_axe_expression exp;
   exp.type = IMMEDIATE;
   exp.immediate = value;
   exp.registerId = REG_INVALID;
   return exp;
}

t_axe_expression getRegisterExpression(int registerId)
{
   t_axe_expression exp;
   exp.type = REGISTER;
   exp.immediate = 0;
   exp.registerId = registerId;
   return exp;
}


int computeBinaryOperation(int val1, int val2, int binop)
{
   switch (binop) {
      case OP_ADD: return val1 + val2;
      case OP_ANDB: return val1 & val2;
      case OP_ANDL: return val1 && val2;
      case OP_ORB: return val1 | val2;
      case OP_ORL: return val1 || val2;
      case OP_XORB: return val1 ^ val2;
      case OP_SUB: return val1 - val2;
      case OP_MUL: return val1 * val2;
      /* SHL, SHR, DIV need special handling to avoid undefined behavior */
      case OP_SHL:
         if (val2 < 0 || val2 >= 32) {
            emitWarning(WARN_INVALID_SHIFT_AMOUNT);
            val2 = (unsigned)val2 & 0x1F;
         }
         return val1 << (val2 & 0x1F);
      case OP_SHR:
         if (val2 < 0 || val2 >= 32) {
            emitWarning(WARN_INVALID_SHIFT_AMOUNT);
            val2 = (unsigned)val2 & 0x1F;
         }
         /* the C language does not guarantee a right shift of a signed value
          * is an arithmetic shift, so we have to make sure it is */
         return (val1 >> val2) |
                     (val1 < 0 ? (((1 << val2) - 1) << MAX(32 - val2, 0)) : 0);
      case OP_DIV:
         if (val2 == 0) {
            emitWarning(WARN_DIVISION_BY_ZERO);
            return INT_MAX;
         } else if (val1 == INT_MIN && val2 == -1) {
            emitWarning(WARN_OVERFLOW);
            return INT_MIN;
         }
         return val1 / val2;
      case OP_LT: return val1 < val2;
      case OP_GT: return val1 > val2;
      case OP_EQ: return val1 == val2;
      case OP_NOTEQ: return val1 != val2;
      case OP_LTEQ: return val1 <= val2;
      case OP_GTEQ: return val1 >= val2;
   }

   fatalError(AXE_INVALID_EXPRESSION);
   return 0;
}

int genBinaryOperationWithImmediate(t_program_infos *program,
      int r1, int immediate, int op)
{
   int rd = getNewRegister(program);

   if (op == OP_ANDL || op == OP_ORL) {
      int norm_r1 = getNewRegister(program);
      genSLTUInstruction(program, norm_r1, REG_0, r1);
      if (op == OP_ANDL)
         genANDIInstruction(program, rd, norm_r1, !!immediate);
      else
         genORIInstruction(program, rd, norm_r1, !!immediate);
   } else{
      switch (op) {
         case OP_ADD:
            genADDIInstruction(program, rd, r1, immediate);
            break;
         case OP_ANDB:
            genANDIInstruction(program, rd, r1, immediate);
            break;
         case OP_ORB:
            genORIInstruction(program, rd, r1, immediate);
            break;
         case OP_XORB:
            genXORIInstruction(program, rd, r1, immediate);
            break;
         case OP_SUB:
            genSUBIInstruction(program, rd, r1, immediate);
            break;
         case OP_MUL:
            genMULIInstruction(program, rd, r1, immediate);
            break;
         case OP_SHL:
            genSLLIInstruction(program, rd, r1, immediate);
            break;
         case OP_SHR:
            genSRAIInstruction(program, rd, r1, immediate);
            break;
         case OP_DIV:
            genDIVIInstruction(program, rd, r1, immediate);
            break;
         case OP_LT:
            genSLTIInstruction(program, rd, r1, immediate);
            break;
         case OP_GT:
            genSGTIInstruction(program, rd, r1, immediate);
            break;
         case OP_EQ:
            genSEQIInstruction(program, rd, r1, immediate);
            break;
         case OP_NOTEQ:
            genSNEIInstruction(program, rd, r1, immediate);
            break;
         case OP_LTEQ:
            genSLEIInstruction(program, rd, r1, immediate);
            break;
         case OP_GTEQ:
            genSGEIInstruction(program, rd, r1, immediate);
            break;
         default:
            fatalError(AXE_INVALID_EXPRESSION);
      }
   }

   return rd;
}

int genBinaryOperation(t_program_infos *program, int r1, int r2, int op)
{
   int rd = getNewRegister(program);

   if (op == OP_ANDL || op == OP_ORL) {
      int norm_r1 = getNewRegister(program);
      int norm_r2 = getNewRegister(program);
      genSLTUInstruction(program, norm_r1, REG_0, r1);
      genSLTUInstruction(program, norm_r2, REG_0, r2);
      if (op == OP_ANDL)
         genANDInstruction(program, rd, norm_r1, norm_r2);
      else
         genORInstruction(program, rd, norm_r1, norm_r2);
   } else{
      switch (op) {
         case OP_ADD:
            genADDInstruction(program, rd, r1, r2);
            break;
         case OP_ANDB:
            genANDInstruction(program, rd, r1, r2);
            break;
         case OP_ORB:
            genORInstruction(program, rd, r1, r2);
            break;
         case OP_XORB:
            genXORInstruction(program, rd, r1, r2);
            break;
         case OP_SUB:
            genSUBInstruction(program, rd, r1, r2);
            break;
         case OP_MUL:
            genMULInstruction(program, rd, r1, r2);
            break;
         case OP_SHL:
            genSLLInstruction(program, rd, r1, r2);
            break;
         case OP_SHR:
            genSRAInstruction(program, rd, r1, r2);
            break;
         case OP_DIV:
            genDIVInstruction(program, rd, r1, r2);
            break;
         case OP_LT:
            genSLTInstruction(program, rd, r1, r2);
            break;
         case OP_GT:
            genSGTInstruction(program, rd, r1, r2);
            break;
         case OP_EQ:
            genSEQInstruction(program, rd, r1, r2);
            break;
         case OP_NOTEQ:
            genSNEInstruction(program, rd, r1, r2);
            break;
         case OP_LTEQ:
            genSLEInstruction(program, rd, r1, r2);
            break;
         case OP_GTEQ:
            genSGEInstruction(program, rd, r1, r2);
            break;
         default:
            fatalError(AXE_INVALID_EXPRESSION);
      }
   }

   return rd;
}

t_axe_expression handleBinaryOperator(t_program_infos *program,
      t_axe_expression exp1, t_axe_expression exp2, int operator)
{
   t_axe_expression res;

   if (exp1.type == IMMEDIATE && exp2.type == IMMEDIATE) {
      int v = computeBinaryOperation(exp1.immediate, exp2.immediate, operator);
      res = getImmediateExpression(v);

   } else if (exp1.type == REGISTER && exp2.type == IMMEDIATE) {
      int rd = genBinaryOperationWithImmediate(program,
            exp1.registerId, exp2.immediate, operator);
      res = getRegisterExpression(rd);

   } else if ((exp1.type == IMMEDIATE || exp1.type == REGISTER)
                && exp2.type == REGISTER) {
      int r1, rd;
      if (exp1.type == IMMEDIATE)
         r1 = genLoadImmediate(program, exp1.immediate);
      else
         r1 = exp1.registerId;
      rd = genBinaryOperation(program, r1, exp2.registerId, operator);
      res = getRegisterExpression(rd);

   } else {
      fatalError(AXE_INVALID_EXPRESSION);
   }

   return res;
}
