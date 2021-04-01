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


static t_axe_expression handle_bin_numeric_op_Imm
         (int val1, int val2, int binop);
         
static t_axe_expression handle_bin_comparison_Imm
         (int val1, int val2, int condition);


t_axe_expression handleBinaryOperator (t_program_infos *program
         , t_axe_expression exp1, t_axe_expression exp2, int binop)
{
   int output_register;

   /* we have to test if one (or both) of
   * the two operands is an immediate value */
   if (  (exp2.expression_type == IMMEDIATE)
         && (exp1.expression_type == IMMEDIATE) )
   {
      return handle_bin_numeric_op_Imm(exp1.value, exp2.value, binop);
   }
   
   /* at first we have to ask for a free register
   * where to store the result of the operation. */
   output_register = getNewRegister(program);

   if (exp2.expression_type == IMMEDIATE)
   {
      /* we have to produce an instruction */
      switch(binop)
      {
         case ADD : genADDIInstruction (program, output_register
                             , exp1.value, exp2.value); break;
         case ANDB : genANDBIInstruction (program, output_register
                              , exp1.value, exp2.value); break;
         case ANDL : genANDLIInstruction (program, output_register
                              , exp1.value, exp2.value); break;
         case ORB  : genORBIInstruction (program, output_register
                             , exp1.value, exp2.value); break;
         case ORL  : genORLIInstruction (program, output_register
                             , exp1.value, exp2.value); break;
         case EORB  : genEORBIInstruction (program, output_register
                             , exp1.value, exp2.value); break;
         case EORL  : genEORLIInstruction (program, output_register
                             , exp1.value, exp2.value); break;
         case SUB : genSUBIInstruction (program, output_register
                             , exp1.value, exp2.value); break;
         case MUL : genMULIInstruction (program, output_register
                             , exp1.value, exp2.value); break;
         case SHL :
               if (exp2.value < 0)
                  printWarningMessage(WARN_INVALID_SHIFT_AMOUNT);
               genSHLIInstruction(program, output_register, exp1.value, 
                     exp2.value); break;
         case SHR : 
               if (exp2.value < 0)
                  printWarningMessage(WARN_INVALID_SHIFT_AMOUNT);
               genSHRIInstruction(program, output_register, exp1.value, 
                     exp2.value); break;
         case DIV :
               if (exp2.value == 0){
                  printWarningMessage(WARN_DIVISION_BY_ZERO);
               }
               genDIVIInstruction (program, output_register
                        , exp1.value, exp2.value);
               break;
         default :
               notifyError(AXE_INVALID_EXPRESSION);
      }
   }
   else if (exp1.expression_type == IMMEDIATE)
   {
      int other_reg;

      /* we have to produce an instruction */
      switch(binop)
      {
         case ADD :  genADDIInstruction (program, output_register
                              , exp2.value, exp1.value); break;
         case ANDB :  genANDBIInstruction (program, output_register
                              , exp2.value, exp1.value); break;
         case ANDL :  genANDLIInstruction (program, output_register
                              , exp2.value, exp1.value); break;
         case ORB  :  genORBIInstruction (program, output_register
                              , exp2.value, exp1.value); break;
         case ORL  :  genORLIInstruction (program, output_register
                              , exp2.value, exp1.value); break;
         case EORB  :  genEORBIInstruction (program, output_register
                              , exp2.value, exp1.value); break;
         case EORL  :  genEORLIInstruction (program, output_register
                              , exp2.value, exp1.value); break;
         case SUB :
                  genSUBIInstruction (program, output_register
                           , exp2.value, exp1.value);

                  /* we have to produce a NEG instruction */
                  genNEGInstruction (program, output_register
                           , output_register, CG_DIRECT_ALL);
                  break;
         case MUL :  genMULIInstruction (program, output_register
                              , exp2.value, exp1.value); break;
         case DIV :
                  /* we have to load into a register the immediate value */
                  other_reg = getNewRegister(program);

                  /* In order to load the immediate inside a new
                   * register we have to insert an ADDI instr. */
                  genADDIInstruction (program, other_reg
                           , REG_0, exp1.value);

                  /* we have to produce a DIV instruction */
                  genDIVInstruction (program, output_register
                           , other_reg, exp2.value, CG_DIRECT_ALL);
                  break;
         case SHL :
                  /* we have to load into a register the immediate value */
                  other_reg = getNewRegister(program);

                  /* In order to load the immediate inside a new
                   * register we have to insert an ADDI instr. */
                  genADDIInstruction (program, other_reg
                           , REG_0, exp1.value);

                  /* we have to produce a SHL instruction */
                  genSHLInstruction (program, output_register
                           , other_reg, exp2.value, CG_DIRECT_ALL);
                  break;
         case SHR :
                  /* we have to load into a register the immediate value */
                  other_reg = getNewRegister(program);

                  /* In order to load the immediate inside a new
                   * register we have to insert an ADDI instr. */
                  genADDIInstruction (program, other_reg
                           , REG_0, exp1.value);

                  /* we have to produce a SHR instruction */
                  genSHRInstruction (program, output_register
                           , other_reg, exp2.value, CG_DIRECT_ALL);
                  break;
         default :
                  notifyError(AXE_INVALID_EXPRESSION);
      }
   }
   else
   {
      /* we have to produce an instruction */
      switch(binop)
      {
         case ADD :  genADDInstruction (program, output_register
                              , exp1.value, exp2.value, CG_DIRECT_ALL);
                     break;
         case ANDB :  genANDBInstruction (program, output_register
                              , exp1.value, exp2.value, CG_DIRECT_ALL);
                     break;
         case ANDL :  genANDLInstruction (program, output_register
                              , exp1.value, exp2.value, CG_DIRECT_ALL);
                     break;
         case ORB :  genORBInstruction (program, output_register
                              , exp1.value, exp2.value, CG_DIRECT_ALL);
                     break;
         case ORL :  genORLInstruction (program, output_register
                              , exp1.value, exp2.value, CG_DIRECT_ALL);
                     break;
         case EORB :  genEORBInstruction (program, output_register
                              , exp1.value, exp2.value, CG_DIRECT_ALL);
                     break;
         case EORL :  genEORLInstruction (program, output_register
                              , exp1.value, exp2.value, CG_DIRECT_ALL);
                     break;
         case SUB :  genSUBInstruction (program, output_register
                              , exp1.value, exp2.value, CG_DIRECT_ALL);
                     break;
         case MUL :  genMULInstruction (program, output_register
                              , exp1.value, exp2.value, CG_DIRECT_ALL);
                     break;
         case DIV :  genDIVInstruction (program, output_register
                              , exp1.value, exp2.value, CG_DIRECT_ALL);
                     break;
         case SHL :  genSHLInstruction (program, output_register
                              , exp1.value, exp2.value, CG_DIRECT_ALL);
                     break;
         case SHR :  genSHRInstruction (program, output_register
                              , exp1.value, exp2.value, CG_DIRECT_ALL);
                     break;
         default :
                     notifyError(AXE_INVALID_EXPRESSION);
      }
   }
         
   /* assign a value to result */
   return createExpression (output_register, REGISTER);
}

t_axe_expression handle_bin_numeric_op_Imm
         (int val1, int val2, int binop)
{
   switch(binop)
   {
      case ADD : return createExpression((val1 + val2), IMMEDIATE);
      case ANDB : return createExpression((val1 & val2), IMMEDIATE);
      case ANDL : return createExpression((val1 && val2), IMMEDIATE);
      case ORB  : return createExpression((val1 | val2), IMMEDIATE);
      case ORL  : return createExpression((val1 || val2), IMMEDIATE);
      case EORB  : return createExpression((val1 ^ val2), IMMEDIATE);
      case EORL  : return createExpression(((!!val1) != (!!val2)), IMMEDIATE);
      case SUB : return createExpression((val1 - val2), IMMEDIATE);
      case MUL : return createExpression((val1 * val2), IMMEDIATE);
      /* SHL, SHR, DIV need special handling to avoid undefined behavior */
      case SHL:
         if (val2 < 0) {
            printWarningMessage(WARN_INVALID_SHIFT_AMOUNT);
            return createExpression(val2, IMMEDIATE);
         } else if (val2 >= 32)
            return createExpression(0, IMMEDIATE);
         return createExpression((val1 << val2), IMMEDIATE);
      case SHR:
         if (val2 < 0) {
            printWarningMessage(WARN_INVALID_SHIFT_AMOUNT);
            return createExpression(val2, IMMEDIATE);
         } else if (val2 >= 32)
            val2 = 31;
         /* the C language does not guarantee a right shift of a signed value
          * is an arithmetic shift, so we have to make sure it is */
         return createExpression((val1 >> val2) | 
               (val1 < 0 ? (((1 << val2) - 1) << MAX(32 - val2, 0)) : 0), 
               IMMEDIATE);
      case DIV :
         if (val2 == 0){
            printWarningMessage(WARN_DIVISION_BY_ZERO);
            return createExpression(INT_MAX, IMMEDIATE);
         }
         return createExpression ((val1 / val2), IMMEDIATE);
      default :
         notifyError(AXE_INVALID_EXPRESSION);
   }
   
   return createExpression (0, INVALID_EXPRESSION);
}

t_axe_expression handle_bin_comparison_Imm
         (int val1, int val2, int condition)
{
   switch(condition)
   {
      case _LT_ : return createExpression ((val1 < val2), IMMEDIATE);
      case _GT_ : return createExpression ((val1 > val2), IMMEDIATE);
      case _EQ_  : return createExpression ((val1 == val2), IMMEDIATE);
      case _NOTEQ_ : return createExpression ((val1 != val2), IMMEDIATE);
      case _LTEQ_ : return createExpression ((val1 <= val2), IMMEDIATE);
      case _GTEQ_ : return createExpression ((val1 >= val2), IMMEDIATE);
      default :
         notifyError(AXE_INVALID_EXPRESSION);
   }

   return createExpression (0, INVALID_EXPRESSION);
}

t_axe_expression handleBinaryComparison (t_program_infos *program
         , t_axe_expression exp1, t_axe_expression exp2, int condition)
{
   int output_register;

   /* we have to test if one (or both) of
   * the two operands is an immediate value */
   if (  (exp2.expression_type == IMMEDIATE)
         && (exp1.expression_type == IMMEDIATE) )
   {
      return handle_bin_comparison_Imm
                  (exp1.value, exp2.value, condition);
   }
                     
   /* at first we have to ask for a free register
   * where to store the result of the comparison. */
   output_register = getNewRegister(program);

   if (exp2.expression_type == IMMEDIATE)
   {
      /* we have to produce a SUBI instruction */
      genSUBIInstruction (program, output_register
               , exp1.value, exp2.value);
   }
   else if (exp1.expression_type == IMMEDIATE)
   {
      genSUBIInstruction (program, output_register
               , exp2.value, exp1.value);

      /* we have to produce a NEG instruction */
      genNEGInstruction (program, output_register
               , output_register, CG_DIRECT_ALL);
   }
   else
   {
      /* we have to produce a SUB instruction */
      genSUBInstruction (program, output_register
               , exp1.value, exp2.value, CG_DIRECT_ALL);
   }

   /* generate a set instruction */
   switch(condition)
   {
      case _LT_ : genSLTInstruction (program, output_register); break;
      case _GT_ : genSGTInstruction (program, output_register); break;
      case _EQ_  : genSEQInstruction (program, output_register); break;
      case _NOTEQ_ : genSNEInstruction (program, output_register); break;
      case _LTEQ_ : genSLEInstruction (program, output_register); break;
      case _GTEQ_ : genSGEInstruction (program, output_register); break;
      default :
         notifyError(AXE_INVALID_EXPRESSION);
   }

   /* return the new expression */
   return createExpression (output_register, REGISTER);
}
