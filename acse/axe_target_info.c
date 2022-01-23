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

extern int instructionUsesPSW(t_axe_instruction *instr)
{
   switch (instr->opcode) {
      case OPC_SEQ: case OPC_SGE: case OPC_SGT: case OPC_SLE: case OPC_SLT: case OPC_SNE: 
      case OPC_BHI: case OPC_BLS: case OPC_BCC: case OPC_BCS: case OPC_BNE: case OPC_BEQ: case OPC_BVC:
      case OPC_BVS: case OPC_BPL: case OPC_BMI: case OPC_BGE: case OPC_BLT: case OPC_BLE:
         return 1;
         break;
   }
   return 0;
}

extern int instructionDefinesPSW(t_axe_instruction *instr)
{
   switch (instr->opcode) {
      case OPC_ADD: case OPC_SUB: case OPC_ANDL: case OPC_ORL: case OPC_EORL: case OPC_ANDB: case OPC_ORB:
      case OPC_EORB: case OPC_MUL: case OPC_DIV: case OPC_SHL: case OPC_SHR: case OPC_ROTL: case OPC_ROTR:
      case OPC_NEG: case OPC_ADDI: case OPC_SUBI: case OPC_ANDLI: case OPC_ORLI: case OPC_EORLI:
      case OPC_ANDBI: case OPC_ORBI: case OPC_EORBI: case OPC_MULI: case OPC_DIVI: case OPC_SHLI:
      case OPC_SHRI: case OPC_ROTLI: case OPC_ROTRI: case OPC_NOTL: case OPC_NOTB:
      case OPC_SEQ: case OPC_SGE: case OPC_SGT: case OPC_SLE: case OPC_SLT: case OPC_SNE: 
         return 1;
         break;
   }
   return 0;
}
