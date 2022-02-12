/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * gencode.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 * Code generation functions. See also utils.h for genLoadImmediate()
 * and genMoveImmediate().
 */

#ifndef _AXE_GENCODE_H
#define _AXE_GENCODE_H

#include "engine.h"


/*----------------------------------------------------
 *                   UTILITY
 *---------------------------------------------------*/

/* Generate the instruction to load an `immediate' value into a new register.
 * It returns the new register identifier or REG_INVALID if an error occurs */
extern int genLoadImmediate(t_program_infos *program, int immediate);

/* Generate the instruction to move an `immediate' value into a register. */
extern void genMoveImmediate(t_program_infos *program, int dest, int imm);


/*----------------------------------------------------
 *                   ARITHMETIC
 *---------------------------------------------------*/

extern t_axe_instruction *genADDInstruction(t_program_infos *program,
   int r_dest, int r_src1, int r_src2);
extern t_axe_instruction *genSUBInstruction(t_program_infos *program,
   int r_dest, int r_src1, int r_src2);
extern t_axe_instruction *genANDInstruction(t_program_infos *program,
   int r_dest, int r_src1, int r_src2);
extern t_axe_instruction *genORInstruction(t_program_infos *program,
   int r_dest, int r_src1, int r_src2);
extern t_axe_instruction *genXORInstruction(t_program_infos *program,
   int r_dest, int r_src1, int r_src2);
extern t_axe_instruction *genMULInstruction(t_program_infos *program,
   int r_dest, int r_src1, int r_src2);
extern t_axe_instruction *genDIVInstruction(t_program_infos *program,
   int r_dest, int r_src1, int r_src2);
extern t_axe_instruction *genSLLInstruction(t_program_infos *program,
   int r_dest, int r_src1, int r_src2);
extern t_axe_instruction *genSRLInstruction(t_program_infos *program,
   int r_dest, int r_src1, int r_src2);
extern t_axe_instruction *genSRAInstruction(t_program_infos *program,
   int r_dest, int r_src1, int r_src2);

/*----------------------------------------------------
 *                   ARITHMETIC WITH IMMEDIATE
 *---------------------------------------------------*/

/* Used in order to create and assign to the current `program'
 * an ADDI instruction. The semantic of an ADDI instruction
 * is the following: ADDI r_dest, r_source1, immediate. `RDest' is a register
 * location identifier: the result of the ADDI instruction will be
 * stored in that register. Using an RTL (Register Transfer Language)
 * representation we can say that an ADDI instruction of the form: 
 * ADDI R1 R2 #IMM can be represented in the following manner: R1 <-- R2 + IMM.
 * `Rsource1' and `#IMM' are the two operands of the binary numeric
 * operation. `r_dest' is a register location, `immediate' is an immediate
 * value. The content of `r_source1' is added to the value of `immediate'
 * and the result is then stored into the register `RDest'. */
extern t_axe_instruction *genADDIInstruction(
      t_program_infos *program, int r_dest, int r_src1, int immediate);

/* Used in order to create and assign to the current `program'
 * a SUBI instruction. The semantic of an SUBI instruction
 * is the following: SUBI r_dest, r_source1, immediate. `RDest' is a register
 * location identifier: the result of the SUBI instruction will be
 * stored in that register. Using an RTL representation we can say
 * that a SUBI instruction of the form: SUBI R1 R2 #IMM can be represented
 * in the following manner: R1 <-- R2 - IMM.
 * `Rsource1' and `#IMM' are the two operands of the binary numeric
 * operation. `r_dest' is a register location, `immediate' is an immediate
 * value. The content of `r_source1' is subtracted to the value of `immediate'
 * and the result is then stored into the register `RDest'. */
extern t_axe_instruction *genSUBIInstruction(
      t_program_infos *program, int r_dest, int r_src1, int immediate);

/* Used in order to create and assign to the current `program'
 * an ANDBI instruction. An example RTL representation of ANDBI R1 R2 #IMM is:
 * R1 <-- R2 & IMM (bitwise AND).
 * `r_source1' and `immediate' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location, `immediate' is an immediate
 * value. */
extern t_axe_instruction *genANDIInstruction(
      t_program_infos *program, int r_dest, int r_src1, int immediate);

/* Used in order to create and assign to the current `program'
 * an ORBI instruction. An example RTL representation of ORBI R1 R2 #IMM is:
 * R1 <-- R2 | IMM.
 * `r_source1' and `immediate' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location, `immediate' is an immediate
 * value. */
extern t_axe_instruction *genORIInstruction(
      t_program_infos *program, int r_dest, int r_src1, int immediate);

/* Used in order to create and assign to the current `program'
 * a EORBI instruction. An example RTL representation of EORBI R1 R2 #IMM is:
 * R1 <-- R2 ^ IMM.
 * `r_source1' and `immediate' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location, `immediate' is an immediate
 * value. */
extern t_axe_instruction *genXORIInstruction(
      t_program_infos *program, int r_dest, int r_src1, int immediate);

/* Used in order to create and assign to the current `program'
 * a MULI instruction. An example RTL representation of MULI is:
 * R1 <-- R2 * IMM.
 * `r_source1' and `immediate' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location, `immediate' is an immediate
 * value. */
extern t_axe_instruction *genMULIInstruction(
      t_program_infos *program, int r_dest, int r_src1, int immediate);

/* Used in order to create and assign to the current `program'
 * a DIVI instruction. An example RTL representation of DIVI R1 R2 #IMM is:
 * R1 <-- R2 / IMM.
 * `r_source1' and `immediate' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location, `immediate' is an immediate
 * value. */
extern t_axe_instruction *genDIVIInstruction(
      t_program_infos *program, int r_dest, int r_src1, int immediate);

/* Used in order to create and assign to the current `program'
 * a SHLI instruction. An example RTL representation of SHLI R1 R2 #IMM is:
 * R1 <-- R2 / IMM.
 * `r_source1' and `immediate' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location, `immediate' is an immediate
 * value. */
extern t_axe_instruction *genSLLIInstruction(
      t_program_infos *program, int r_dest, int r_src1, int immediate);

extern t_axe_instruction *genSRLIInstruction(
      t_program_infos *program, int r_dest, int r_src1, int immediate);

/* Used in order to create and assign to the current `program'
 * a SHRI instruction. An example RTL representation of SHRI R1 R2 #IMM is:
 * R1 <-- R2 / IMM.
 * `r_source1' and `immediate' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location, `immediate' is an immediate
 * value. */
extern t_axe_instruction *genSRAIInstruction(
      t_program_infos *program, int r_dest, int r_src1, int immediate);


/*----------------------------------------------------
 *                   COMPARISON
 *---------------------------------------------------*/

extern t_axe_instruction *genSEQInstruction(t_program_infos *program,
      int r_dest, int r_src1, int r_src2);
extern t_axe_instruction *genSNEInstruction(t_program_infos *program,
      int r_dest, int r_src1, int r_src2);
extern t_axe_instruction *genSLTInstruction(t_program_infos *program,
      int r_dest, int r_src1, int r_src2);
extern t_axe_instruction *genSLTUInstruction(t_program_infos *program,
      int r_dest, int r_src1, int r_src2);
extern t_axe_instruction *genSGEInstruction(t_program_infos *program,
      int r_dest, int r_src1, int r_src2);
extern t_axe_instruction *genSGEUInstruction(t_program_infos *program,
      int r_dest, int r_src1, int r_src2);
extern t_axe_instruction *genSGTInstruction(t_program_infos *program,
      int r_dest, int r_src1, int r_src2);
extern t_axe_instruction *genSGTUInstruction(t_program_infos *program,
      int r_dest, int r_src1, int r_src2);
extern t_axe_instruction *genSLEInstruction(t_program_infos *program,
      int r_dest, int r_src1, int r_src2);
extern t_axe_instruction *genSLEUInstruction(t_program_infos *program,
      int r_dest, int r_src1, int r_src2);


/*----------------------------------------------------
 *                   COMPARISON WITH IMMEDIATE
 *---------------------------------------------------*/

extern t_axe_instruction *genSEQIInstruction(t_program_infos *program,
      int r_dest, int r_src1, int immediate);
extern t_axe_instruction *genSNEIInstruction(t_program_infos *program,
      int r_dest, int r_src1, int immediate);
extern t_axe_instruction *genSLTIInstruction(t_program_infos *program,
      int r_dest, int r_src1, int immediate);
extern t_axe_instruction *genSLTIUInstruction(t_program_infos *program,
      int r_dest, int r_src1, int immediate);
extern t_axe_instruction *genSGEIInstruction(t_program_infos *program,
      int r_dest, int r_src1, int immediate);
extern t_axe_instruction *genSGEIUInstruction(t_program_infos *program,
      int r_dest, int r_src1, int immediate);
extern t_axe_instruction *genSGTIInstruction(t_program_infos *program,
      int r_dest, int r_src1, int immediate);
extern t_axe_instruction *genSGTIUInstruction(t_program_infos *program,
      int r_dest, int r_src1, int immediate);
extern t_axe_instruction *genSLEIInstruction(t_program_infos *program,
      int r_dest, int r_src1, int immediate);
extern t_axe_instruction *genSLEIUInstruction(t_program_infos *program,
      int r_dest, int r_src1, int immediate);


/*----------------------------------------------------
 *                   JUMP
 *---------------------------------------------------*/

extern t_axe_instruction *genJInstruction(t_program_infos *program,
      t_axe_label *label);

extern t_axe_instruction *genBEQInstruction(t_program_infos *program,
      int rs1, int rs2, t_axe_label *label);
extern t_axe_instruction *genBEQInstruction(t_program_infos *program,
      int rs1, int rs2, t_axe_label *label);
extern t_axe_instruction *genBNEInstruction(t_program_infos *program,
      int rs1, int rs2, t_axe_label *label);  
extern t_axe_instruction *genBLTInstruction(t_program_infos *program,
      int rs1, int rs2, t_axe_label *label);  
extern t_axe_instruction *genBLTUInstruction(t_program_infos *program,
      int rs1, int rs2, t_axe_label *label);  
extern t_axe_instruction *genBGEInstruction(t_program_infos *program,
      int rs1, int rs2, t_axe_label *label);  
extern t_axe_instruction *genBGEUInstruction(t_program_infos *program,
      int rs1, int rs2, t_axe_label *label);  
extern t_axe_instruction *genBGTInstruction(t_program_infos *program,
      int rs1, int rs2, t_axe_label *label);  
extern t_axe_instruction *genBGTUInstruction(t_program_infos *program,
      int rs1, int rs2, t_axe_label *label);  
extern t_axe_instruction *genBLEInstruction(t_program_infos *program,
      int rs1, int rs2, t_axe_label *label);  
extern t_axe_instruction *genBLEUInstruction(t_program_infos *program,
      int rs1, int rs2, t_axe_label *label);  


/*----------------------------------------------------
 *                  LOAD/STORE
 *---------------------------------------------------*/

extern t_axe_instruction *genLIInstruction(
      t_program_infos *program, int r_dest, int immediate);

/* A MOVA instruction copies an address value into a register.
 * An address can be either an instance of `t_axe_label'
 * or a number (numeric address) */
extern t_axe_instruction *genLAInstruction(
      t_program_infos *program, int r_dest, t_axe_label *label);

extern t_axe_instruction *genLWInstruction(
      t_program_infos *program, int r_dest, int immediate, int rs1);
extern t_axe_instruction *genSWInstruction(
      t_program_infos *program, int rs2, int immediate, int rs1);

extern t_axe_instruction *genLWGlobalInstruction(
      t_program_infos *program, int r_dest, t_axe_label *label);
extern t_axe_instruction *genSWGlobalInstruction(
      t_program_infos *program, int rs2, t_axe_label *label);


/*----------------------------------------------------
 *                   OTHER
 *---------------------------------------------------*/

/* By calling this function, a new NOP instruction will be added
 * to `program'. A NOP instruction doesn't make use of
 * any kind of parameter */
extern t_axe_instruction *genNOPInstruction(t_program_infos *program);

extern t_axe_instruction *genECALLInstruction(t_program_infos *program);
extern t_axe_instruction *genEBREAKInstruction(t_program_infos *program);


/*----------------------------------------------------
 *                  SYSTEM CALLS
 *---------------------------------------------------*/

/* By calling this function, a new HALT instruction will be added
 * to `program'. An HALT instruction doesn't require
 * any kind of parameter */
extern t_axe_instruction *genHALTInstruction(t_program_infos *program);

/* A READ instruction requires only one parameter:
 * A destination register (where the value
 * read from standard input will be loaded). */
extern t_axe_instruction *genREADInstruction(
      t_program_infos *program, int r_dest);

/* A WRITE instruction requires only one parameter:
 * A destination register (where the value
 * that will be written to the standard output is located). */
extern t_axe_instruction *genWRITEInstruction(
      t_program_infos *program, int r_src1);


#endif
