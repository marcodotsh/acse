/*
 * Daniele Cattaneo
 * Politecnico di Milano, 2022
 * 
 * axe_target_info.h
 * Formal Languages & Compilers Machine, 2007-2022
 * 
 * Properties of the target machine
 */

#include "axe_target_info.h"


int isHaltOrRetInstruction(t_axe_instruction *instr)
{
   if (instr == NULL) {
      return 0;
   }

   return instr->opcode == OPC_HALT || instr->opcode == OPC_RET;
}

/* test if the current instruction `instr' is a BT or a BF */
int isUnconditionalJump(t_axe_instruction *instr)
{
   if (isJumpInstruction(instr))
   {
      if ((instr->opcode == OPC_BT) || (instr->opcode == OPC_BF))
         return 1;
   }

   return 0;
}

/* test if the current instruction `instr' is a branch instruction */
int isJumpInstruction(t_axe_instruction *instr)
{
   if (instr == NULL)
      return 0;

   switch(instr->opcode)
   {
      case OPC_BT :
      case OPC_BF :
      case OPC_BHI :
      case OPC_BLS :
      case OPC_BCC :
      case OPC_BCS :
      case OPC_BNE :
      case OPC_BEQ :
      case OPC_BVC :
      case OPC_BVS :
      case OPC_BPL :
      case OPC_BMI :
      case OPC_BGE :
      case OPC_BLT :
      case OPC_BGT :
      case OPC_BLE : return 1;
      default : return 0;
   }
}
