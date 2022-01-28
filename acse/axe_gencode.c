/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_gencode.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#include "axe_gencode.h"
#include "axe_errors.h"
#include "axe_target_info.h"


void genMoveImmediate(t_program_infos *program, int dest, int immediate)
{
   genLIInstruction(program, dest, immediate);
}

int genLoadImmediate(t_program_infos *program, int immediate)
{
   int imm_register = getNewRegister(program);
   genMoveImmediate(program, imm_register, immediate);
   return imm_register;
}


static t_axe_instruction *genRFormatInstruction(t_program_infos *program,
      int opcode, int rd, int rs1, int rs2)
{
   return genInstruction(program, opcode,
         initializeRegister(rd, 0),
         initializeRegister(rs1, 0),
         initializeRegister(rs2, 0),
         NULL,
         0);
}

static t_axe_instruction *genIFormatInstruction(t_program_infos *program,
      int opcode, int rd, int rs1, int immediate)
{
   return genInstruction(program, opcode,
         initializeRegister(rd, 0),
         initializeRegister(rs1, 0),
         NULL,
         NULL,
         immediate);
}

static t_axe_instruction *genBFormatInstruction(t_program_infos *program,
      int opcode, int rs1, int rs2, t_axe_label *label)
{
   return genInstruction(program, opcode,
         NULL,
         initializeRegister(rs1, 0),
         initializeRegister(rs2, 0),
         label,
         0);
}


t_axe_instruction *genADDInstruction(t_program_infos *program,
   int r_dest, int r_src1, int r_src2)
{
   return genRFormatInstruction(program, OPC_ADD, r_dest, r_src1, r_src2);
}

t_axe_instruction *genSUBInstruction(t_program_infos *program,
   int r_dest, int r_src1, int r_src2)
{
   return genRFormatInstruction(program, OPC_SUB, r_dest, r_src1, r_src2);
}

t_axe_instruction *genANDInstruction(t_program_infos *program,
   int r_dest, int r_src1, int r_src2)
{
   return genRFormatInstruction(program, OPC_AND, r_dest, r_src1, r_src2);
}

t_axe_instruction *genORInstruction(t_program_infos *program,
   int r_dest, int r_src1, int r_src2)
{
   return genRFormatInstruction(program, OPC_OR, r_dest, r_src1, r_src2);
}

t_axe_instruction *genXORInstruction(t_program_infos *program,
   int r_dest, int r_src1, int r_src2)
{
   return genRFormatInstruction(program, OPC_XOR, r_dest, r_src1, r_src2);
}

t_axe_instruction *genMULInstruction(t_program_infos *program,
   int r_dest, int r_src1, int r_src2)
{
   return genRFormatInstruction(program, OPC_MUL, r_dest, r_src1, r_src2);
}

t_axe_instruction *genDIVInstruction(t_program_infos *program,
   int r_dest, int r_src1, int r_src2)
{
   return genRFormatInstruction(program, OPC_DIV, r_dest, r_src1, r_src2);
}

t_axe_instruction *genSLLInstruction(t_program_infos *program,
   int r_dest, int r_src1, int r_src2)
{
   return genRFormatInstruction(program, OPC_SLL, r_dest, r_src1, r_src2);
}

t_axe_instruction *genSRLInstruction(t_program_infos *program,
   int r_dest, int r_src1, int r_src2)
{
   return genRFormatInstruction(program, OPC_SRL, r_dest, r_src1, r_src2);
}

t_axe_instruction *genSRAInstruction(t_program_infos *program,
   int r_dest, int r_src1, int r_src2)
{
   return genRFormatInstruction(program, OPC_SRA, r_dest, r_src1, r_src2);
}


t_axe_instruction * genADDIInstruction
      (t_program_infos *program, int r_dest, int r_src1, int immediate)
{
   return genIFormatInstruction(program, OPC_ADDI, r_dest, r_src1, immediate);
}

t_axe_instruction * genSUBIInstruction
      (t_program_infos *program, int r_dest, int r_src1, int immediate)
{
   return genIFormatInstruction(program, OPC_SUBI, r_dest, r_src1, immediate);
}

t_axe_instruction * genANDIInstruction
      (t_program_infos *program, int r_dest, int r_src1, int immediate)
{
   return genIFormatInstruction(program, OPC_ANDI, r_dest, r_src1, immediate);
}

t_axe_instruction * genMULIInstruction
      (t_program_infos *program, int r_dest, int r_src1, int immediate)
{
   return genIFormatInstruction(program, OPC_MULI, r_dest, r_src1, immediate);
}

t_axe_instruction * genORIInstruction
      (t_program_infos *program, int r_dest, int r_src1, int immediate)
{
   return genIFormatInstruction(program, OPC_ORI, r_dest, r_src1, immediate);
}

t_axe_instruction * genXORIInstruction
      (t_program_infos *program, int r_dest, int r_src1, int immediate)
{
   return genIFormatInstruction(program, OPC_XORI, r_dest, r_src1, immediate);
}

t_axe_instruction * genDIVIInstruction
      (t_program_infos *program, int r_dest, int r_src1, int immediate)
{
   return genIFormatInstruction(program, OPC_DIVI, r_dest, r_src1, immediate);
}

t_axe_instruction * genSLLIInstruction
      (t_program_infos *program, int r_dest, int r_src1, int immediate)
{
   return genIFormatInstruction(program, OPC_SLLI, r_dest, r_src1, immediate);
}

t_axe_instruction *genSRLIInstruction(
      t_program_infos *program, int r_dest, int r_src1, int immediate)
{
   return genIFormatInstruction(program, OPC_SRLI, r_dest, r_src1, immediate);
}

t_axe_instruction * genSRAIInstruction
      (t_program_infos *program, int r_dest, int r_src1, int immediate)
{
   return genIFormatInstruction(program, OPC_SRAI, r_dest, r_src1, immediate);
}


t_axe_instruction *genSEQInstruction(t_program_infos *program,
      int r_dest, int r_src1, int r_src2)
{
   return genRFormatInstruction(program, OPC_SEQ, r_dest, r_src1, r_src2);
}

t_axe_instruction *genSNEInstruction(t_program_infos *program,
      int r_dest, int r_src1, int r_src2)
{
   return genRFormatInstruction(program, OPC_SNE, r_dest, r_src1, r_src2);
}

t_axe_instruction *genSLTInstruction(t_program_infos *program,
      int r_dest, int r_src1, int r_src2)
{
   return genRFormatInstruction(program, OPC_SLT, r_dest, r_src1, r_src2);
}

t_axe_instruction *genSLTUInstruction(t_program_infos *program,
      int r_dest, int r_src1, int r_src2)
{
   return genRFormatInstruction(program, OPC_SLTU, r_dest, r_src1, r_src2);
}

t_axe_instruction *genSGEInstruction(t_program_infos *program,
      int r_dest, int r_src1, int r_src2)
{
   return genRFormatInstruction(program, OPC_SGE, r_dest, r_src1, r_src2);
}

t_axe_instruction *genSGEUInstruction(t_program_infos *program,
      int r_dest, int r_src1, int r_src2)
{
   return genRFormatInstruction(program, OPC_SGEU, r_dest, r_src1, r_src2);
}

t_axe_instruction *genSGTInstruction(t_program_infos *program,
      int r_dest, int r_src1, int r_src2)
{
   return genRFormatInstruction(program, OPC_SGT, r_dest, r_src1, r_src2);
}

t_axe_instruction *genSGTUInstruction(t_program_infos *program,
      int r_dest, int r_src1, int r_src2)
{
   return genRFormatInstruction(program, OPC_SGTU, r_dest, r_src1, r_src2);
}

t_axe_instruction *genSLEInstruction(t_program_infos *program,
      int r_dest, int r_src1, int r_src2)
{
   return genRFormatInstruction(program, OPC_SLE, r_dest, r_src1, r_src2);
}

t_axe_instruction *genSLEUInstruction(t_program_infos *program,
      int r_dest, int r_src1, int r_src2)
{
   return genRFormatInstruction(program, OPC_SLEU, r_dest, r_src1, r_src2);
}


t_axe_instruction *genSEQIInstruction(t_program_infos *program,
      int r_dest, int r_src1, int immediate)
{
   return genIFormatInstruction(program, OPC_SEQI, r_dest, r_src1, immediate);
}

t_axe_instruction *genSNEIInstruction(t_program_infos *program,
      int r_dest, int r_src1, int immediate)
{
   return genIFormatInstruction(program, OPC_SNEI, r_dest, r_src1, immediate);
}

t_axe_instruction *genSLTIInstruction(t_program_infos *program,
      int r_dest, int r_src1, int immediate)
{
   return genIFormatInstruction(program, OPC_SLTI, r_dest, r_src1, immediate);
}

t_axe_instruction *genSLTIUInstruction(t_program_infos *program,
      int r_dest, int r_src1, int immediate)
{
   return genIFormatInstruction(program, OPC_SLTIU, r_dest, r_src1, immediate);
}

t_axe_instruction *genSGEIInstruction(t_program_infos *program,
      int r_dest, int r_src1, int immediate)
{
   return genIFormatInstruction(program, OPC_SGEI, r_dest, r_src1, immediate);
}

t_axe_instruction *genSGEIUInstruction(t_program_infos *program,
      int r_dest, int r_src1, int immediate)
{
   return genIFormatInstruction(program, OPC_SGEIU, r_dest, r_src1, immediate);
}

t_axe_instruction *genSGTIInstruction(t_program_infos *program,
      int r_dest, int r_src1, int immediate)
{
   return genIFormatInstruction(program, OPC_SGTI, r_dest, r_src1, immediate);
}

t_axe_instruction *genSGTIUInstruction(t_program_infos *program,
      int r_dest, int r_src1, int immediate)
{
   return genIFormatInstruction(program, OPC_SGTIU, r_dest, r_src1, immediate);
}

t_axe_instruction *genSLEIInstruction(t_program_infos *program,
      int r_dest, int r_src1, int immediate)
{
   return genIFormatInstruction(program, OPC_SLEI, r_dest, r_src1, immediate);
}

t_axe_instruction *genSLEIUInstruction(t_program_infos *program,
      int r_dest, int r_src1, int immediate)
{
   return genIFormatInstruction(program, OPC_SLEIU, r_dest, r_src1, immediate);
}


t_axe_instruction *genJInstruction(t_program_infos *program,
      t_axe_label *label)
{
   return genInstruction(program, OPC_J,
         NULL,
         NULL,
         NULL,
         label,
         0);
}


t_axe_instruction *genBEQInstruction(t_program_infos *program,
      int rs1, int rs2, t_axe_label *label)
{
   return genBFormatInstruction(program, OPC_BEQ, rs1, rs2, label);
}

t_axe_instruction *genBNEInstruction(t_program_infos *program,
      int rs1, int rs2, t_axe_label *label)
{
   return genBFormatInstruction(program, OPC_BEQ, rs1, rs2, label);
}

t_axe_instruction *genBLTInstruction(t_program_infos *program,
      int rs1, int rs2, t_axe_label *label)
{
   return genBFormatInstruction(program, OPC_BEQ, rs1, rs2, label);
}

t_axe_instruction *genBLTUInstruction(t_program_infos *program,
      int rs1, int rs2, t_axe_label *label)
{
   return genBFormatInstruction(program, OPC_BEQ, rs1, rs2, label);
}

t_axe_instruction *genBGEInstruction(t_program_infos *program,
      int rs1, int rs2, t_axe_label *label)
{
   return genBFormatInstruction(program, OPC_BEQ, rs1, rs2, label);
}

t_axe_instruction *genBGEUInstruction(t_program_infos *program,
      int rs1, int rs2, t_axe_label *label)
{
   return genBFormatInstruction(program, OPC_BEQ, rs1, rs2, label);
}

t_axe_instruction *genBGTInstruction(t_program_infos *program,
      int rs1, int rs2, t_axe_label *label)
{
   return genBFormatInstruction(program, OPC_BEQ, rs1, rs2, label);
}

t_axe_instruction *genBGTUInstruction(t_program_infos *program,
      int rs1, int rs2, t_axe_label *label)
{
   return genBFormatInstruction(program, OPC_BEQ, rs1, rs2, label);
}

t_axe_instruction *genBLEInstruction(t_program_infos *program,
      int rs1, int rs2, t_axe_label *label)
{
   return genBFormatInstruction(program, OPC_BEQ, rs1, rs2, label);
}

t_axe_instruction *genBLEUInstruction(t_program_infos *program,
      int rs1, int rs2, t_axe_label *label)
{
   return genBFormatInstruction(program, OPC_BEQ, rs1, rs2, label);
}


t_axe_instruction *genLIInstruction(
      t_program_infos *program, int r_dest, int immediate)
{
   return genInstruction(program, OPC_LI,
         initializeRegister(r_dest, 0),
         NULL,
         NULL,
         NULL,
         immediate);
}

t_axe_instruction * genLAInstruction(t_program_infos *program,
      int r_dest, t_axe_label *label)
{
   return genInstruction(program, OPC_LA,
         initializeRegister(r_dest, 0),
         NULL,
         NULL,
         label,
         0);
}

t_axe_instruction *genLWInstruction(
      t_program_infos *program, int r_dest, int immediate, int rs1)
{
   return genInstruction(program, OPC_LW,
         initializeRegister(r_dest, 0),
         initializeRegister(rs1, 0),
         NULL,
         NULL,
         immediate);
}

t_axe_instruction *genSWInstruction(
      t_program_infos *program, int rs2, int immediate, int rs1)
{
   return genInstruction(program, OPC_SW,
         NULL,
         initializeRegister(rs1, 0),
         initializeRegister(rs2, 0),
         NULL,
         immediate);
}

t_axe_instruction *genLWGlobalInstruction(
      t_program_infos *program, int r_dest, t_axe_label *label)
{
   return genInstruction(program, OPC_LW_G,
         initializeRegister(r_dest, 0),
         NULL,
         NULL,
         label,
         0);
}

t_axe_instruction *genSWGlobalInstruction(
      t_program_infos *program, int rs2, t_axe_label *label)
{
   return genInstruction(program, OPC_SW_G,
         NULL,
         initializeRegister(REG_T6, 0),
         initializeRegister(rs2, 0),
         label,
         0);
}


t_axe_instruction * genNOPInstruction(t_program_infos *program)
{
   return genInstruction(program, OPC_NOP, NULL, NULL, NULL, NULL, 0);
}

t_axe_instruction * genECALLInstruction(t_program_infos *program)
{
   return genInstruction(program, OPC_ECALL, NULL, NULL, NULL, NULL, 0);
}

t_axe_instruction * genEBREAKInstruction(t_program_infos *program)
{
   return genInstruction(program, OPC_EBREAK, NULL, NULL, NULL, NULL, 0);
}


t_axe_instruction * genHALTInstruction
      (t_program_infos *program)
{
   return genInstruction(program, OPC_HALT, NULL, NULL, NULL, NULL, 0);
}

t_axe_instruction * genREADInstruction(t_program_infos *program, int r_dest)
{
   return genInstruction(program, OPC_AXE_READ,
         initializeRegister(r_dest, 0), NULL, NULL, NULL, 0);
}

t_axe_instruction * genWRITEInstruction
               (t_program_infos *program, int r_src1)
{
   return genInstruction(program, OPC_AXE_WRITE,
         NULL, initializeRegister(r_src1, 0), NULL, NULL, 0);
}

