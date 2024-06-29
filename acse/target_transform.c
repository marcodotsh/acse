/// @file mace_transform.h

#include <stdarg.h>
#include <stdbool.h>
#include "target_transform.h"
#include "gencode.h"
#include "acse.h"
#include "list.h"
#include "target_info.h"
#include "target_asm_print.h"

#define RD(i)  (i->rDest->ID)
#define RS1(i) (i->rSrc1->ID)
#define RS2(i) (i->rSrc2->ID)
#define IMM(i) (i->immediate)

#define SYSCALL_ID_PRINT_INT 1
#define SYSCALL_ID_READ_INT 5
#define SYSCALL_ID_EXIT_0 10
#define SYSCALL_ID_PRINT_CHAR 11


t_listNode *addInstrAfter(
    t_program *program, t_listNode *prev, t_instruction *instr)
{
  program->instructions = listInsertAfter(program->instructions, prev, (void *)instr);
  if (prev == NULL)
    return program->instructions;
  return prev->next;
}


void setMCRegisterWhitelist(t_instrArg *regObj, ...)
{
  t_listNode *res = NULL;
  va_list args;
  t_regID cur;

  va_start(args, regObj);
  cur = va_arg(args, t_regID);
  while (cur != REG_INVALID) {
    res = listInsert(res, INT_TO_LIST_DATA(cur), -1);
    cur = va_arg(args, t_regID);
  }
  va_end(args);

  if (regObj->mcRegWhitelist)
    deleteList(regObj->mcRegWhitelist);
  regObj->mcRegWhitelist = res;
}


bool isImmediateArgumentInstrOpcode(int opcode)
{
  switch (opcode) {
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
      return true;
  }
  return false;
}


int getMatchingNonImmediateOpcode(int orig)
{
  switch (orig) {
    case OPC_ADDI:
      return OPC_ADD;
    case OPC_SUBI:
      return OPC_SUB;
    case OPC_ANDI:
      return OPC_AND;
    case OPC_ORI:
      return OPC_OR;
    case OPC_XORI:
      return OPC_XOR;
    case OPC_MULI:
      return OPC_MUL;
    case OPC_DIVI:
      return OPC_DIV;
    case OPC_SLLI:
      return OPC_SLL;
    case OPC_SRLI:
      return OPC_SRL;
    case OPC_SRAI:
      return OPC_SRA;
    case OPC_SEQI:
      return OPC_SEQ;
    case OPC_SNEI:
      return OPC_SNE;
    case OPC_SLTI:
      return OPC_SLT;
    case OPC_SLTIU:
      return OPC_SLTU;
    case OPC_SGEI:
      return OPC_SGE;
    case OPC_SGEIU:
      return OPC_SGEU;
    case OPC_SGTI:
      return OPC_SGT;
    case OPC_SGTIU:
      return OPC_SGTU;
    case OPC_SLEI:
      return OPC_SLE;
    case OPC_SLEIU:
      return OPC_SLEU;
  }
  return orig;
}


bool isInt12(int immediate)
{
  return immediate < (1 << 11) && immediate >= -(1 << 11);
}


void fixUnsupportedImmediates(t_program *program)
{
  t_listNode *curi = program->instructions;

  while (curi) {
    t_listNode *transformedInstrLnk = curi;
    t_instruction *instr = curi->data;

    if (!isImmediateArgumentInstrOpcode(instr->opcode)) {
      curi = curi->next;
      continue;
    }

    if (instr->opcode == OPC_ADDI && instr->rSrc1->ID == REG_0) {
      if (!isInt12(instr->immediate)) {
        curi = addInstrAfter(
            program, curi, genLI(NULL, RD(instr), IMM(instr)));
        removeInstructionAt(program, transformedInstrLnk);
      }

    } else if (instr->opcode == OPC_MULI || instr->opcode == OPC_DIVI ||
        !isInt12(instr->immediate)) {
      t_regID reg = getNewRegister(program);
      int newOpc = getMatchingNonImmediateOpcode(instr->opcode);
      curi =
          addInstrAfter(program, curi, genLI(NULL, reg, IMM(instr)));
      curi = addInstrAfter(program, curi,
          genInstruction(NULL, newOpc, RD(instr), RS1(instr), reg, NULL, 0));
      removeInstructionAt(program, transformedInstrLnk);

    } else if (instr->opcode == OPC_SLLI || instr->opcode == OPC_SRLI ||
        instr->opcode == OPC_SRAI) {
      instr->immediate = (unsigned)(instr->immediate) & 0x1F;
    }

    curi = curi->next;
  }
}


void fixPseudoInstructions(t_program *program)
{
  t_listNode *curi = program->instructions;

  while (curi) {
    t_listNode *transformedInstrLnk = curi;
    t_instruction *instr = curi->data;

    if (instr->opcode == OPC_SUBI) {
      instr->opcode = OPC_ADDI;
      instr->immediate = -instr->immediate;

    } else if (instr->opcode == OPC_SEQ || instr->opcode == OPC_SNE ||
        instr->opcode == OPC_SEQI || instr->opcode == OPC_SNEI) {
      if (instr->opcode == OPC_SEQ || instr->opcode == OPC_SNE)
        curi = addInstrAfter(program, curi,
            genSUB(NULL, RD(instr), RS1(instr), RS2(instr)));
      else
        curi = addInstrAfter(program, curi,
            genADDI(NULL, RD(instr), RS1(instr), -IMM(instr)));
      if (instr->opcode == OPC_SEQ || instr->opcode == OPC_SEQI)
        curi = addInstrAfter(
            program, curi, genSLTIU(NULL, RD(instr), RD(instr), 1));
      else
        curi = addInstrAfter(program, curi,
            genSLTU(NULL, RD(instr), REG_0, RD(instr)));
      removeInstructionAt(program, transformedInstrLnk);

    } else if ((instr->opcode == OPC_SGTI && IMM(instr) == INT32_MAX) ||
        (instr->opcode == OPC_SGTIU && (uint32_t)IMM(instr) == UINT32_MAX)) {
      curi = addInstrAfter(program, curi, genLI(NULL, RD(instr), 0));
      removeInstructionAt(program, transformedInstrLnk);

    } else if (instr->opcode == OPC_SGE || instr->opcode == OPC_SGEU ||
        instr->opcode == OPC_SGEI || instr->opcode == OPC_SGEIU ||
        instr->opcode == OPC_SGTI || instr->opcode == OPC_SGTIU ||
        instr->opcode == OPC_SLE || instr->opcode == OPC_SLEU) {
      if (instr->opcode == OPC_SGE) {
        instr->opcode = OPC_SLT;
      } else if (instr->opcode == OPC_SGEI) {
        instr->opcode = OPC_SLTI;
      } else if (instr->opcode == OPC_SGEU) {
        instr->opcode = OPC_SLTU;
      } else if (instr->opcode == OPC_SGEIU) {
        instr->opcode = OPC_SLTIU;
      } else if (instr->opcode == OPC_SGTI) {
        instr->opcode = OPC_SLTI;
        instr->immediate += 1;
      } else if (instr->opcode == OPC_SGTIU) {
        instr->opcode = OPC_SLTIU;
        instr->immediate += 1;
      } else {
        t_instrArg *tmp = instr->rSrc1;
        instr->rSrc1 = instr->rSrc2;
        instr->rSrc2 = tmp;
        if (instr->opcode == OPC_SLE)
          instr->opcode = OPC_SLT;
        else if (instr->opcode == OPC_SLEU)
          instr->opcode = OPC_SLTU;
      }
      curi = addInstrAfter(
          program, curi, genXORI(NULL, RD(instr), RD(instr), 1));

    } else if ((instr->opcode == OPC_SLEI && IMM(instr) == INT32_MAX) ||
        (instr->opcode == OPC_SLEIU && (uint32_t)IMM(instr) == UINT32_MAX)) {
      curi = addInstrAfter(program, curi, genLI(NULL, RD(instr), 1));
      removeInstructionAt(program, transformedInstrLnk);

    } else if (instr->opcode == OPC_SLEI) {
      instr->opcode = OPC_SLTI;
      instr->immediate += 1;

    } else if (instr->opcode == OPC_SLEIU) {
      instr->opcode = OPC_SLTIU;
      instr->immediate += 1;

    } else if (instr->opcode == OPC_SGT || instr->opcode == OPC_SGTU ||
        instr->opcode == OPC_BGT || instr->opcode == OPC_BGTU ||
        instr->opcode == OPC_BLE || instr->opcode == OPC_BLEU) {
      t_instrArg *tmp;
      if (instr->opcode == OPC_SGT)
        instr->opcode = OPC_SLT;
      else if (instr->opcode == OPC_SGTU)
        instr->opcode = OPC_SLTU;
      else if (instr->opcode == OPC_BGT)
        instr->opcode = OPC_BLT;
      else if (instr->opcode == OPC_BGTU)
        instr->opcode = OPC_BLTU;
      else if (instr->opcode == OPC_BLE)
        instr->opcode = OPC_BGE;
      else if (instr->opcode == OPC_BLEU)
        instr->opcode = OPC_BGEU;
      tmp = instr->rSrc1;
      instr->rSrc1 = instr->rSrc2;
      instr->rSrc2 = tmp;

    } else if (instr->opcode == OPC_SW_G) {
      // We always force the temporary argument of SW instructions to be T6.
      //   This solves two problems. First, as T6 is never used by register
      // allocation, we can freely use global SW instructions to generate stores
      // to spilled registers. The other problem this solves is that the
      // register allocation is not aware of the fact that the register used for
      // the first operand must be different than the register of the temporary
      // operand; by forcing T6 here we avoid the assignment of the two to the
      // same register by construction.
      setMCRegisterWhitelist(instr->rDest, REG_T6, -1);
    }

    curi = curi->next;
  }
}


void fixSyscalls(t_program *program)
{
  t_listNode *curi = program->instructions;

  while (curi) {
    t_listNode *transformedInstrLnk = curi;
    t_instruction *instr = curi->data;
    t_instruction *tmp, *ecall;
    int func;
    t_regID r_func, r_arg, r_dest;

    if (instr->opcode != OPC_CALL_EXIT_0 &&
        instr->opcode != OPC_CALL_READ_INT &&
        instr->opcode != OPC_CALL_PRINT_INT &&
        instr->opcode != OPC_CALL_PRINT_CHAR) {
      curi = curi->next;
      continue;
    }

    /* load syscall ID in a7 */
    if (instr->opcode == OPC_CALL_EXIT_0)
      func = SYSCALL_ID_EXIT_0;
    else if (instr->opcode == OPC_CALL_PRINT_INT)
      func = SYSCALL_ID_PRINT_INT;
    else if (instr->opcode == OPC_CALL_READ_INT)
      func = SYSCALL_ID_READ_INT;
    else // if (instr->opcode == OPC_CALL_PRINT_CHAR)
      func = SYSCALL_ID_PRINT_CHAR;
    r_func = getNewRegister(program);
    curi = addInstrAfter(program, curi, genLI(NULL, r_func, func));

    /* load argument in a0, if there is one */
    if (instr->rSrc1) {
      r_arg = getNewRegister(program);
      tmp = genADDI(NULL, r_arg, RS1(instr), 0);
      curi = addInstrAfter(program, curi, tmp);
    } else {
      r_arg = REG_INVALID;
    }

    /* generate an ECALL */
    if (instr->rDest)
      r_dest = getNewRegister(program);
    else
      r_dest = REG_INVALID;
    ecall = genInstruction(NULL, OPC_ECALL, r_dest, r_func, r_arg, NULL, 0);
    curi = addInstrAfter(program, curi, ecall);
    if (ecall->rDest)
      setMCRegisterWhitelist(ecall->rDest, REG_A0, -1);
    if (ecall->rSrc1)
      setMCRegisterWhitelist(ecall->rSrc1, REG_A7, -1);
    if (ecall->rSrc2)
      setMCRegisterWhitelist(ecall->rSrc2, REG_A0, -1);

    /* move a0 (result) to the destination reg. if needed */
    if (instr->rDest)
      curi = addInstrAfter(
          program, curi, genADDI(NULL, RD(instr), r_dest, 0));

    /* remove the old call instruction */
    removeInstructionAt(program, transformedInstrLnk);

    curi = curi->next;
  }
}


void doTargetSpecificTransformations(t_program *program)
{
  fixPseudoInstructions(program);
  fixSyscalls(program);
  fixUnsupportedImmediates(program);
}
