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
#include "list.h"
#include "target_info.h"
#include "target_asm_print.h"

#define RD(i)  (i->reg_dest->ID)
#define RS1(i) (i->reg_src1->ID)
#define RS2(i) (i->reg_src2->ID)
#define IMM(i) (i->immediate)

#define SYSCALL_ID_EXIT   93
#define SYSCALL_ID_PUTINT 2002
#define SYSCALL_ID_GETINT 2003


t_list *addInstrAfter(t_program_infos *program, t_list *prev, t_axe_instruction *instr)
{
   program->instructions = addAfter(program->instructions, prev, (void *)instr);
   if (prev == NULL)
      return program->instructions;
   return prev->next;
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
      t_list *transformedInstrLnk = curi;
      t_axe_instruction *instr = curi->data;

      if (!isImmediateArgumentInstrOpcode(instr->opcode)) {
         curi = curi->next;
         continue;
      }

      if (instr->opcode == OPC_ADDI && instr->reg_src1->ID == REG_0) {
         if (!isInt12(instr->immediate)) {
            curi = addInstrAfter(program, curi, genLIInstruction(NULL, RD(instr), IMM(instr)));
            removeInstructionLink(program, transformedInstrLnk);
         }

      } else if (instr->opcode == OPC_MULI || instr->opcode == OPC_DIVI ||
            !isInt12(instr->immediate)) {
         int reg = getNewRegister(program);
         curi = addInstrAfter(program, curi, genLIInstruction(NULL, reg, IMM(instr)));
         if (instr->opcode == OPC_MULI)
            curi = addInstrAfter(program, curi, genMULInstruction(NULL, RD(instr), RS1(instr), reg));
         else if (instr->opcode == OPC_DIVI)
            curi = addInstrAfter(program, curi, genDIVInstruction(NULL, RD(instr), RS1(instr), reg));
         removeInstructionLink(program, transformedInstrLnk);

      } else if (instr->opcode == OPC_SLLI || instr->opcode == OPC_SRLI || instr->opcode == OPC_SRAI) {
         instr->immediate = (unsigned)(instr->immediate) & 0x1F;
      }

      curi = curi->next;
   }
}

void fixPseudoInstructions(t_program_infos *program)
{
   t_list *curi = program->instructions;
   
   while (curi) {
      t_list *transformedInstrLnk = curi;
      t_axe_instruction *instr = curi->data;
      
      if (instr->opcode == OPC_SUBI) {
         instr->opcode = OPC_ADDI;
         instr->immediate = -instr->immediate;
         
      } else if (instr->opcode == OPC_SEQ || instr->opcode == OPC_SNE ||
            instr->opcode == OPC_SEQI || instr->opcode == OPC_SNEI) {
         if (instr->opcode == OPC_SEQ || instr->opcode == OPC_SNE)
            curi = addInstrAfter(program, curi, genSUBInstruction(NULL, RD(instr), RS1(instr), RS2(instr)));
         else
            curi = addInstrAfter(program, curi, genADDIInstruction(NULL, RD(instr), RS1(instr), -IMM(instr)));
         if (instr->opcode == OPC_SEQ || instr->opcode == OPC_SEQI)
            curi = addInstrAfter(program, curi, genSLTIUInstruction(NULL, RD(instr), RD(instr), 1));
         else
            curi = addInstrAfter(program, curi, genSLTUInstruction(NULL, RD(instr), REG_0, RD(instr)));
         removeInstructionLink(program, transformedInstrLnk);

      } else if ((instr->opcode == OPC_SGTI && IMM(instr) == INT32_MAX) ||
            instr->opcode == OPC_SGTIU && (uint32_t)IMM(instr) == UINT32_MAX) {
         curi = addInstrAfter(program, curi, genLIInstruction(NULL, RD(instr), 0));
         removeInstructionLink(program, transformedInstrLnk);

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
            t_axe_register *tmp = instr->reg_src1;
            instr->reg_src1 = instr->reg_src2;
            instr->reg_src2 = tmp;
            if (instr->opcode == OPC_SLE)
               instr->opcode = OPC_SLT;
            else if (instr->opcode == OPC_SLEU)
               instr->opcode = OPC_SLTU;
         }
         curi = addInstrAfter(program, curi, genXORIInstruction(NULL, RD(instr), RD(instr), 1));

      } else if ((instr->opcode == OPC_SLEI && IMM(instr) == INT32_MAX)
            || (instr->opcode == OPC_SLEIU && (uint32_t)IMM(instr) == UINT32_MAX)) {
         curi = addInstrAfter(program, curi, genLIInstruction(NULL, RD(instr), 1));
         removeInstructionLink(program, transformedInstrLnk);

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
      
      curi = curi->next;
   }
}

void fixSyscalls(t_program_infos *program)
{
   t_list *curi = program->instructions;

   while (curi) {
      t_list *transformedInstrLnk = curi;
      t_axe_instruction *instr = curi->data;
      t_axe_instruction *ecall;
      int r_func, func;

      if (instr->opcode != OPC_HALT && 
            instr->opcode != OPC_AXE_READ && 
            instr->opcode != OPC_AXE_WRITE) {
         curi = curi->next;
         continue;
      }

      if (instr->opcode == OPC_HALT)
         func = SYSCALL_ID_EXIT;
      else if (instr->opcode == OPC_AXE_WRITE)
         func = SYSCALL_ID_PUTINT;
      else if (instr->opcode == OPC_AXE_READ)
         func = SYSCALL_ID_GETINT;
      r_func = getNewRegister(program);
      curi = addInstrAfter(program, curi, genLIInstruction(NULL, r_func, func));

      if (instr->opcode == OPC_HALT) {
         int r_arg = getNewRegister(program);
         curi = addInstrAfter(program, curi, genLIInstruction(NULL, r_arg, 0));
         ecall = genInstruction(NULL, OPC_ECALL, REG_INVALID, r_func, r_arg, NULL, 0);
         curi = addInstrAfter(program, curi, ecall);

      } else if (instr->opcode == OPC_AXE_WRITE) {
         int r_arg = getNewRegister(program);
         curi = addInstrAfter(program, curi, genADDIInstruction(NULL, r_arg, RS1(instr), 0));
         ecall = genInstruction(NULL, OPC_ECALL, REG_INVALID, r_func, r_arg, NULL, 0);
         curi = addInstrAfter(program, curi, ecall);

      } else if (instr->opcode == OPC_AXE_READ) {
         int r_res = getNewRegister(program);
         ecall = genInstruction(NULL, OPC_ECALL, r_res, r_func, REG_INVALID, NULL, 0);
         curi = addInstrAfter(program, curi, ecall);
         curi = addInstrAfter(program, curi, genADDIInstruction(NULL, RD(instr), r_res, 0));
      }

      if (ecall->reg_dest)
         setMCRegisterWhitelist(ecall->reg_dest, REG_A0, -1);
      if (ecall->reg_src1)
         setMCRegisterWhitelist(ecall->reg_src1, REG_A0, -1);
      if (ecall->reg_src2)
         setMCRegisterWhitelist(ecall->reg_src2, REG_A1, -1);

      removeInstructionLink(program, transformedInstrLnk);

      curi = curi->next;
   }
}

void doTargetSpecificTransformations(t_program_infos *program)
{
   fixPseudoInstructions(program);
   fixSyscalls(program);
   fixUnsupportedImmediates(program);
}
