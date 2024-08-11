/// @file codegen.c

#include <stddef.h>
#include "errors.h"
#include "codegen.h"
#include "scanner.h"
#include "target_info.h"


void validateRegisterId(t_program *program, t_regID r)
{
  if (!program)
    return;
  if (r >= 0 && r < program->firstUnusedReg)
    return;
  fatalError("bug: invalid register identifier %d", r);
}


static t_instruction *genRFormatInstruction(
    t_program *program, int opcode, t_regID rd, t_regID rs1, t_regID rs2)
{
  validateRegisterId(program, rd);
  validateRegisterId(program, rs1);
  validateRegisterId(program, rs2);
  return genInstruction(program, opcode, rd, rs1, rs2, NULL, 0);
}

static t_instruction *genIFormatInstruction(
    t_program *program, int opcode, t_regID rd, t_regID rs1, int immediate)
{
  validateRegisterId(program, rd);
  validateRegisterId(program, rs1);
  return genInstruction(program, opcode, rd, rs1, REG_INVALID, NULL, immediate);
}

static t_instruction *genBFormatInstruction(
    t_program *program, int opcode, t_regID rs1, t_regID rs2, t_label *label)
{
  validateRegisterId(program, rs1);
  validateRegisterId(program, rs2);
  return genInstruction(program, opcode, REG_INVALID, rs1, rs2, label, 0);
}


t_instruction *genADD(t_program *program, t_regID rd, t_regID rs1, t_regID rs2)
{
  return genRFormatInstruction(program, OPC_ADD, rd, rs1, rs2);
}

t_instruction *genSUB(t_program *program, t_regID rd, t_regID rs1, t_regID rs2)
{
  return genRFormatInstruction(program, OPC_SUB, rd, rs1, rs2);
}

t_instruction *genAND(t_program *program, t_regID rd, t_regID rs1, t_regID rs2)
{
  return genRFormatInstruction(program, OPC_AND, rd, rs1, rs2);
}

t_instruction *genOR(t_program *program, t_regID rd, t_regID rs1, t_regID rs2)
{
  return genRFormatInstruction(program, OPC_OR, rd, rs1, rs2);
}

t_instruction *genXOR(t_program *program, t_regID rd, t_regID rs1, t_regID rs2)
{
  return genRFormatInstruction(program, OPC_XOR, rd, rs1, rs2);
}

t_instruction *genMUL(t_program *program, t_regID rd, t_regID rs1, t_regID rs2)
{
  return genRFormatInstruction(program, OPC_MUL, rd, rs1, rs2);
}

t_instruction *genDIV(t_program *program, t_regID rd, t_regID rs1, t_regID rs2)
{
  return genRFormatInstruction(program, OPC_DIV, rd, rs1, rs2);
}

t_instruction *genREM(t_program *program, t_regID rd, t_regID rs1, t_regID rs2)
{
  return genRFormatInstruction(program, OPC_REM, rd, rs1, rs2);
}

t_instruction *genSLL(t_program *program, t_regID rd, t_regID rs1, t_regID rs2)
{
  return genRFormatInstruction(program, OPC_SLL, rd, rs1, rs2);
}

t_instruction *genSRL(t_program *program, t_regID rd, t_regID rs1, t_regID rs2)
{
  return genRFormatInstruction(program, OPC_SRL, rd, rs1, rs2);
}

t_instruction *genSRA(t_program *program, t_regID rd, t_regID rs1, t_regID rs2)
{
  return genRFormatInstruction(program, OPC_SRA, rd, rs1, rs2);
}


t_instruction *genADDI(
    t_program *program, t_regID rd, t_regID rs1, int immediate)
{
  return genIFormatInstruction(program, OPC_ADDI, rd, rs1, immediate);
}

t_instruction *genSUBI(
    t_program *program, t_regID rd, t_regID rs1, int immediate)
{
  return genIFormatInstruction(program, OPC_SUBI, rd, rs1, immediate);
}

t_instruction *genANDI(
    t_program *program, t_regID rd, t_regID rs1, int immediate)
{
  return genIFormatInstruction(program, OPC_ANDI, rd, rs1, immediate);
}

t_instruction *genMULI(
    t_program *program, t_regID rd, t_regID rs1, int immediate)
{
  return genIFormatInstruction(program, OPC_MULI, rd, rs1, immediate);
}

t_instruction *genORI(
    t_program *program, t_regID rd, t_regID rs1, int immediate)
{
  return genIFormatInstruction(program, OPC_ORI, rd, rs1, immediate);
}

t_instruction *genXORI(
    t_program *program, t_regID rd, t_regID rs1, int immediate)
{
  return genIFormatInstruction(program, OPC_XORI, rd, rs1, immediate);
}

t_instruction *genDIVI(
    t_program *program, t_regID rd, t_regID rs1, int immediate)
{
  return genIFormatInstruction(program, OPC_DIVI, rd, rs1, immediate);
}

t_instruction *genREMI(
    t_program *program, t_regID rd, t_regID rs1, int immediate)
{
  return genIFormatInstruction(program, OPC_REMI, rd, rs1, immediate);
}

t_instruction *genSLLI(
    t_program *program, t_regID rd, t_regID rs1, int immediate)
{
  return genIFormatInstruction(program, OPC_SLLI, rd, rs1, immediate);
}

t_instruction *genSRLI(
    t_program *program, t_regID rd, t_regID rs1, int immediate)
{
  return genIFormatInstruction(program, OPC_SRLI, rd, rs1, immediate);
}

t_instruction *genSRAI(
    t_program *program, t_regID rd, t_regID rs1, int immediate)
{
  return genIFormatInstruction(program, OPC_SRAI, rd, rs1, immediate);
}


t_instruction *genSEQ(t_program *program, t_regID rd, t_regID rs1, t_regID rs2)
{
  return genRFormatInstruction(program, OPC_SEQ, rd, rs1, rs2);
}

t_instruction *genSNE(t_program *program, t_regID rd, t_regID rs1, t_regID rs2)
{
  return genRFormatInstruction(program, OPC_SNE, rd, rs1, rs2);
}

t_instruction *genSLT(t_program *program, t_regID rd, t_regID rs1, t_regID rs2)
{
  return genRFormatInstruction(program, OPC_SLT, rd, rs1, rs2);
}

t_instruction *genSLTU(t_program *program, t_regID rd, t_regID rs1, t_regID rs2)
{
  return genRFormatInstruction(program, OPC_SLTU, rd, rs1, rs2);
}

t_instruction *genSGE(t_program *program, t_regID rd, t_regID rs1, t_regID rs2)
{
  return genRFormatInstruction(program, OPC_SGE, rd, rs1, rs2);
}

t_instruction *genSGEU(t_program *program, t_regID rd, t_regID rs1, t_regID rs2)
{
  return genRFormatInstruction(program, OPC_SGEU, rd, rs1, rs2);
}

t_instruction *genSGT(t_program *program, t_regID rd, t_regID rs1, t_regID rs2)
{
  return genRFormatInstruction(program, OPC_SGT, rd, rs1, rs2);
}

t_instruction *genSGTU(t_program *program, t_regID rd, t_regID rs1, t_regID rs2)
{
  return genRFormatInstruction(program, OPC_SGTU, rd, rs1, rs2);
}

t_instruction *genSLE(t_program *program, t_regID rd, t_regID rs1, t_regID rs2)
{
  return genRFormatInstruction(program, OPC_SLE, rd, rs1, rs2);
}

t_instruction *genSLEU(t_program *program, t_regID rd, t_regID rs1, t_regID rs2)
{
  return genRFormatInstruction(program, OPC_SLEU, rd, rs1, rs2);
}


t_instruction *genSEQI(
    t_program *program, t_regID rd, t_regID rs1, int immediate)
{
  return genIFormatInstruction(program, OPC_SEQI, rd, rs1, immediate);
}

t_instruction *genSNEI(
    t_program *program, t_regID rd, t_regID rs1, int immediate)
{
  return genIFormatInstruction(program, OPC_SNEI, rd, rs1, immediate);
}

t_instruction *genSLTI(
    t_program *program, t_regID rd, t_regID rs1, int immediate)
{
  return genIFormatInstruction(program, OPC_SLTI, rd, rs1, immediate);
}

t_instruction *genSLTIU(
    t_program *program, t_regID rd, t_regID rs1, int immediate)
{
  return genIFormatInstruction(program, OPC_SLTIU, rd, rs1, immediate);
}

t_instruction *genSGEI(
    t_program *program, t_regID rd, t_regID rs1, int immediate)
{
  return genIFormatInstruction(program, OPC_SGEI, rd, rs1, immediate);
}

t_instruction *genSGEIU(
    t_program *program, t_regID rd, t_regID rs1, int immediate)
{
  return genIFormatInstruction(program, OPC_SGEIU, rd, rs1, immediate);
}

t_instruction *genSGTI(
    t_program *program, t_regID rd, t_regID rs1, int immediate)
{
  return genIFormatInstruction(program, OPC_SGTI, rd, rs1, immediate);
}

t_instruction *genSGTIU(
    t_program *program, t_regID rd, t_regID rs1, int immediate)
{
  return genIFormatInstruction(program, OPC_SGTIU, rd, rs1, immediate);
}

t_instruction *genSLEI(
    t_program *program, t_regID rd, t_regID rs1, int immediate)
{
  return genIFormatInstruction(program, OPC_SLEI, rd, rs1, immediate);
}

t_instruction *genSLEIU(
    t_program *program, t_regID rd, t_regID rs1, int immediate)
{
  return genIFormatInstruction(program, OPC_SLEIU, rd, rs1, immediate);
}


t_instruction *genJ(t_program *program, t_label *label)
{
  return genInstruction(
      program, OPC_J, REG_INVALID, REG_INVALID, REG_INVALID, label, 0);
}


t_instruction *genBEQ(
    t_program *program, t_regID rs1, t_regID rs2, t_label *label)
{
  return genBFormatInstruction(program, OPC_BEQ, rs1, rs2, label);
}

t_instruction *genBNE(
    t_program *program, t_regID rs1, t_regID rs2, t_label *label)
{
  return genBFormatInstruction(program, OPC_BNE, rs1, rs2, label);
}

t_instruction *genBLT(
    t_program *program, t_regID rs1, t_regID rs2, t_label *label)
{
  return genBFormatInstruction(program, OPC_BLT, rs1, rs2, label);
}

t_instruction *genBLTU(
    t_program *program, t_regID rs1, t_regID rs2, t_label *label)
{
  return genBFormatInstruction(program, OPC_BLTU, rs1, rs2, label);
}

t_instruction *genBGE(
    t_program *program, t_regID rs1, t_regID rs2, t_label *label)
{
  return genBFormatInstruction(program, OPC_BGE, rs1, rs2, label);
}

t_instruction *genBGEU(
    t_program *program, t_regID rs1, t_regID rs2, t_label *label)
{
  return genBFormatInstruction(program, OPC_BGEU, rs1, rs2, label);
}

t_instruction *genBGT(
    t_program *program, t_regID rs1, t_regID rs2, t_label *label)
{
  return genBFormatInstruction(program, OPC_BGT, rs1, rs2, label);
}

t_instruction *genBGTU(
    t_program *program, t_regID rs1, t_regID rs2, t_label *label)
{
  return genBFormatInstruction(program, OPC_BGTU, rs1, rs2, label);
}

t_instruction *genBLE(
    t_program *program, t_regID rs1, t_regID rs2, t_label *label)
{
  return genBFormatInstruction(program, OPC_BLE, rs1, rs2, label);
}

t_instruction *genBLEU(
    t_program *program, t_regID rs1, t_regID rs2, t_label *label)
{
  return genBFormatInstruction(program, OPC_BLEU, rs1, rs2, label);
}


t_instruction *genLI(t_program *program, t_regID rd, int immediate)
{
  validateRegisterId(program, rd);
  return genInstruction(
      program, OPC_LI, rd, REG_INVALID, REG_INVALID, NULL, immediate);
}

t_instruction *genLA(t_program *program, t_regID rd, t_label *label)
{
  validateRegisterId(program, rd);
  return genInstruction(
      program, OPC_LA, rd, REG_INVALID, REG_INVALID, label, 0);
}

t_instruction *genLW(t_program *program, t_regID rd, int immediate, t_regID rs1)
{
  validateRegisterId(program, rd);
  validateRegisterId(program, rs1);
  return genInstruction(program, OPC_LW, rd, rs1, REG_INVALID, NULL, immediate);
}

t_instruction *genSW(
    t_program *program, t_regID rs2, int immediate, t_regID rs1)
{
  validateRegisterId(program, rs2);
  validateRegisterId(program, rs1);
  return genInstruction(
      program, OPC_SW, REG_INVALID, rs1, rs2, NULL, immediate);
}

t_instruction *genLWGlobal(t_program *program, t_regID rd, t_label *label)
{
  validateRegisterId(program, rd);
  return genInstruction(
      program, OPC_LW_G, rd, REG_INVALID, REG_INVALID, label, 0);
}

t_instruction *genSWGlobal(
    t_program *program, t_regID rs1, t_label *label, t_regID r_temp)
{
  validateRegisterId(program, rs1);
  return genInstruction(program, OPC_SW_G, r_temp, rs1, REG_INVALID, label, 0);
}


t_instruction *genNOP(t_program *program)
{
  return genInstruction(
      program, OPC_NOP, REG_INVALID, REG_INVALID, REG_INVALID, NULL, 0);
}

t_instruction *genECALL(t_program *program)
{
  return genInstruction(
      program, OPC_ECALL, REG_INVALID, REG_INVALID, REG_INVALID, NULL, 0);
}

t_instruction *genEBREAK(t_program *program)
{
  return genInstruction(
      program, OPC_EBREAK, REG_INVALID, REG_INVALID, REG_INVALID, NULL, 0);
}


t_instruction *genExit0Syscall(t_program *program)
{
  return genInstruction(
      program, OPC_CALL_EXIT_0, REG_INVALID, REG_INVALID, REG_INVALID, NULL, 0);
}

t_instruction *genReadIntSyscall(t_program *program, t_regID rd)
{
  validateRegisterId(program, rd);
  return genInstruction(
      program, OPC_CALL_READ_INT, rd, REG_INVALID, REG_INVALID, NULL, 0);
}

t_instruction *genPrintIntSyscall(t_program *program, t_regID rs1)
{
  validateRegisterId(program, rs1);
  return genInstruction(
      program, OPC_CALL_PRINT_INT, REG_INVALID, rs1, REG_INVALID, NULL, 0);
}

t_instruction *genPrintCharSyscall(t_program *program, t_regID rs1)
{
  validateRegisterId(program, rs1);
  return genInstruction(
      program, OPC_CALL_PRINT_CHAR, REG_INVALID, rs1, REG_INVALID, NULL, 0);
}


t_regID genLoadVariable(t_program *program, t_symbol *var)
{
  // Check if the symbol is an array; in that case do not generate any more
  // code. Calling emitError will eventually stop compilation anyway.
  if (isArray(var)) {
    emitError(curFileLoc, "'%s' is an array", var->ID);
    return REG_0;
  }

  // Generate an LA instruction to load the label address into a register.
  t_regID rAddr = getNewRegister(program);
  genLA(program, rAddr, var->label);
  // Generate a LW from the address.
  t_regID rRes = getNewRegister(program);
  genLW(program, rRes, 0, rAddr);
  return rRes;
}


void genStoreRegisterToVariable(t_program *program, t_symbol *var, t_regID reg)
{
  // Check if the symbol is an array; in that case bail out without generating
  // any code (but emitting an error that will eventually stop further
  // compilation).
  if (isArray(var)) {
    emitError(curFileLoc, "'%s' is an array", var->ID);
    return;
  }

  // Generate an LA instruction to load the label address into a register.
  t_regID rAddr = getNewRegister(program);
  genLA(program, rAddr, var->label);
  // Generate a SW to the address specified by the label.
  genSW(program, reg, 0, rAddr);
}

void genStoreConstantToVariable(t_program *program, t_symbol *var, int val)
{
  // Generate a copy of the constant value into a register.
  t_regID rVal = getNewRegister(program);
  genLI(program, rVal, val);
  // Copy the register value into the variable.
  genStoreRegisterToVariable(program, var, rVal);
}


/** Generate instructions that load the address of an array element to a
 *  register.
 *  @param program The program where the array belongs.
 *  @param array   The symbol object that refers to an array.
 *  @param rIdx    Register that at runtime contains an index into the array.
 *  @returns The identifier of the register that (at runtime) will contain the
 *           address of the array element. If the symbol is not of the correct
 *           type, REG_0 is returned instead. */
t_regID genLoadArrayAddress(t_program *program, t_symbol *array, t_regID rIdx)
{
  if (!isArray(array)) {
    // If the symbol is not an array, bail out returning a dummy register ID.
    emitError(curFileLoc, "'%s' is a scalar", array->ID);
    return REG_0;
  }
  t_label *label = array->label;

  // Generate a load of the base address using LA
  t_regID rAddr = getNewRegister(program);
  genLA(program, rAddr, label);

  // Generate the code to compute the offset of the element in the array in
  // bytes. Assume the type is an integer (no other scalar types are supported).
  int sizeofElem = 4 / TARGET_PTR_GRANULARITY;
  if (sizeofElem != 1)
    genMULI(program, rIdx, rIdx, sizeofElem);

  // Generate the code which computes the final address by summing the base
  // address to the offset of the element.
  genADD(program, rAddr, rAddr, rIdx);
  return rAddr;
}


t_regID genLoadArrayElement(t_program *program, t_symbol *array, t_regID rIdx)
{
  // Generate code that loads the address of the array element in a register
  // and then loads the element itself from that memory address.
  t_regID rAddr = genLoadArrayAddress(program, array, rIdx);
  t_regID rVal = getNewRegister(program);
  genLW(program, rVal, 0, rAddr);
  return rVal;
}


void genStoreRegisterToArrayElement(
    t_program *program, t_symbol *array, t_regID rIdx, t_regID rVal)
{
  // Generate code that loads the address of the array element in a register
  // and then stores the new value in that address.
  t_regID rAddr = genLoadArrayAddress(program, array, rIdx);
  genSW(program, rVal, 0, rAddr);
}

void genStoreConstantToArrayElement(
    t_program *program, t_symbol *array, t_regID rIdx, int val)
{
  // Generate code to move the constant value into a register.
  t_regID rVal = getNewRegister(program);
  genLI(program, rVal, val);
  // Generate the array access itself.
  genStoreRegisterToArrayElement(program, array, rIdx, rVal);
}
