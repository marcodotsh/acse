/*
 * Daniele Cattaneo
 * Politecnico di Milano, 2022
 * 
 * axe_target_info.h
 * Formal Languages & Compilers Machine, 2007-2022
 * 
 * Properties of the target machine
 */

#include <assert.h>
#include "axe_target_info.h"

int isHaltOrRetInstruction(t_axe_instruction *instr)
{
   if (instr == NULL) {
      return 0;
   }

   return instr->opcode == OPC_HALT;
}

/* test if the current instruction `instr' is a BT or a BF */
int isUnconditionalJump(t_axe_instruction *instr)
{
   if (isJumpInstruction(instr))
   {
      if (instr->opcode == OPC_J)
         return 1;
   }

   return 0;
}

/* test if the current instruction `instr' is a branch instruction */
int isJumpInstruction(t_axe_instruction *instr)
{
   if (instr == NULL)
      return 0;

   switch (instr->opcode) {
      case OPC_J:
      case OPC_BEQ:
      case OPC_BNE:
      case OPC_BLT:
      case OPC_BLTU:
      case OPC_BGE:
      case OPC_BGEU:
      case OPC_BGT:
      case OPC_BGTU:
      case OPC_BLE:
      case OPC_BLEU: return 1;
      default: return 0;
   }
}

extern int instructionUsesPSW(t_axe_instruction *instr)
{
   return 0;
}

extern int instructionDefinesPSW(t_axe_instruction *instr)
{
   return 0;
}

extern int isSpecialRegister(int regId)
{
   switch (regId) {
      case REG_0:
      case REG_RA:
      case REG_SP:
      case REG_GP:
      case REG_TP:
      case REG_T3:
      case REG_T4:
      case REG_T5:
      case REG_T6:
         return 1;
   }
   return 0;
}

extern int getSpillRegister(int i)
{
   assert(i < NUM_SPILL_REGS);
   return i + REG_T3;
}

