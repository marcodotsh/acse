/*
 * Daniele Cattaneo
 * Politecnico di Milano, 2020
 * 
 * mace_transform.h
 * Formal Languages & Compilers Machine, 2007-2020
 */

#include <assert.h>
#include "target_transform.h"
#include "gencode.h"
#include "utils.h"
#include "collections.h"
#include "target_info.h"
#include "target_asm_print.h"

#define RD(i)  (i->reg_dest->ID)
#define RS1(i) (i->reg_src1->ID)
#define RS2(i) (i->reg_src2->ID)
#define IMM(i) (i->immediate)

#define SYSCALL_ID_EXIT   93
#define SYSCALL_ID_PUTINT 2002
#define SYSCALL_ID_GETINT 2003


void moveLabel(t_axe_instruction *dest, t_axe_instruction *src)
{
   assert(dest->label == NULL && "moveLabel failed: destination already is labeled");
   dest->label = src->label;
   src->label = NULL;
   
   if (!dest->user_comment) {
      dest->user_comment = src->user_comment;
      src->user_comment = NULL;
   }
}

int isImmediateArgumentInstrOpcode(int opcode)
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
      case OPC_SLEIU: return 1;
   }
   return 0;
}

int switchOpcodeImmediateForm(int orig)
{
   switch (orig) {
      case OPC_ADDI: return OPC_ADD;
      case OPC_SUBI: return OPC_SUB;
      case OPC_ANDI: return OPC_AND;
      case OPC_ORI: return OPC_OR;
      case OPC_XORI: return OPC_XOR;
      case OPC_MULI: return OPC_MUL;
      case OPC_DIVI: return OPC_DIV;
      case OPC_SLLI: return OPC_SLL;
      case OPC_SRLI: return OPC_SRL;
      case OPC_SRAI: return OPC_SRA;
      case OPC_SEQI: return OPC_SEQ;
      case OPC_SNEI: return OPC_SNE;
      case OPC_SLTI: return OPC_SLT;
      case OPC_SLTIU: return OPC_SLTU;
      case OPC_SGEI: return OPC_SGE;
      case OPC_SGEIU: return OPC_SGEU;
      case OPC_SGTI: return OPC_SGT;
      case OPC_SGTIU: return OPC_SGTU;
      case OPC_SLEI: return OPC_SLE;
      case OPC_SLEIU: return OPC_SLEU;
   }
   return orig;
}

int isInt12(int immediate)
{
   return immediate < (1 << 11) && immediate >= -(1 << 11);
}

void fixUnsupportedImmediates(t_program_infos *program)
{
   t_list *curi = program->instructions;

   while (curi) {
      t_axe_instruction *instr = LDATA(curi);
      t_list *nexti = LNEXT(curi);

      if (!isImmediateArgumentInstrOpcode(instr->opcode)) {
         curi = LNEXT(curi);
         continue;
      }

      if (instr->opcode == OPC_ADDI && instr->reg_src1->ID == REG_0) {
         if (!isInt12(instr->immediate)) {
            pushInstrInsertionPoint(program, curi);
            genLIInstruction(program, RD(instr), IMM(instr));
            removeInstructionLink(program, curi);
            popInstrInsertionPoint(program);
         }

      } else if (instr->opcode == OPC_MULI || instr->opcode == OPC_DIVI ||
            !isInt12(instr->immediate)) {
         int reg = getNewRegister(program);
         pushInstrInsertionPoint(program, curi->prev);
         moveLabel(genLIInstruction(program, reg, IMM(instr)), instr);
         instr->immediate = 0;
         instr->reg_src2 = initializeRegister(reg, 0);
         instr->opcode = switchOpcodeImmediateForm(instr->opcode);
         popInstrInsertionPoint(program);

      } else if (instr->opcode == OPC_SLLI || instr->opcode == OPC_SRLI || instr->opcode == OPC_SRAI) {
         instr->immediate = (unsigned)(instr->immediate) & 0x1F;
      }

      curi = nexti;
   }
}

void fixPseudoInstructions(t_program_infos *program)
{
   t_list *curi = program->instructions;
   
   while (curi) {
      t_axe_instruction *instr = LDATA(curi);
      t_list *nexti = LNEXT(curi);
      
      if (instr->opcode == OPC_SUBI) {
         instr->opcode = OPC_ADDI;
         instr->immediate = -instr->immediate;
         
      } else if (instr->opcode == OPC_SEQ || instr->opcode == OPC_SNE ||
            instr->opcode == OPC_SEQI || instr->opcode == OPC_SNEI) {
         pushInstrInsertionPoint(program, curi);
         if (instr->opcode == OPC_SEQ || instr->opcode == OPC_SNE)
            moveLabel(genSUBInstruction(program, RD(instr), RS1(instr), RS2(instr)), instr);
         else
            moveLabel(genADDIInstruction(program, RD(instr), RS1(instr), -IMM(instr)), instr);
         if (instr->opcode == OPC_SEQ || instr->opcode == OPC_SEQI)
            genSLTIUInstruction(program, RD(instr), RD(instr), 1);
         else
            genSLTUInstruction(program, RD(instr), REG_0, RD(instr));
         removeInstructionLink(program, curi);
         popInstrInsertionPoint(program);

      } else if ((instr->opcode == OPC_SGTI && IMM(instr) == INT32_MAX) ||
            instr->opcode == OPC_SGTIU && (uint32_t)IMM(instr) == UINT32_MAX) {
         pushInstrInsertionPoint(program, curi);
         genLIInstruction(program, RD(instr), 0);
         removeInstructionLink(program, curi);
         popInstrInsertionPoint(program);

      } else if (instr->opcode == OPC_SGE || instr->opcode == OPC_SGEU ||
            instr->opcode == OPC_SGEI || instr->opcode == OPC_SGEIU ||
            instr->opcode == OPC_SGTI || instr->opcode == OPC_SGTIU ||
            instr->opcode == OPC_SLE || instr->opcode == OPC_SLEU) {
         pushInstrInsertionPoint(program, curi);
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
            t_axe_register *tmp = instr->reg_src1;
            instr->reg_src1 = instr->reg_src2;
            instr->reg_src2 = tmp;
            if (instr->opcode == OPC_SLE)
               instr->opcode = OPC_SLT;
            else if (instr->opcode == OPC_SLEU)
               instr->opcode = OPC_SLTU;
         }
         genXORIInstruction(program, RD(instr), RD(instr), 1);
         popInstrInsertionPoint(program);

      } else if ((instr->opcode == OPC_SLEI && IMM(instr) == INT32_MAX)
            || (instr->opcode == OPC_SLEIU && (uint32_t)IMM(instr) == UINT32_MAX)) {
         pushInstrInsertionPoint(program, curi);
         genLIInstruction(program, RD(instr), 1);
         removeInstructionLink(program, curi);
         popInstrInsertionPoint(program);

      } else if (instr->opcode == OPC_SLEI) {
         instr->opcode = OPC_SLTI;
         instr->immediate += 1;

      } else if (instr->opcode == OPC_SLEIU) {
         instr->opcode = OPC_SLTIU;
         instr->immediate += 1;

      } else if (instr->opcode == OPC_SGT || instr->opcode == OPC_SGTU ||
            instr->opcode == OPC_BGT || instr->opcode == OPC_BGTU ||
            instr->opcode == OPC_BLE || instr->opcode == OPC_BLEU) {
         t_axe_register *tmp;
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
         tmp = instr->reg_src1;
         instr->reg_src1 = instr->reg_src2;
         instr->reg_src2 = tmp;
      }
      
      curi = nexti;
   }
}

void fixSyscalls(t_program_infos *program)
{
   t_list *curi = program->instructions;

   while (curi) {
      t_axe_instruction *instr = LDATA(curi);
      t_list *nexti = LNEXT(curi);
      t_axe_instruction *ecall;

      if (instr->opcode != OPC_HALT && 
            instr->opcode != OPC_AXE_READ && 
            instr->opcode != OPC_AXE_WRITE) {
         curi = nexti;
         continue;
      }

      pushInstrInsertionPoint(program, curi);

      if (instr->opcode == OPC_HALT) {
         int r_func = genLoadImmediate(program, SYSCALL_ID_EXIT);
         int r_arg = genLoadImmediate(program, 0);
         ecall = genInstruction(program, OPC_ECALL,
               NULL,
               initializeRegister(r_func, 0),
               initializeRegister(r_arg, 0),
               NULL, 0);
         
      } else if (instr->opcode == OPC_AXE_WRITE) {
         int r_func = genLoadImmediate(program, SYSCALL_ID_PUTINT);
         int r_arg = getNewRegister(program);
         genADDIInstruction(program, r_arg, RS1(instr), 0);
         ecall = genInstruction(program, OPC_ECALL,
               NULL,
               initializeRegister(r_func, 0),
               initializeRegister(r_arg, 0),
               NULL, 0);

      } else if (instr->opcode == OPC_AXE_READ) {
         int r_func = genLoadImmediate(program, SYSCALL_ID_GETINT);
         int r_res = getNewRegister(program);
         ecall = genInstruction(program, OPC_ECALL,
               initializeRegister(r_res, 0),
               initializeRegister(r_func, 0),
               NULL,
               NULL, 0);
         genADDIInstruction(program, RD(instr), r_res, 0);
      }

      if (ecall->reg_dest)
         setMCRegisterWhitelist(ecall->reg_dest, REG_A0, -1);
      if (ecall->reg_src1)
         setMCRegisterWhitelist(ecall->reg_src1, REG_A0, -1);
      if (ecall->reg_src2)
         setMCRegisterWhitelist(ecall->reg_src2, REG_A1, -1);

      removeInstructionLink(program, curi);
      popInstrInsertionPoint(program);

      curi = nexti;
   }
}

void doTargetSpecificTransformations(t_program_infos *program)
{
   fixPseudoInstructions(program);
   fixSyscalls(program);
   fixUnsupportedImmediates(program);
}
