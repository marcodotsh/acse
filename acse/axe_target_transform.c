/*
 * Daniele Cattaneo
 * Politecnico di Milano, 2020
 * 
 * axe_mace_transform.h
 * Formal Languages & Compilers Machine, 2007-2020
 */

#include <assert.h>
#include "axe_target_transform.h"
#include "axe_gencode.h"
#include "axe_utils.h"
#include "collections.h"
#include "axe_target_info.h"
#include "axe_target_asm_print.h"

#define RD(i)  (i->reg_dest->ID)
#define RS1(i) (i->reg_src1->ID)
#define RS2(i) (i->reg_src2->ID)
#define IMM(i) (i->immediate)

void genUseOfPhysReg(t_program_infos *program, int physReg)
{
   int reg = getNewRegister(program);
   t_axe_register *rstruct = initializeRegister(reg, 0);
   t_axe_instruction *instr = genInstruction(program, CFLOW_OPC_USE,
         NULL, rstruct, NULL, NULL, 0);
   setMCRegisterWhitelist(rstruct, physReg, REG_INVALID);
}

int genDefOfPhysReg(t_program_infos *program, int physReg)
{
   int reg = getNewRegister(program);
   t_axe_register *rstruct = initializeRegister(reg, 0);
   t_axe_instruction *instr = genInstruction(program, CFLOW_OPC_DEFINE,
         rstruct, NULL, NULL, NULL, 0);
   setMCRegisterWhitelist(rstruct, physReg, REG_INVALID);
   return reg;
}

void moveLabel(t_axe_instruction *dest, t_axe_instruction *src)
{
   assert(dest->labelID == NULL && "moveLabel failed: destination already is labeled");
   dest->labelID = src->labelID;
   src->labelID = NULL;
   
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

int is_int12(int immediate)
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
         if (!is_int12(instr->immediate)) {
            pushInstrInsertionPoint(program, curi);
            genLIInstruction(program, RD(instr), IMM(instr));
            removeInstructionLink(program, curi);
            popInstrInsertionPoint(program);
         }

      } else if (instr->opcode == OPC_MULI || instr->opcode == OPC_DIVI ||
            !is_int12(instr->immediate)) {
         int reg = getNewRegister(program);
         pushInstrInsertionPoint(program, curi->prev);
         moveLabel(genLIInstruction(program, reg, IMM(instr)), instr);
         instr->immediate = 0;
         instr->reg_src2 = initializeRegister(reg, 0);
         instr->opcode = switchOpcodeImmediateForm(instr->opcode);
         popInstrInsertionPoint(program);
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
      
      } else if ((instr->opcode == OPC_SGTI && IMM(instr) == INT32_MAX) ||
            instr->opcode == OPC_SGTIU && (uint32_t)IMM(instr) == UINT32_MAX) {
         pushInstrInsertionPoint(program, curi);
         genLIInstruction(program, RD(instr), 0);
         removeInstructionLink(program, curi);
         popInstrInsertionPoint(program);
         
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

      } else if (instr->opcode == OPC_SGT || instr->opcode == OPC_SGTU) {
         if (instr->opcode == OPC_SGT)
            instr->opcode = OPC_SLT;
         else
            instr->opcode = OPC_SLTU;
         t_axe_register *tmp = instr->reg_src1;
         instr->reg_src1 = instr->reg_src2;
         instr->reg_src2 = tmp;
      }
      
      curi = nexti;
   }
}

void markRegistersTouchedByCall(t_program_infos *program, int numRet)
{
   /* list of registers potentially affected by a function call */
   static const int regList[] = {
      REG_A0, REG_A1, REG_A2, REG_A3, REG_A4, REG_A5, REG_A6, REG_A7,
      REG_T0, REG_T1, REG_T2
   };

   for (int i = numRet; i<sizeof(regList)/sizeof(int); i++) {
      int reg = regList[i];
      genDefOfPhysReg(program, reg);
   }
}

void fixSyscalls(t_program_infos *program)
{
   t_list *curi = program->instructions;

   while (curi) {
      t_axe_instruction *instr = LDATA(curi);
      t_list *nexti = LNEXT(curi);
      int callid, rcallid, rarg;
      t_axe_instruction *i1 = NULL, *i2 = NULL, *i3 = NULL;
      t_list *afterCall;

      if (instr->opcode != OPC_HALT && 
            instr->opcode != OPC_AXE_READ && 
            instr->opcode != OPC_AXE_WRITE) {
         curi = nexti;
         continue;
      }

      pushInstrInsertionPoint(program, curi);
      
      if (instr->opcode == OPC_HALT)
         callid = 93;
      else if (instr->opcode == OPC_AXE_READ)
         callid = 0x10002;
      else if (instr->opcode == OPC_AXE_WRITE)
         callid = 0x10003;
      rcallid = getNewRegister(program);
      i1 = genLIInstruction(program, rcallid, callid);
      setMCRegisterWhitelist(i1->reg_dest, REG_A0, REG_INVALID);

      rarg = getNewRegister(program);
      if (instr->opcode == OPC_HALT) {
         i2 = genLIInstruction(program, rarg, 0);
      } else if (instr->opcode == OPC_AXE_WRITE) {
         i2 = genADDIInstruction(program, rarg, RS1(instr), 0);
      }
      if (i2)
         setMCRegisterWhitelist(i2->reg_dest, REG_A1, REG_INVALID);

      genECALLInstruction(program);
      

      if (instr->opcode == OPC_AXE_READ) {
         markRegistersTouchedByCall(program, 1);
         rarg = genDefOfPhysReg(program, REG_A0);
         genADDIInstruction(program, RD(instr), rarg, 0);
      } else {
         markRegistersTouchedByCall(program, 0);
      }

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
