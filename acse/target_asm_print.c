/// @file target_asm_print.c

#include <string.h>
#include "utils.h"
#include "target_asm_print.h"
#include "target_info.h"
#include "target_transform.h"
#include "symbols.h"
#include "acse.h"

#define BUF_LENGTH 256


const char *opcodeToString(int opcode)
{
  switch (opcode) {
    /* Arithmetic */
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
    case OPC_SLL:
      return "sll";
    case OPC_SRL:
      return "srl";
    case OPC_SRA:
      return "sra";
    /* Arithmetic with immediate */
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
    case OPC_SLLI:
      return "slli";
    case OPC_SRLI:
      return "srli";
    case OPC_SRAI:
      return "srai";
    /* Comparison */
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
    /* Comparison with immediate */
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
    /* Jump, Branch */
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
    /* Load/Store */
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
    /* Other */
    case OPC_NOP:
      return "nop";
    case OPC_ECALL:
      return "ecall";
    case OPC_EBREAK:
      return "ebreak";
    /* Syscall */
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
  snprintf(buf, 24, "t%d", regID);
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
  int format, res;
  const char *opc;
  char *rd, *rs1, *rs2, *buf0 = buf;
  int32_t imm;
  char *address = NULL;

  opc = opcodeToString(instr->opcode);
  rd = registerToString(instr->rDest, machineRegIDs);
  rs1 = registerToString(instr->rSrc1, machineRegIDs);
  rs2 = registerToString(instr->rSrc2, machineRegIDs);
  if (instr->addressParam) {
    int n = labelToString(NULL, 0, instr->addressParam, 0);
    address = calloc((size_t)n + 1, sizeof(char));
    if (!address)
      fatalError("out of memory");
    labelToString(address, n + 1, instr->addressParam, 0);
  }
  imm = instr->immediate;

  format = opcodeToFormat(instr->opcode);
  switch (format) {
    case FORMAT_OP:
      if (!instr->rDest || !instr->rSrc1 || !instr->rSrc2)
        fatalError("invalid instruction found in the program");
      res = snprintf(buf, bufsz, "%-6s %s, %s, %s", opc, rd, rs1, rs2);
      break;
    case FORMAT_OPIMM:
      if (!instr->rDest || !instr->rSrc1)
        fatalError("invalid instruction found in the program");
      res = snprintf(buf, bufsz, "%-6s %s, %s, %d", opc, rd, rs1, imm);
      break;
    case FORMAT_LOAD:
      if (!instr->rDest || !instr->rSrc1)
        fatalError("invalid instruction found in the program");
      res = snprintf(buf, bufsz, "%-6s %s, %d(%s)", opc, rd, imm, rs1);
      break;
    case FORMAT_LOAD_GL:
      if (!instr->rDest || !instr->addressParam)
        fatalError("invalid instruction found in the program");
      res = snprintf(buf, bufsz, "%-6s %s, %s", opc, rd, address);
      break;
    case FORMAT_STORE:
      if (!instr->rSrc2 || !instr->rSrc1)
        fatalError("invalid instruction found in the program");
      res = snprintf(buf, bufsz, "%-6s %s, %d(%s)", opc, rs2, imm, rs1);
      break;
    case FORMAT_STORE_GL:
      if (!instr->rDest || !instr->rSrc1 || !instr->addressParam)
        fatalError("invalid instruction found in the program");
      res = snprintf(buf, bufsz, "%-6s %s, %s, %s", opc, rs1, address, rd);
      break;
    case FORMAT_BRANCH:
      if (!instr->rSrc1 || !instr->rSrc2 || !instr->addressParam)
        fatalError("invalid instruction found in the program");
      res = snprintf(buf, bufsz, "%-6s %s, %s, %s", opc, rs1, rs2, address);
      break;
    case FORMAT_JUMP:
      if (!instr->addressParam)
        fatalError("invalid instruction found in the program");
      res = snprintf(buf, bufsz, "%-6s %s", opc, address);
      break;
    case FORMAT_LI:
      if (!instr->rDest)
        fatalError("invalid instruction found in the program");
      res = snprintf(buf, bufsz, "%-6s %s, %d", opc, rd, imm);
      break;
    case FORMAT_LA:
      if (!instr->rDest || !instr->addressParam)
        fatalError("invalid instruction found in the program");
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


int translateForwardDeclarations(t_program *program, FILE *fp)
{
  /* print declarations for all global labels */
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
        return -1;
    }
  }

  return 0;
}


int printInstruction(t_instruction *instr, FILE *fp, bool machineRegIDs)
{
  char buf[BUF_LENGTH];
  int res;

  if (instr->label != NULL) {
    labelToString(buf, BUF_LENGTH, instr->label, 1);
  } else {
    buf[0] = '\0';
  }
  if (fprintf(fp, "%-8s", buf) < 0)
    return -1;

  instructionToString(buf, BUF_LENGTH, instr, machineRegIDs);
  if (instr->comment) {
    res = fprintf(fp, "%-24s# %s", buf, instr->comment);
  } else {
    res = fprintf(fp, "%s", buf);
  }
  if (res < 0)
    return -1;

  return 0;
}


int translateCodeSegment(t_program *program, FILE *fp)
{
  /* if the instruction list is empty, there is nothing to print, exit */
  if (!program->instructions)
    return 0;

  /* write the .text directive */
  if (fprintf(fp, "%-8s.text\n", "") < 0)
    return -1;

  /* iterate through the instruction list */
  t_listNode *current_element = program->instructions;
  while (current_element != NULL) {
    /* retrieve the current instruction */
    t_instruction *current_instr = (t_instruction *)current_element->data;
    if (current_instr == NULL)
      fatalError("NULL instruction found in the program");

    /* print label, instruction and comment */
    if (printInstruction(current_instr, fp, true) < 0)
      return -1;
    if (fprintf(fp, "\n") < 0)
      return -1;

    /* advance to the next instruction */
    current_element = current_element->next;
  }

  return 0;
}


int printGlobalDeclaration(t_symbol *data, FILE *fp)
{
  char buf[BUF_LENGTH];

  /* print the label */
  if (data->label != NULL) {
    labelToString(buf, BUF_LENGTH, data->label, 1);
  } else {
    buf[0] = '\0';
  }
  if (fprintf(fp, "%-8s", buf) < 0)
    return -1;

  /* print the directive */
  int size;
  switch (data->type) {
    case TYPE_INT:
      size = 4 / TARGET_PTR_GRANULARITY;
      break;
    case TYPE_INT_ARRAY:
      size = (4 / TARGET_PTR_GRANULARITY) * data->arraySize;
      break;
    default:
      fatalError("invalid data type found in the program");
  }
  if (fprintf(fp, ".space %d", size) < 0)
    return -1;

  return 0;
}


int translateDataSegment(t_program *program, FILE *fp)
{
  // If the symbol table is empty, nothing to do
  if (program->symbols == NULL)
    return 0;

  // write the .data directive to switch to the data segment
  if (fprintf(fp, "%-8s.data\n", "") < 0)
    return -1;

  // Print a static declaration for each symbol
  t_listNode *li = program->symbols;
  while (li != NULL) {
    t_symbol *symbol = (t_symbol *)li->data;

    if (printGlobalDeclaration(symbol, fp) < 0)
      return -1;
    if (fprintf(fp, "\n") < 0)
      return -1;

    li = li->next;
  }

  return 0;
}


void writeAssembly(t_program *program, FILE *fp)
{
  debugPrintf(" -> Code segment size: %d instructions\n",
      listLength(program->instructions));
  debugPrintf(" -> Data segment size: %d elements\n", listLength(program->symbols));
  debugPrintf(" -> Number of labels: %d\n", listLength(program->labels));

  if (translateForwardDeclarations(program, fp) < 0)
    fatalError("error writing to the assembly output file");

  /* print the data segment */
  if (translateDataSegment(program, fp) < 0)
    fatalError("error writing to the assembly output file");

  /* print the code segment */
  if (translateCodeSegment(program, fp) < 0)
    fatalError("error writing to the assembly output file");

  /* close the file and return */
  if (fclose(fp) == EOF)
    fatalError("error writing to the assembly output file");
}
