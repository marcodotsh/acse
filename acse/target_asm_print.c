/// @file target_asm_print.c

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "list.h"
#include "errors.h"
#include "target_asm_print.h"
#include "target_info.h"

#define BUF_LENGTH 256


const char *opcodeToString(int opcode)
{
  switch (opcode) {
    // Arithmetic
    case OPC_ADD:
      return "add";
    case OPC_SUB:
      return "sub";
    case OPC_AND:
      return "and";
    case OPC_OR:
      return "or";
    case OPC_XOR:
      return "xor";
    case OPC_MUL:
      return "mul";
    case OPC_DIV:
      return "div";
    case OPC_REM:
      return "rem";
    case OPC_SLL:
      return "sll";
    case OPC_SRL:
      return "srl";
    case OPC_SRA:
      return "sra";
    // Arithmetic with immediate
    case OPC_ADDI:
      return "addi";
    case OPC_SUBI:
      return "subi";
    case OPC_ANDI:
      return "andi";
    case OPC_ORI:
      return "ori";
    case OPC_XORI:
      return "xori";
    case OPC_MULI:
      return "muli";
    case OPC_DIVI:
      return "divi";
    case OPC_REMI:
      return "remi";
    case OPC_SLLI:
      return "slli";
    case OPC_SRLI:
      return "srli";
    case OPC_SRAI:
      return "srai";
    // Comparison
    case OPC_SEQ:
      return "seq";
    case OPC_SNE:
      return "sne";
    case OPC_SLT:
      return "slt";
    case OPC_SLTU:
      return "sltu";
    case OPC_SGE:
      return "sge";
    case OPC_SGEU:
      return "sgeu";
    case OPC_SGT:
      return "sgt";
    case OPC_SGTU:
      return "sgtu";
    case OPC_SLE:
      return "sle";
    case OPC_SLEU:
      return "sleu";
    // Comparison with immediate
    case OPC_SEQI:
      return "seqi";
    case OPC_SNEI:
      return "snei";
    case OPC_SLTI:
      return "slti";
    case OPC_SLTIU:
      return "sltiu";
    case OPC_SGEI:
      return "sgei";
    case OPC_SGEIU:
      return "sgeiu";
    case OPC_SGTI:
      return "sgti";
    case OPC_SGTIU:
      return "sgtiu";
    case OPC_SLEI:
      return "slei";
    case OPC_SLEIU:
      return "sleiu";
    // Jump, Branch
    case OPC_J:
      return "j";
    case OPC_BEQ:
      return "beq";
    case OPC_BNE:
      return "bne";
    case OPC_BLT:
      return "blt";
    case OPC_BLTU:
      return "bltu";
    case OPC_BGE:
      return "bge";
    case OPC_BGEU:
      return "bgeu";
    case OPC_BGT:
      return "bgt";
    case OPC_BGTU:
      return "bgtu";
    case OPC_BLE:
      return "ble";
    case OPC_BLEU:
      return "bleu";
    // Load/Store
    case OPC_LW:
      return "lw";
    case OPC_LW_G:
      return "lw";
    case OPC_SW:
      return "sw";
    case OPC_SW_G:
      return "sw";
    case OPC_LI:
      return "li";
    case OPC_LA:
      return "la";
    // Other
    case OPC_NOP:
      return "nop";
    case OPC_ECALL:
      return "ecall";
    case OPC_EBREAK:
      return "ebreak";
    // Syscall
    case OPC_CALL_EXIT_0:
      return "Exit";
    case OPC_CALL_READ_INT:
      return "ReadInt";
    case OPC_CALL_PRINT_INT:
      return "PrintInt";
    case OPC_CALL_PRINT_CHAR:
      return "PrintChar";
  }
  return "<unknown>";
}


#define FORMAT_AUTO -1
#define FORMAT_OP 0       // mnemonic rd, rs1, rs2
#define FORMAT_OPIMM 1    // mnemonic rd, rs1, imm
#define FORMAT_LOAD 2     // mnemonic rd, imm(rs1)
#define FORMAT_LOAD_GL 3  // mnemonic rd, label
#define FORMAT_STORE 4    // mnemonic rs2, imm(rs1)
#define FORMAT_STORE_GL 5 // mnemonic rs2, label, rd
#define FORMAT_BRANCH 6   // mnemonic rs1, rs2, label
#define FORMAT_JUMP 7     // mnemonic label
#define FORMAT_LI 8       // mnemonic rd, imm
#define FORMAT_LA 9       // mnemonic rd, label
#define FORMAT_SYSTEM 10  // mnemonic
#define FORMAT_FUNC 11    // rd = fname(rs1, rs2)

static int opcodeToFormat(int opcode)
{
  switch (opcode) {
    case OPC_ADD:
    case OPC_SUB:
    case OPC_AND:
    case OPC_OR:
    case OPC_XOR:
    case OPC_MUL:
    case OPC_DIV:
    case OPC_REM:
    case OPC_SLL:
    case OPC_SRL:
    case OPC_SRA:
    case OPC_SEQ:
    case OPC_SNE:
    case OPC_SLT:
    case OPC_SLTU:
    case OPC_SGE:
    case OPC_SGEU:
    case OPC_SGT:
    case OPC_SGTU:
    case OPC_SLE:
    case OPC_SLEU:
      return FORMAT_OP;
    case OPC_ADDI:
    case OPC_SUBI:
    case OPC_ANDI:
    case OPC_ORI:
    case OPC_XORI:
    case OPC_MULI:
    case OPC_DIVI:
    case OPC_REMI:
    case OPC_SLLI:
    case OPC_SRLI:
    case OPC_SRAI:
    case OPC_SEQI:
    case OPC_SNEI:
    case OPC_SLTI:
    case OPC_SLTIU:
    case OPC_SGEI:
    case OPC_SGEIU:
    case OPC_SGTI:
    case OPC_SGTIU:
    case OPC_SLEI:
    case OPC_SLEIU:
      return FORMAT_OPIMM;
    case OPC_J:
      return FORMAT_JUMP;
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
      return FORMAT_BRANCH;
    case OPC_LW:
      return FORMAT_LOAD;
    case OPC_LW_G:
      return FORMAT_LOAD_GL;
    case OPC_SW:
      return FORMAT_STORE;
    case OPC_SW_G:
      return FORMAT_STORE_GL;
    case OPC_LI:
      return FORMAT_LI;
    case OPC_LA:
      return FORMAT_LA;
    case OPC_NOP:
    case OPC_ECALL:
    case OPC_EBREAK:
      return FORMAT_SYSTEM;
    case OPC_CALL_EXIT_0:
    case OPC_CALL_READ_INT:
    case OPC_CALL_PRINT_INT:
    case OPC_CALL_PRINT_CHAR:
      return FORMAT_FUNC;
  }
  return -1;
}


char *registerIDToString(t_regID regID, bool machineRegIDs)
{
  char *buf;
  static const char *mcRegIds[] = {"zero", "ra", "sp", "gp", "tp", "t0", "t1",
      "t2", "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "s2",
      "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5",
      "t6"};

  if (machineRegIDs) {
    if (regID < 0 || regID >= 32)
      return NULL;
    return strdup(mcRegIds[regID]);
  }

  if (regID < 0)
    return strdup("invalid_reg");
  buf = calloc(24, sizeof(char));
  snprintf(buf, 24, "temp%d", regID);
  return buf;
}


char *registerToString(t_instrArg *reg, bool machineRegIDs)
{
  if (!reg)
    return NULL;
  return registerIDToString(reg->ID, machineRegIDs);
}


int labelToString(char *buf, int bufsz, t_label *label, int finalColon)
{
  char *labelName;
  int res;

  if (!label)
    return -1;

  labelName = getLabelName(label);

  if (finalColon)
    res = snprintf(buf, bufsz, "%s:", labelName);
  else
    res = snprintf(buf, bufsz, "%s", labelName);

  free(labelName);
  return res;
}


int instructionToString(
    char *buf, int bufsz, t_instruction *instr, bool machineRegIDs)
{
  int res;
  char *buf0 = buf;
  char *address = NULL;

  const char *opc = opcodeToString(instr->opcode);
  char *rd = registerToString(instr->rDest, machineRegIDs);
  char *rs1 = registerToString(instr->rSrc1, machineRegIDs);
  char *rs2 = registerToString(instr->rSrc2, machineRegIDs);
  if (instr->addressParam) {
    int n = labelToString(NULL, 0, instr->addressParam, 0);
    address = calloc((size_t)n + 1, sizeof(char));
    if (!address)
      fatalError("out of memory");
    labelToString(address, n + 1, instr->addressParam, 0);
  }
  int32_t imm = instr->immediate;

  int format = opcodeToFormat(instr->opcode);
  switch (format) {
    case FORMAT_OP:
      if (!instr->rDest || !instr->rSrc1 || !instr->rSrc2)
        fatalError("bug: invalid instruction found in the program");
      res = snprintf(buf, bufsz, "%-6s %s, %s, %s", opc, rd, rs1, rs2);
      break;
    case FORMAT_OPIMM:
      if (!instr->rDest || !instr->rSrc1)
        fatalError("bug: invalid instruction found in the program");
      res = snprintf(buf, bufsz, "%-6s %s, %s, %d", opc, rd, rs1, imm);
      break;
    case FORMAT_LOAD:
      if (!instr->rDest || !instr->rSrc1)
        fatalError("bug: invalid instruction found in the program");
      res = snprintf(buf, bufsz, "%-6s %s, %d(%s)", opc, rd, imm, rs1);
      break;
    case FORMAT_LOAD_GL:
      if (!instr->rDest || !instr->addressParam)
        fatalError("bug: invalid instruction found in the program");
      res = snprintf(buf, bufsz, "%-6s %s, %s", opc, rd, address);
      break;
    case FORMAT_STORE:
      if (!instr->rSrc2 || !instr->rSrc1)
        fatalError("bug: invalid instruction found in the program");
      res = snprintf(buf, bufsz, "%-6s %s, %d(%s)", opc, rs2, imm, rs1);
      break;
    case FORMAT_STORE_GL:
      if (!instr->rDest || !instr->rSrc1 || !instr->addressParam)
        fatalError("bug: invalid instruction found in the program");
      res = snprintf(buf, bufsz, "%-6s %s, %s, %s", opc, rs1, address, rd);
      break;
    case FORMAT_BRANCH:
      if (!instr->rSrc1 || !instr->rSrc2 || !instr->addressParam)
        fatalError("bug: invalid instruction found in the program");
      res = snprintf(buf, bufsz, "%-6s %s, %s, %s", opc, rs1, rs2, address);
      break;
    case FORMAT_JUMP:
      if (!instr->addressParam)
        fatalError("bug: invalid instruction found in the program");
      res = snprintf(buf, bufsz, "%-6s %s", opc, address);
      break;
    case FORMAT_LI:
      if (!instr->rDest)
        fatalError("bug: invalid instruction found in the program");
      res = snprintf(buf, bufsz, "%-6s %s, %d", opc, rd, imm);
      break;
    case FORMAT_LA:
      if (!instr->rDest || !instr->addressParam)
        fatalError("bug: invalid instruction found in the program");
      res = snprintf(buf, bufsz, "%-6s %s, %s", opc, rd, address);
      break;
    case FORMAT_SYSTEM:
      res = snprintf(buf, bufsz, "%s", opc);
      break;
    case FORMAT_FUNC:
    default:
      if (instr->rDest)
        buf += sprintf(buf, "%s = ", rd);
      buf += sprintf(buf, "%s(", opc);
      if (instr->rSrc1)
        buf += sprintf(buf, "%s", rs1);
      if (instr->rSrc1 && instr->rSrc2)
        buf += sprintf(buf, ", ");
      if (instr->rSrc2)
        buf += sprintf(buf, "%s", rs2);
      buf += sprintf(buf, ")");
      res = (int)(buf - buf0);
      break;
  }

  free(address);
  free(rd);
  free(rs1);
  free(rs2);
  return res;
}


bool translateForwardDeclarations(t_program *program, FILE *fp)
{
  for (t_listNode *li = program->labels; li != NULL; li = li->next) {
    t_label *nextLabel = li->data;

    if (nextLabel->isAlias)
      continue;

    if (nextLabel->global) {
      char *labelName;
      int res;

      labelName = getLabelName(nextLabel);
      res = fprintf(fp, "%-8s.global %s\n", "", labelName);
      free(labelName);

      if (res < 0)
        return false;
    }
  }

  return true;
}


bool printInstruction(t_instruction *instr, FILE *fp, bool machineRegIDs)
{
  char buf[BUF_LENGTH];

  if (instr->label != NULL) {
    labelToString(buf, BUF_LENGTH, instr->label, 1);
  } else {
    buf[0] = '\0';
  }
  if (fprintf(fp, "%-8s", buf) < 0)
    return false;

  instructionToString(buf, BUF_LENGTH, instr, machineRegIDs);
  int res;
  if (instr->comment) {
    res = fprintf(fp, "%-48s# %s", buf, instr->comment);
  } else {
    res = fprintf(fp, "%s", buf);
  }
  if (res < 0)
    return false;

  return true;
}

bool translateCodeSegment(t_program *program, FILE *fp)
{
  if (!program->instructions)
    return true;

  // Write the .text directive to switch to the text segment.
  if (fprintf(fp, "%-8s.text\n", "") < 0)
    return false;

  t_listNode *curNode = program->instructions;
  while (curNode != NULL) {
    t_instruction *curInstr = (t_instruction *)curNode->data;
    if (curInstr == NULL)
      fatalError("bug: NULL instruction found in the program");

    if (!printInstruction(curInstr, fp, true))
      return false;
    if (fprintf(fp, "\n") < 0)
      return false;

    curNode = curNode->next;
  }
  return true;
}


int printGlobalDeclaration(t_symbol *data, FILE *fp)
{
  char buf[BUF_LENGTH];

  // Print the label.
  if (data->label != NULL) {
    labelToString(buf, BUF_LENGTH, data->label, 1);
  } else {
    buf[0] = '\0';
  }
  if (fprintf(fp, "%-8s", buf) < 0)
    return -1;

  // Print the directive.
  int size;
  switch (data->type) {
    case TYPE_INT:
      size = 4 / TARGET_PTR_GRANULARITY;
      break;
    case TYPE_INT_ARRAY:
      size = (4 / TARGET_PTR_GRANULARITY) * data->arraySize;
      break;
    default:
      fatalError("bug: invalid data type found in the program");
  }
  if (fprintf(fp, ".space %d", size) < 0)
    return -1;

  return 0;
}

bool translateDataSegment(t_program *program, FILE *fp)
{
  // If the symbol table is empty, nothing to do.
  if (program->symbols == NULL)
    return true;

  // Write the .data directive to switch to the data segment.
  if (fprintf(fp, "%-8s.data\n", "") < 0)
    return false;

  // Print a static declaration for each symbol.
  t_listNode *li = program->symbols;
  while (li != NULL) {
    t_symbol *symbol = (t_symbol *)li->data;

    if (printGlobalDeclaration(symbol, fp) < 0)
      return false;
    if (fprintf(fp, "\n") < 0)
      return false;

    li = li->next;
  }

  return true;
}


bool writeAssembly(t_program *program, const char *fn)
{
  bool res = false;
  FILE *fp = fopen(fn, "w");
  if (fp == NULL)
    return res;

  if (!translateForwardDeclarations(program, fp))
    goto fail;
  if (!translateDataSegment(program, fp))
    goto fail;
  if (!translateCodeSegment(program, fp))
    goto fail;

  res = true;
fail:
  if (fclose(fp) == EOF)
    res = false;
  return res;
}
