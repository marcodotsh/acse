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


static t_axe_instruction * genUnaryInstruction (t_program_infos *program
         , int opcode, int r_dest, t_axe_label *label, int addr);
static t_axe_instruction * genBinaryInstruction (t_program_infos *program
      , int opcode, int r_dest, int r_source1, int immediate);
static t_axe_instruction * genTernaryInstruction (t_program_infos *program
      , int opcode, int r_dest, int r_source1, int r_source2, int flags);
static t_axe_instruction * genJumpInstruction (t_program_infos *program
      , int opcode, t_axe_label *label, int addr);


void genMoveImmediate(t_program_infos *program, int dest, int immediate)
{
   genADDIInstruction(program, dest, REG_0, immediate);
}

int genLoadImmediate(t_program_infos *program, int immediate)
{
   int imm_register;

   imm_register = getNewRegister(program);

   genMoveImmediate(program, imm_register, immediate);

   return imm_register;
}

t_axe_instruction * genADDIInstruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return genBinaryInstruction(program, OPC_ADDI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * genSUBIInstruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return genBinaryInstruction(program, OPC_SUBI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * genANDIInstruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return genBinaryInstruction(program, OPC_ANDI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * genMULIInstruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return genBinaryInstruction(program, OPC_MULI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * genORIInstruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return genBinaryInstruction(program, OPC_ORI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * genXORIInstruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return genBinaryInstruction(program, OPC_XORI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * genDIVIInstruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return genBinaryInstruction(program, OPC_DIVI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * genSLLIInstruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return genBinaryInstruction(program, OPC_SLLI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * genSRAIInstruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return genBinaryInstruction(program, OPC_SRAI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * genREADInstruction
               (t_program_infos *program, int r_dest)
{
   return genUnaryInstruction(program, OPC_AXE_READ, r_dest, NULL, 0);
}

t_axe_instruction * genWRITEInstruction
               (t_program_infos *program, int r_dest)
{
   return genUnaryInstruction(program, OPC_AXE_WRITE, r_dest, NULL, 0);
}

t_axe_instruction * genLAInstruction
      (t_program_infos *program, int r_dest, t_axe_label *label, int address)
{
   return genUnaryInstruction(program, OPC_LA, r_dest, label, address);
}

t_axe_instruction * genHALTInstruction
      (t_program_infos *program)
{
   t_axe_instruction *instr;

   /* test if program is initialized */
   if (program == NULL)
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);
      
   /* create an instance of `t_axe_instruction' */
   instr = initializeInstruction(OPC_HALT);

   if (instr == NULL)
      notifyError(AXE_OUT_OF_MEMORY);

   /* add the newly created instruction to the current program */
   addInstruction(program, instr);

   /* return the load instruction */
   return instr;
}

t_axe_instruction * genNOPInstruction(t_program_infos *program)
{
   t_axe_instruction *instr;

   /* test if program is initialized */
   if (program == NULL)
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);
   
   /* create an instance of `t_axe_instruction' */
   instr = initializeInstruction(OPC_NOP);

   if (instr == NULL)
      notifyError(AXE_OUT_OF_MEMORY);

   /* add the newly created instruction to the current program */
   addInstruction(program, instr);

   /* return the load instruction */
   return instr;
}




/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/* TODO: REMOVE FROM HERE */
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

t_axe_instruction * OLDgenBTInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, OPC_OLD_BT, label, addr);
}

t_axe_instruction * OLDgenBFInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, OPC_OLD_BF, label, addr);
}

t_axe_instruction * OLDgenBHIInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, OPC_OLD_BHI, label, addr);
}      
      
t_axe_instruction * OLDgenBLSInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, OPC_OLD_BLS, label, addr);
}

t_axe_instruction * OLDgenBCCInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, OPC_OLD_BCC, label, addr);
}

t_axe_instruction * OLDgenBCSInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, OPC_OLD_BCS, label, addr);
}

t_axe_instruction * OLDgenBNEInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, OPC_OLD_BNE, label, addr);
}

t_axe_instruction * OLDgenBEQInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, OPC_OLD_BEQ, label, addr);
}

t_axe_instruction * OLDgenBVCInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, OPC_OLD_BVC, label, addr);
}

t_axe_instruction * OLDgenBvsInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, OPC_OLD_BVS, label, addr);
}

t_axe_instruction * OLDgenBPLInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, OPC_OLD_BPL, label, addr);
}

t_axe_instruction * OLDgenBMIInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, OPC_OLD_BMI, label, addr);
}

t_axe_instruction * OLDgenBGEInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, OPC_OLD_BGE, label, addr);
}

t_axe_instruction * OLDgenBLTInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, OPC_OLD_BLT, label, addr);
}

t_axe_instruction * OLDgenBGTInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, OPC_OLD_BGT, label, addr);
}

t_axe_instruction * OLDgenBLEInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, OPC_OLD_BLE, label, addr);
}

t_axe_instruction * OLDgenADDInstruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return genTernaryInstruction
         (program, OPC_ADD, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * OLDgenSUBInstruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return genTernaryInstruction
         (program, OPC_SUB, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * OLDgenANDInstruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return genTernaryInstruction
         (program, OPC_AND, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * OLDgenORInstruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return genTernaryInstruction
         (program, OPC_OR, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * OLDgenXORInstruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return genTernaryInstruction
         (program, OPC_XOR, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * OLDgenMULInstruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return genTernaryInstruction
         (program, OPC_MUL, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * OLDgenDIVInstruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return genTernaryInstruction
         (program, OPC_DIV, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * OLDgenSLLInstruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return genTernaryInstruction
         (program, OPC_SLL, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * OLDgenSRAInstruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return genTernaryInstruction
         (program, OPC_SRA, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * OLDgenANDLInstruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return genTernaryInstruction
         (program, OPC_OLD_ANDL, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * OLDgenORLInstruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return genTernaryInstruction
         (program, OPC_OLD_ORL, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * OLDgenEORLInstruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return genTernaryInstruction
         (program, OPC_OLD_EORL, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * OLDgenNEGInstruction (t_program_infos *program
      , int r_dest, int r_source, int flags)
{
   return genTernaryInstruction(program, OPC_OLD_NEG, r_dest, REG_0, r_source, flags);
}

t_axe_instruction * OLDgenSPCLInstruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return genTernaryInstruction
         (program, OPC_OLD_SPCL, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * OLDgenANDLIInstruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return genBinaryInstruction(program, OPC_OLD_ANDLI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * OLDgenORLIInstruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return genBinaryInstruction(program, OPC_OLD_ORLI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * OLDgenEORLIInstruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return genBinaryInstruction(program, OPC_OLD_EORLI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * OLDgenNOTLInstruction
      (t_program_infos *program, int r_dest, int r_source1)
{
   return genBinaryInstruction(program, OPC_OLD_NOTL
         , r_dest, r_source1, 0);
}

t_axe_instruction * OLDgenNOTBInstruction
      (t_program_infos *program, int r_dest, int r_source1)
{
   return genBinaryInstruction(program, OPC_OLD_NOTB
         , r_dest, r_source1, 0);
}

t_axe_instruction * OLDgenLOADInstruction
      (t_program_infos *program, int r_dest, t_axe_label *label, int address)
{
   return genUnaryInstruction(program, OPC_OLD_LOAD, r_dest, label, address);
}

t_axe_instruction * OLDgenSTOREInstruction
      (t_program_infos *program, int r_dest, t_axe_label *label, int address)
{
   return genUnaryInstruction(program, OPC_OLD_STORE, r_dest, label, address);
}

t_axe_instruction * OLDgenSGEInstruction
                  (t_program_infos *program, int r_dest)
{
   return genUnaryInstruction(program, OPC_OLD_SGE, r_dest, NULL, 0);
}
   
t_axe_instruction * OLDgenSEQInstruction
                  (t_program_infos *program, int r_dest)
{
   return genUnaryInstruction(program, OPC_OLD_SEQ, r_dest, NULL, 0);
}

t_axe_instruction * OLDgenSGTInstruction
                  (t_program_infos *program, int r_dest)
{
   return genUnaryInstruction(program, OPC_OLD_SGT, r_dest, NULL, 0);
}

t_axe_instruction * OLDgenSLEInstruction
                  (t_program_infos *program, int r_dest)
{
   return genUnaryInstruction(program, OPC_OLD_SLE, r_dest, NULL, 0);
}

t_axe_instruction * OLDgenSLTInstruction
                  (t_program_infos *program, int r_dest)
{
   return genUnaryInstruction(program, OPC_OLD_SLT, r_dest, NULL, 0);
}

t_axe_instruction * OLDgenSNEInstruction
                  (t_program_infos *program, int r_dest)
{
   return genUnaryInstruction(program, OPC_OLD_SNE, r_dest, NULL, 0);
}

t_axe_instruction * genUnaryInstruction (t_program_infos *program
         , int opcode, int r_dest, t_axe_label *label, int addr)
{
   t_axe_instruction *instr;
   t_axe_register *reg;
   t_axe_address *address;
   int addressType;

   if (r_dest == REG_INVALID)
      notifyError(AXE_INVALID_REGISTER_INFO);
   
   /* test if value is correctly initialized */
   if (label != NULL)
   {
      /* address type is a label type */
      addressType = LABEL_TYPE;
   }
   else
   {
      if (addr < 0)
         notifyError(AXE_INVALID_ADDRESS);

      /* address type is a label type */
      addressType = ADDRESS_TYPE;

   }

   /* test if the opcode is a valid opcode */
   if (opcode == OPC_INVALID)
      notifyError(AXE_INVALID_OPCODE);

   /* create an instance of `t_axe_instruction' */
   instr = initializeInstruction(opcode);

   if (instr == NULL)
      notifyError(AXE_OUT_OF_MEMORY);

   /* initialize a register info */
   reg = initializeRegister(r_dest, 0);

   if (reg == NULL)
   {
      finalizeInstruction(instr);
      notifyError(AXE_OUT_OF_MEMORY);
   }
   
   /* update the reg_dest info */
   if (opcode != OPC_AXE_WRITE && opcode != OPC_OLD_STORE) {
      instr->reg_dest = reg;
   } else {
      instr->reg_src1 = reg;
   }

   /* initialize an address info */
   address = initializeAddress(addressType, addr, label);
   
   if (address == NULL)
   {
      finalizeInstruction(instr);
      notifyError(AXE_OUT_OF_MEMORY);
   }

   /* update the instruction address info */
   instr->address = address;

   /* add the newly created instruction to the current program */
   if (program != NULL)
      addInstruction(program, instr);

   /* return the load instruction */
   return instr;
}

t_axe_instruction * genBinaryInstruction (t_program_infos *program
      , int opcode, int r_dest, int r_source1, int immediate)
{
   t_axe_instruction *instr;
   t_axe_register *reg_dest;
   t_axe_register *reg_source1;

   /* test if value is correctly initialized */
   if (  (r_dest == REG_INVALID)
         || (r_source1 == REG_INVALID) )
   {
      notifyError(AXE_INVALID_REGISTER_INFO);
   }

   /* create an instance of `t_axe_instruction' */
   instr = initializeInstruction(opcode);

   if (instr == NULL)
      notifyError(AXE_OUT_OF_MEMORY);

   /* initialize a register info */
   reg_dest = initializeRegister(r_dest, 0);
   if (reg_dest == NULL)
   {
      finalizeInstruction(instr);
      notifyError(AXE_OUT_OF_MEMORY);
   }

   /* update the reg_dest info */
   instr->reg_dest = reg_dest;

   reg_source1 = initializeRegister(r_source1, 0);
   if (reg_source1 == NULL)
   {
      finalizeInstruction(instr);
      notifyError(AXE_OUT_OF_MEMORY);
      return NULL;
   }

   /* update the reg_dest info */
   instr->reg_src1 = reg_source1;

   /* assign an immediate value */
   instr->immediate = immediate;

   /* add the newly created instruction to the current program */
   if (program != NULL)
      addInstruction(program, instr);

   /* return the load instruction */
   return instr;
}

t_axe_instruction * genTernaryInstruction (t_program_infos *program
      , int opcode, int r_dest, int r_source1, int r_source2, int flags)
{
   t_axe_instruction *instr;
   t_axe_register *reg_dest;
   t_axe_register *reg_source1;
   t_axe_register *reg_source2;

   /* test if value is correctly initialized */
   if (  (r_dest == REG_INVALID)
         || (r_source1 == REG_INVALID)
         || (r_source2 == REG_INVALID))
   {
      notifyError(AXE_INVALID_REGISTER_INFO);
   }

   /* create an instance of `t_axe_instruction' */
   instr = initializeInstruction(opcode);

   if (instr == NULL)
      notifyError(AXE_OUT_OF_MEMORY);

   /* initialize a register info */
   reg_dest = initializeRegister(r_dest, ((flags & CG_INDIRECT_DEST)? 1 : 0) );
   if (reg_dest == NULL)
   {
      finalizeInstruction(instr);
      notifyError(AXE_OUT_OF_MEMORY);
   }

   /* update the reg_dest info */
   instr->reg_dest = reg_dest;

   reg_source1 = initializeRegister(r_source1, 0);
   if (reg_source1 == NULL)
   {
      finalizeInstruction(instr);
      notifyError(AXE_OUT_OF_MEMORY);
   }

   /* update the reg_src1 info */
   instr->reg_src1 = reg_source1;

   reg_source2 = initializeRegister(r_source2, ((flags & CG_INDIRECT_SOURCE)? 1 : 0));
   if (reg_source1 == NULL)
   {
      finalizeInstruction(instr);
      notifyError(AXE_OUT_OF_MEMORY);
   }

   /* update the reg_src2 info */
   instr->reg_src2 = reg_source2;

   /* add the newly created instruction to the current program */
   if (program != NULL)
      addInstruction(program, instr);

   /* return the load instruction */
   return instr;
}

t_axe_instruction * genJumpInstruction (t_program_infos *program
      , int opcode, t_axe_label *label, int addr)
{
   t_axe_instruction *instr;
   t_axe_address * address;
   int addressType;

   /* test if value is correctly initialized */
   if (label != NULL)
   {
      addressType = LABEL_TYPE;
   }
   else
   {
      if (addr < 0)
         notifyError(AXE_INVALID_ADDRESS);

      addressType = ADDRESS_TYPE;
   }

   /* test if the opcode is a valid opcode */
   if (opcode == OPC_INVALID)
      notifyError(AXE_INVALID_OPCODE);

   /* create an instance of `t_axe_instruction' */
   instr = initializeInstruction(opcode);

   if (instr == NULL)
      notifyError(AXE_OUT_OF_MEMORY);

   /* initialize an address info */
   address = initializeAddress(addressType, addr, label);
   
   if (address == NULL)
   {
      finalizeInstruction(instr);
      notifyError(AXE_OUT_OF_MEMORY);
   }

   /* update the instruction address info */
   instr->address = address;

   /* add the newly created instruction to the current program */
   if (program != NULL)
      addInstruction(program, instr);

   /* return the load instruction */
   return instr;
}
