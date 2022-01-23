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

t_axe_instruction * genBTInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, BT, label, addr);
}

t_axe_instruction * genBFInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, BF, label, addr);
}

t_axe_instruction * genBHIInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, BHI, label, addr);
}      
      
t_axe_instruction * genBLSInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, BLS, label, addr);
}

t_axe_instruction * genBCCInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, BCC, label, addr);
}

t_axe_instruction * genBCSInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, BCS, label, addr);
}

t_axe_instruction * genBNEInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, BNE, label, addr);
}

t_axe_instruction * genBEQInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, BEQ, label, addr);
}

t_axe_instruction * genBVCInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, BVC, label, addr);
}

t_axe_instruction * genBvsInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, BVS, label, addr);
}

t_axe_instruction * genBPLInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, BPL, label, addr);
}

t_axe_instruction * genBMIInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, BMI, label, addr);
}

t_axe_instruction * genBGEInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, BGE, label, addr);
}

t_axe_instruction * genBLTInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, BLT, label, addr);
}

t_axe_instruction * genBGTInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, BGT, label, addr);
}

t_axe_instruction * genBLEInstruction
      (t_program_infos *program, t_axe_label *label, int addr)
{
   return genJumpInstruction (program, BLE, label, addr);
}

t_axe_instruction * genADDInstruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return genTernaryInstruction
         (program, ADD, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * genSUBInstruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return genTernaryInstruction
         (program, SUB, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * genANDLInstruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return genTernaryInstruction
         (program, ANDL, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * genORLInstruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return genTernaryInstruction
         (program, ORL, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * genEORLInstruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return genTernaryInstruction
         (program, EORL, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * genANDBInstruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return genTernaryInstruction
         (program, ANDB, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * genORBInstruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return genTernaryInstruction
         (program, ORB, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * genEORBInstruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return genTernaryInstruction
         (program, EORB, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * genMULInstruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return genTernaryInstruction
         (program, MUL, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * genDIVInstruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return genTernaryInstruction
         (program, DIV, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * genSHLInstruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return genTernaryInstruction
         (program, SHL, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * genSHRInstruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return genTernaryInstruction
         (program, SHR, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * genNEGInstruction (t_program_infos *program
      , int r_dest, int r_source, int flags)
{
   return genTernaryInstruction(program, NEG, r_dest, REG_0, r_source, flags);
}

t_axe_instruction * genSPCLInstruction (t_program_infos *program
      , int r_dest, int r_source1, int r_source2, int flags)
{
   return genTernaryInstruction
         (program, SPCL, r_dest, r_source1, r_source2, flags);
}

t_axe_instruction * genADDIInstruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return genBinaryInstruction(program, ADDI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * genSUBIInstruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return genBinaryInstruction(program, SUBI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * genANDLIInstruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return genBinaryInstruction(program, ANDLI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * genORLIInstruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return genBinaryInstruction(program, ORLI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * genEORLIInstruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return genBinaryInstruction(program, EORLI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * genANDBIInstruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return genBinaryInstruction(program, ANDBI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * genMULIInstruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return genBinaryInstruction(program, MULI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * genORBIInstruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return genBinaryInstruction(program, ORBI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * genEORBIInstruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return genBinaryInstruction(program, EORBI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * genDIVIInstruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return genBinaryInstruction(program, DIVI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * genSHLIInstruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return genBinaryInstruction(program, SHLI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * genSHRIInstruction
      (t_program_infos *program, int r_dest, int r_source1, int immediate)
{
   return genBinaryInstruction(program, SHRI
         , r_dest, r_source1, immediate);
}

t_axe_instruction * genNOTLInstruction
      (t_program_infos *program, int r_dest, int r_source1)
{
   return genBinaryInstruction(program, NOTL
         , r_dest, r_source1, 0);
}

t_axe_instruction * genNOTBInstruction
      (t_program_infos *program, int r_dest, int r_source1)
{
   return genBinaryInstruction(program, NOTB
         , r_dest, r_source1, 0);
}

t_axe_instruction * genREADInstruction
               (t_program_infos *program, int r_dest)
{
   return genUnaryInstruction(program, AXE_READ, r_dest, NULL, 0);
}

t_axe_instruction * genWRITEInstruction
               (t_program_infos *program, int r_dest)
{
   return genUnaryInstruction(program, AXE_WRITE, r_dest, NULL, 0);
}

t_axe_instruction * genLOADInstruction
      (t_program_infos *program, int r_dest, t_axe_label *label, int address)
{
   return genUnaryInstruction(program, LOAD, r_dest, label, address);
}

t_axe_instruction * genSTOREInstruction
      (t_program_infos *program, int r_dest, t_axe_label *label, int address)
{
   return genUnaryInstruction(program, STORE, r_dest, label, address);
}

t_axe_instruction * genMOVAInstruction
      (t_program_infos *program, int r_dest, t_axe_label *label, int address)
{
   return genUnaryInstruction(program, MOVA, r_dest, label, address);
}

t_axe_instruction * genSGEInstruction
                  (t_program_infos *program, int r_dest)
{
   return genUnaryInstruction(program, SGE, r_dest, NULL, 0);
}
   
t_axe_instruction * genSEQInstruction
                  (t_program_infos *program, int r_dest)
{
   return genUnaryInstruction(program, SEQ, r_dest, NULL, 0);
}

t_axe_instruction * genSGTInstruction
                  (t_program_infos *program, int r_dest)
{
   return genUnaryInstruction(program, SGT, r_dest, NULL, 0);
}

t_axe_instruction * genSLEInstruction
                  (t_program_infos *program, int r_dest)
{
   return genUnaryInstruction(program, SLE, r_dest, NULL, 0);
}

t_axe_instruction * genSLTInstruction
                  (t_program_infos *program, int r_dest)
{
   return genUnaryInstruction(program, SLT, r_dest, NULL, 0);
}

t_axe_instruction * genSNEInstruction
                  (t_program_infos *program, int r_dest)
{
   return genUnaryInstruction(program, SNE, r_dest, NULL, 0);
}

t_axe_instruction * genHALTInstruction
      (t_program_infos *program)
{
   t_axe_instruction *instr;

   /* test if program is initialized */
   if (program == NULL)
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);
      
   /* create an instance of `t_axe_instruction' */
   instr = initializeInstruction(HALT);

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
   instr = initializeInstruction(NOP);

   if (instr == NULL)
      notifyError(AXE_OUT_OF_MEMORY);

   /* add the newly created instruction to the current program */
   addInstruction(program, instr);

   /* return the load instruction */
   return instr;
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
   if (opcode == INVALID_OPCODE)
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
   
   /* update the reg_1 info */
   instr->reg_1 = reg;

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

   /* update the reg_1 info */
   instr->reg_1 = reg_dest;

   reg_source1 = initializeRegister(r_source1, 0);
   if (reg_source1 == NULL)
   {
      finalizeInstruction(instr);
      notifyError(AXE_OUT_OF_MEMORY);
      return NULL;
   }

   /* update the reg_1 info */
   instr->reg_2 = reg_source1;

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

   /* update the reg_1 info */
   instr->reg_1 = reg_dest;

   reg_source1 = initializeRegister(r_source1, 0);
   if (reg_source1 == NULL)
   {
      finalizeInstruction(instr);
      notifyError(AXE_OUT_OF_MEMORY);
   }

   /* update the reg_2 info */
   instr->reg_2 = reg_source1;

   reg_source2 = initializeRegister(r_source2, ((flags & CG_INDIRECT_SOURCE)? 1 : 0));
   if (reg_source1 == NULL)
   {
      finalizeInstruction(instr);
      notifyError(AXE_OUT_OF_MEMORY);
   }

   /* update the reg_3 info */
   instr->reg_3 = reg_source2;

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
   if (opcode == INVALID_OPCODE)
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
