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

   return instr->opcode == OPC_HALT || instr->opcode == OPC_OLD_RET;
}

/* test if the current instruction `instr' is a BT or a BF */
int isUnconditionalJump(t_axe_instruction *instr)
{
   if (isJumpInstruction(instr))
   {
      if (instr->opcode == OPC_J)
         return 1;
      if ((instr->opcode == OPC_OLD_BT) || (instr->opcode == OPC_OLD_BF))
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
      case OPC_J   :
      case OPC_BEQ :
      case OPC_BNE :
      case OPC_BLT :
      case OPC_BLTU:
      case OPC_BGE :
      case OPC_BGEU:
      case OPC_BGT :
      case OPC_BGTU:
      case OPC_BLE :
      case OPC_BLEU:
      case OPC_OLD_BT :
      case OPC_OLD_BF :
      case OPC_OLD_BHI :
      case OPC_OLD_BLS :
      case OPC_OLD_BCC :
      case OPC_OLD_BCS :
      case OPC_OLD_BNE :
      case OPC_OLD_BEQ :
      case OPC_OLD_BVC :
      case OPC_OLD_BVS :
      case OPC_OLD_BPL :
      case OPC_OLD_BMI :
      case OPC_OLD_BGE :
      case OPC_OLD_BLT :
      case OPC_OLD_BGT :
      case OPC_OLD_BLE : return 1;
      default : return 0;
   }
}

extern int instructionUsesPSW(t_axe_instruction *instr)
{
   switch (instr->opcode) {
      case OPC_OLD_SEQ: case OPC_OLD_SGE: case OPC_OLD_SGT: case OPC_OLD_SLE: case OPC_OLD_SLT: case OPC_OLD_SNE: 
      case OPC_OLD_BHI: case OPC_OLD_BLS: case OPC_OLD_BCC: case OPC_OLD_BCS: case OPC_OLD_BNE: case OPC_OLD_BEQ: case OPC_OLD_BVC:
      case OPC_OLD_BVS: case OPC_OLD_BPL: case OPC_OLD_BMI: case OPC_OLD_BGE: case OPC_OLD_BLT: case OPC_OLD_BLE:
         return 1;
         break;
   }
   return 0;
}

extern int instructionDefinesPSW(t_axe_instruction *instr)
{
   switch (instr->opcode) {
      case OPC_ADD: case OPC_SUB: case OPC_OLD_ANDL: case OPC_OLD_ORL: case OPC_OLD_EORL: case OPC_AND: case OPC_OR:
      case OPC_XOR: case OPC_MUL: case OPC_DIV: case OPC_SLL: case OPC_SRA: case OPC_OLD_ROTL: case OPC_OLD_ROTR:
      case OPC_OLD_NEG: case OPC_ADDI: case OPC_SUBI: case OPC_OLD_ANDLI: case OPC_OLD_ORLI: case OPC_OLD_EORLI:
      case OPC_ANDI: case OPC_ORI: case OPC_XORI: case OPC_MULI: case OPC_DIVI: case OPC_SLLI:
      case OPC_SRAI: case OPC_OLD_ROTLI: case OPC_OLD_ROTRI: case OPC_OLD_NOTL: case OPC_OLD_NOTB:
      case OPC_OLD_SEQ: case OPC_OLD_SGE: case OPC_OLD_SGT: case OPC_OLD_SLE: case OPC_OLD_SLT: case OPC_OLD_SNE: 
         return 1;
         break;
   }
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

