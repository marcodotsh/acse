/// @file target_info.c
/// @brief Properties of the target machine implementation

#include <assert.h>
#include <stddef.h>
#include "target_info.h"


bool isExitInstruction(t_instruction *instr)
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


t_regID getSpillMachineRegister(int i)
{
  assert(i < NUM_SPILL_REGS);
  return (t_regID)i + REG_S9;
}


t_listNode *getListOfGenPurposeMachineRegisters(void)
{
  static const int regs[NUM_GP_REGS] = {REG_S0, REG_S1, REG_S2, REG_S3, REG_S4,
      REG_S5, REG_S6, REG_S7, REG_S8, REG_T0, REG_T1, REG_T2, REG_T3, REG_T4,
      REG_T5, REG_A0, REG_A1, REG_A2, REG_A3, REG_A4, REG_A5, REG_A6, REG_A7};
  t_listNode *res = NULL;

  for (int i = NUM_GP_REGS - 1; i >= 0; i--) {
    res = listInsert(res, INT_TO_LIST_DATA(regs[i]), 0);
  }
  return res;
}

t_listNode *getListOfMachineRegisters(void)
{
  t_listNode *res = NULL;
  for (int i = 1; i < NUM_REGISTERS; i++) {
    res = listInsert(res, INT_TO_LIST_DATA(i), 0);
  }
  return res;
}

t_listNode *getListOfCallerSaveMachineRegisters(void)
{
  static const t_regID regs[] = {REG_T0, REG_T1, REG_T2, REG_T3, REG_T4, REG_T5,
      REG_A0, REG_A1, REG_A2, REG_A3, REG_A4, REG_A5, REG_A6, REG_A7,
      REG_INVALID};
  t_listNode *res = NULL;

  for (int i = 0; regs[i] != REG_INVALID; i++) {
    res = listInsert(res, INT_TO_LIST_DATA(regs[i]), 0);
  }
  return res;
}
