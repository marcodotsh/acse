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

void fixLargeImmediates(t_program_infos *program)
{
   t_list *curi = program->instructions;
   
   while (curi) {
      t_axe_instruction *instr = LDATA(curi);
      
      if (!isImmediateArgumentInstrOpcode(instr->opcode) || is_int12(instr->immediate)) {
         curi = LNEXT(curi);
         continue;
      }
      t_list *nexti = LNEXT(curi);
      
      if (instr->opcode == OPC_ADDI && instr->reg_src1->ID == REG_0) {
         pushInstrInsertionPoint(program, curi);
         int reg = instr->reg_dest->ID;
         genLIInstruction(program, reg, instr->immediate);
         removeInstructionLink(program, curi);
         popInstrInsertionPoint(program);
         
      } else {
         pushInstrInsertionPoint(program, curi->prev);
         int reg = getNewRegister(program);
         moveLabel(genLIInstruction(program, reg, instr->immediate), instr);
         instr->immediate = 0;
         instr->reg_src2 = initializeRegister(reg, 0);
         instr->opcode = switchOpcodeImmediateForm(instr->opcode);
         popInstrInsertionPoint(program);
      }
      
      curi = nexti;
   }
}

void doTargetSpecificTransformations(t_program_infos *program)
{
   fixLargeImmediates(program);
}
