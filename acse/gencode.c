/// @file gencode.c

#include <assert.h>
#include "gencode.h"
#include "acse.h"
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


t_instruction *genADD(
    t_program *program, t_regID r_dest, t_regID r_src1, t_regID r_src2)
{
  return genRFormatInstruction(program, OPC_ADD, r_dest, r_src1, r_src2);
}

t_instruction *genSUB(
    t_program *program, t_regID r_dest, t_regID r_src1, t_regID r_src2)
{
  return genRFormatInstruction(program, OPC_SUB, r_dest, r_src1, r_src2);
}

t_instruction *genAND(
    t_program *program, t_regID r_dest, t_regID r_src1, t_regID r_src2)
{
  return genRFormatInstruction(program, OPC_AND, r_dest, r_src1, r_src2);
}

t_instruction *genOR(
    t_program *program, t_regID r_dest, t_regID r_src1, t_regID r_src2)
{
  return genRFormatInstruction(program, OPC_OR, r_dest, r_src1, r_src2);
}

t_instruction *genXOR(
    t_program *program, t_regID r_dest, t_regID r_src1, t_regID r_src2)
{
  return genRFormatInstruction(program, OPC_XOR, r_dest, r_src1, r_src2);
}

t_instruction *genMUL(
    t_program *program, t_regID r_dest, t_regID r_src1, t_regID r_src2)
{
  return genRFormatInstruction(program, OPC_MUL, r_dest, r_src1, r_src2);
}

t_instruction *genDIV(
    t_program *program, t_regID r_dest, t_regID r_src1, t_regID r_src2)
{
  return genRFormatInstruction(program, OPC_DIV, r_dest, r_src1, r_src2);
}

t_instruction *genREM(
    t_program *program, t_regID r_dest, t_regID r_src1, t_regID r_src2)
{
  return genRFormatInstruction(program, OPC_REM, r_dest, r_src1, r_src2);
}

t_instruction *genSLL(
    t_program *program, t_regID r_dest, t_regID r_src1, t_regID r_src2)
{
  return genRFormatInstruction(program, OPC_SLL, r_dest, r_src1, r_src2);
}

t_instruction *genSRL(
    t_program *program, t_regID r_dest, t_regID r_src1, t_regID r_src2)
{
  return genRFormatInstruction(program, OPC_SRL, r_dest, r_src1, r_src2);
}

t_instruction *genSRA(
    t_program *program, t_regID r_dest, t_regID r_src1, t_regID r_src2)
{
  return genRFormatInstruction(program, OPC_SRA, r_dest, r_src1, r_src2);
}


t_instruction *genADDI(
    t_program *program, t_regID r_dest, t_regID r_src1, int immediate)
{
  return genIFormatInstruction(program, OPC_ADDI, r_dest, r_src1, immediate);
}

t_instruction *genSUBI(
    t_program *program, t_regID r_dest, t_regID r_src1, int immediate)
{
  return genIFormatInstruction(program, OPC_SUBI, r_dest, r_src1, immediate);
}

t_instruction *genANDI(
    t_program *program, t_regID r_dest, t_regID r_src1, int immediate)
{
  return genIFormatInstruction(program, OPC_ANDI, r_dest, r_src1, immediate);
}

t_instruction *genMULI(
    t_program *program, t_regID r_dest, t_regID r_src1, int immediate)
{
  return genIFormatInstruction(program, OPC_MULI, r_dest, r_src1, immediate);
}

t_instruction *genORI(
    t_program *program, t_regID r_dest, t_regID r_src1, int immediate)
{
  return genIFormatInstruction(program, OPC_ORI, r_dest, r_src1, immediate);
}

t_instruction *genXORI(
    t_program *program, t_regID r_dest, t_regID r_src1, int immediate)
{
  return genIFormatInstruction(program, OPC_XORI, r_dest, r_src1, immediate);
}

t_instruction *genDIVI(
    t_program *program, t_regID r_dest, t_regID r_src1, int immediate)
{
  return genIFormatInstruction(program, OPC_DIVI, r_dest, r_src1, immediate);
}

t_instruction *genREMI(
    t_program *program, t_regID r_dest, t_regID r_src1, int immediate)
{
  return genIFormatInstruction(program, OPC_REMI, r_dest, r_src1, immediate);
}

t_instruction *genSLLI(
    t_program *program, t_regID r_dest, t_regID r_src1, int immediate)
{
  return genIFormatInstruction(program, OPC_SLLI, r_dest, r_src1, immediate);
}

t_instruction *genSRLI(
    t_program *program, t_regID r_dest, t_regID r_src1, int immediate)
{
  return genIFormatInstruction(program, OPC_SRLI, r_dest, r_src1, immediate);
}

t_instruction *genSRAI(
    t_program *program, t_regID r_dest, t_regID r_src1, int immediate)
{
  return genIFormatInstruction(program, OPC_SRAI, r_dest, r_src1, immediate);
}


t_instruction *genSEQ(
    t_program *program, t_regID r_dest, t_regID r_src1, t_regID r_src2)
{
  return genRFormatInstruction(program, OPC_SEQ, r_dest, r_src1, r_src2);
}

t_instruction *genSNE(
    t_program *program, t_regID r_dest, t_regID r_src1, t_regID r_src2)
{
  return genRFormatInstruction(program, OPC_SNE, r_dest, r_src1, r_src2);
}

t_instruction *genSLT(
    t_program *program, t_regID r_dest, t_regID r_src1, t_regID r_src2)
{
  return genRFormatInstruction(program, OPC_SLT, r_dest, r_src1, r_src2);
}

t_instruction *genSLTU(
    t_program *program, t_regID r_dest, t_regID r_src1, t_regID r_src2)
{
  return genRFormatInstruction(program, OPC_SLTU, r_dest, r_src1, r_src2);
}

t_instruction *genSGE(
    t_program *program, t_regID r_dest, t_regID r_src1, t_regID r_src2)
{
  return genRFormatInstruction(program, OPC_SGE, r_dest, r_src1, r_src2);
}

t_instruction *genSGEU(
    t_program *program, t_regID r_dest, t_regID r_src1, t_regID r_src2)
{
  return genRFormatInstruction(program, OPC_SGEU, r_dest, r_src1, r_src2);
}

t_instruction *genSGT(
    t_program *program, t_regID r_dest, t_regID r_src1, t_regID r_src2)
{
  return genRFormatInstruction(program, OPC_SGT, r_dest, r_src1, r_src2);
}

t_instruction *genSGTU(
    t_program *program, t_regID r_dest, t_regID r_src1, t_regID r_src2)
{
  return genRFormatInstruction(program, OPC_SGTU, r_dest, r_src1, r_src2);
}

t_instruction *genSLE(
    t_program *program, t_regID r_dest, t_regID r_src1, t_regID r_src2)
{
  return genRFormatInstruction(program, OPC_SLE, r_dest, r_src1, r_src2);
}

t_instruction *genSLEU(
    t_program *program, t_regID r_dest, t_regID r_src1, t_regID r_src2)
{
  return genRFormatInstruction(program, OPC_SLEU, r_dest, r_src1, r_src2);
}


t_instruction *genSEQI(
    t_program *program, t_regID r_dest, t_regID r_src1, int immediate)
{
  return genIFormatInstruction(program, OPC_SEQI, r_dest, r_src1, immediate);
}

t_instruction *genSNEI(
    t_program *program, t_regID r_dest, t_regID r_src1, int immediate)
{
  return genIFormatInstruction(program, OPC_SNEI, r_dest, r_src1, immediate);
}

t_instruction *genSLTI(
    t_program *program, t_regID r_dest, t_regID r_src1, int immediate)
{
  return genIFormatInstruction(program, OPC_SLTI, r_dest, r_src1, immediate);
}

t_instruction *genSLTIU(
    t_program *program, t_regID r_dest, t_regID r_src1, int immediate)
{
  return genIFormatInstruction(program, OPC_SLTIU, r_dest, r_src1, immediate);
}

t_instruction *genSGEI(
    t_program *program, t_regID r_dest, t_regID r_src1, int immediate)
{
  return genIFormatInstruction(program, OPC_SGEI, r_dest, r_src1, immediate);
}

t_instruction *genSGEIU(
    t_program *program, t_regID r_dest, t_regID r_src1, int immediate)
{
  return genIFormatInstruction(program, OPC_SGEIU, r_dest, r_src1, immediate);
}

t_instruction *genSGTI(
    t_program *program, t_regID r_dest, t_regID r_src1, int immediate)
{
  return genIFormatInstruction(program, OPC_SGTI, r_dest, r_src1, immediate);
}

t_instruction *genSGTIU(
    t_program *program, t_regID r_dest, t_regID r_src1, int immediate)
{
  return genIFormatInstruction(program, OPC_SGTIU, r_dest, r_src1, immediate);
}

t_instruction *genSLEI(
    t_program *program, t_regID r_dest, t_regID r_src1, int immediate)
{
  return genIFormatInstruction(program, OPC_SLEI, r_dest, r_src1, immediate);
}

t_instruction *genSLEIU(
    t_program *program, t_regID r_dest, t_regID r_src1, int immediate)
{
  return genIFormatInstruction(program, OPC_SLEIU, r_dest, r_src1, immediate);
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


t_instruction *genLI(t_program *program, t_regID r_dest, int immediate)
{
  validateRegisterId(program, r_dest);
  return genInstruction(
      program, OPC_LI, r_dest, REG_INVALID, REG_INVALID, NULL, immediate);
}

t_instruction *genLA(t_program *program, t_regID r_dest, t_label *label)
{
  validateRegisterId(program, r_dest);
  return genInstruction(
      program, OPC_LA, r_dest, REG_INVALID, REG_INVALID, label, 0);
}

t_instruction *genLW(
    t_program *program, t_regID r_dest, int immediate, t_regID rs1)
{
  validateRegisterId(program, r_dest);
  validateRegisterId(program, rs1);
  return genInstruction(
      program, OPC_LW, r_dest, rs1, REG_INVALID, NULL, immediate);
}

t_instruction *genSW(
    t_program *program, t_regID rs2, int immediate, t_regID rs1)
{
  validateRegisterId(program, rs2);
  validateRegisterId(program, rs1);
  return genInstruction(
      program, OPC_SW, REG_INVALID, rs1, rs2, NULL, immediate);
}

t_instruction *genLWGlobal(t_program *program, t_regID r_dest, t_label *label)
{
  validateRegisterId(program, r_dest);
  return genInstruction(
      program, OPC_LW_G, r_dest, REG_INVALID, REG_INVALID, label, 0);
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

t_instruction *genReadIntSyscall(t_program *program, t_regID r_dest)
{
  validateRegisterId(program, r_dest);
  return genInstruction(
      program, OPC_CALL_READ_INT, r_dest, REG_INVALID, REG_INVALID, NULL, 0);
}

t_instruction *genPrintIntSyscall(t_program *program, t_regID r_src1)
{
  validateRegisterId(program, r_src1);
  return genInstruction(
      program, OPC_CALL_PRINT_INT, REG_INVALID, r_src1, REG_INVALID, NULL, 0);
}

t_instruction *genPrintCharSyscall(t_program *program, t_regID r_src1)
{
  validateRegisterId(program, r_src1);
  return genInstruction(
      program, OPC_CALL_PRINT_CHAR, REG_INVALID, r_src1, REG_INVALID, NULL, 0);
}


t_regID genLoadVariable(t_program *program, t_symbol *var)
{
  // Check if the symbol is an array; in that case do not generate any more
  // code. Calling emitError will eventually stop compilation anyway.
  if (isArray(var)) {
    emitError("'%s' is an array", var->ID);
    return REG_0;
  }

  // Generate an LA instruction to load the label address into a register
  t_regID rAddr = getNewRegister(program);
  genLA(program, rAddr, var->label); 
  // Generate a LW from the address
  t_regID rRes = getNewRegister(program);
  genLW(program, rRes, 0, rAddr);
  return rRes;
}


void genStoreRegisterToVariable(t_program *program, t_symbol *var, t_regID reg)
{
  // Check if the symbol is an array; in that case bail out without generating
  // any code (but emitting an error that will eventually stop further
  // compilation)
  if (isArray(var)) {
    emitError("'%s' is an array", var->ID);
    return;
  }
  // Reserve a new register which is a temporary required
  // by the pseudo-instruction
  t_regID r_temp = getNewRegister(program);
  // Generate a SW to the address specified by the label
  genSWGlobal(program, reg, var->label, r_temp);
}

void genStoreConstantToVariable(t_program *program, t_symbol *var, int val)
{
  // Generate a copy of the constant value into a register
  t_regID r_val = getNewRegister(program);
  genLI(program, r_val, val);
  // Copy the register value into the variable
  genStoreRegisterToVariable(program, var, r_val);
}


/** Generate instructions that load the address of an element of an array in a
 * register.
 * @param program The program where the array belongs.
 * @param array   The symbol object that refers to an array.
 * @param index   An expression that refers to a specific element of the array.
 * @returns The identifier of the register that (at runtime) will contain the
 *          address of the array element at position `index'. If the symbol
 *          is not of the correct type, REG_0 is returned instead. */
t_regID genLoadArrayAddress(t_program *program, t_symbol *array, t_regID idxReg)
{
  if (!isArray(array)) {
    // If the symbol is not an array, bail out returning a dummy register ID
    emitError("'%s' is a scalar", array->ID);
    return REG_0;
  }
  t_label *label = array->label;

  // Generate a load of the base address using LA
  t_regID mova_register = getNewRegister(program);
  genLA(program, mova_register, label);

  /* We are making the following assumption:
   * the type can only be an INTEGER_TYPE */
  int sizeofElem = 4 / TARGET_PTR_GRANULARITY;
  if (sizeofElem != 1)
    genMULI(program, idxReg, idxReg, sizeofElem);

  genADD(program, mova_register, mova_register, idxReg);

  /* return the identifier of the register that contains
   * the value of the array slot */
  return mova_register;
}


t_regID genLoadArrayElement(t_program *program, t_symbol *array, t_regID index)
{
  t_regID load_register, address;

  /* retrieve the address of the array slot */
  address = genLoadArrayAddress(program, array, index);

  /* get a new register */
  load_register = getNewRegister(program);

  /* load the value into `load_register' */
  genLW(program, load_register, 0, address);

  /* return the register ID that holds the required data */
  return load_register;
}


void genStoreRegisterToArrayElement(
    t_program *program, t_symbol *array, t_regID index, t_regID data)
{
  t_regID address = genLoadArrayAddress(program, array, index);

  /* load the value indirectly into `mova_register' */
  genSW(program, data, 0, address);
}

void genStoreConstantToArrayElement(
    t_program *program, t_symbol *array, t_regID index, int data)
{
  t_regID imm_register = getNewRegister(program);
  genLI(program, imm_register, data);
  genStoreRegisterToArrayElement(program, array, index, imm_register);
}
