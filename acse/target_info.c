/// @file target_info.h
/// @brief Properties of the target machine

#include <assert.h>
#include "target_info.h"


bool isHaltOrRetInstruction(t_instruction *instr)
{
  return instr->opcode == OPC_CALL_EXIT_0;
}


bool isUnconditionalJump(t_instruction *instr)
{
  return instr->opcode == OPC_J;
}


bool isJumpInstruction(t_instruction *instr)
{
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
    case OPC_BLEU:
      return true;
    default:
      return false;
  }
}


bool isCallInstruction(t_instruction *instr)
{
  return instr->opcode == OPC_ECALL;
}


bool instructionUsesPSW(t_instruction *instr)
{
  return false;
}


bool instructionDefinesPSW(t_instruction *instr)
{
  return false;
}


int getSpillRegister(int i)
{
  assert(i < NUM_SPILL_REGS);
  return i + REG_S9;
}


t_listNode *getListOfGenPurposeRegisters(void)
{
  static const int regs[NUM_GP_REGS] = {REG_S0, REG_S1, REG_S2, REG_S3, REG_S4,
      REG_S5, REG_S6, REG_S7, REG_S8, REG_T0, REG_T1, REG_T2, REG_T3, REG_T4,
      REG_T5, REG_A0, REG_A1, REG_A2, REG_A3, REG_A4, REG_A5, REG_A6, REG_A7};
  int i;
  t_listNode *res = NULL;

  for (i = NUM_GP_REGS - 1; i >= 0; i--) {
    res = addElement(res, INT_TO_LIST_DATA(regs[i]), 0);
  }
  return res;
}


t_listNode *getListOfCallerSaveRegisters(void)
{
  static const int regs[] = {REG_T0, REG_T1, REG_T2, REG_T3, REG_T4, REG_T5,
      REG_A0, REG_A1, REG_A2, REG_A3, REG_A4, REG_A5, REG_A6, REG_A7,
      REG_INVALID};
  int i;
  t_listNode *res = NULL;

  for (i = 0; regs[i] != REG_INVALID; i++) {
    res = addElement(res, INT_TO_LIST_DATA(regs[i]), 0);
  }
  return res;
}
