/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_gencode.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 * Code generation functions. See also axe_utils.h for genLoadImmediate()
 * and genMoveImmediate().
 */

#ifndef _AXE_GENCODE_H
#define _AXE_GENCODE_H

#include "axe_engine.h"


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

extern t_axe_instruction *genLWInstruction(
      t_program_infos *program, int r_dest, t_axe_label *label, int rs1);
extern t_axe_instruction *genSWInstruction(
      t_program_infos *program, t_axe_label *label, int rs1, int rs2);

extern t_axe_instruction *genLIInstruction(
      t_program_infos *program, int r_dest, int immediate);

/* A MOVA instruction copies an address value into a register.
 * An address can be either an instance of `t_axe_label'
 * or a number (numeric address) */
extern t_axe_instruction *genLAInstruction(
      t_program_infos *program, int r_dest, t_axe_label *label);


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




/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/* TODO: REMOVE FROM HERE */
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/* CODEGEN FLAGS */
#define CG_DIRECT_ALL 0       /*  DEST =  SRC1 <OP>  SRC2  */
#define CG_INDIRECT_ALL 3     /* [DEST] = SRC1 <OP> [SRC2] */
#define CG_INDIRECT_DEST 1    /* [DEST] = SRC1 <OP>  SRC2  */
#define CG_INDIRECT_SOURCE 2  /*  DEST =  SRC1 <OP> [SRC2] */


/*----------------------------------------------------
 *                   UNARY OPERATIONS
 *---------------------------------------------------*/

/* A LOAD instruction requires the following parameters:
 * 1.  A destination register where the requested value will be loaded
 * 2.  A label information (can be a NULL pointer. If so, the addess
 *     value will be taken into consideration)
 * 3.  A direct address (if label is different from NULL) */
extern t_axe_instruction *OLDgenLOADInstruction(
      t_program_infos *program, int r_dest, t_axe_label *label, int address);

/* A STORE instruction copies a value from a register to a
 * specific memory location. The memory location can be
 * either a label identifier or a address reference.
 * In order to create a STORE instruction the caller must
 * provide a valid register location (`r_dest') and an
 * instance of `t_axe_label' or a numeric address */
extern t_axe_instruction *OLDgenSTOREInstruction(
      t_program_infos *program, int r_dest, t_axe_label *label, int address);

/* 
 * STATUS REGISTER TEST INSTRUCTIONS
 */

/* A SGE instruction tests the content of the STATUS REGISTER. To be more
 * specific, a SGE instruction sets to #1 the content of the register
 * `r_dest' if the condition (N.V + ~N.~V) is TRUE; otherwise the content
 * of `r_dest' is set to 0.
 * (I.e.: r_dest will be set to #1 only if the value computed by
 * the last numeric operation returned a value
 * greater or equal to zero). */
extern t_axe_instruction *OLDgenSGEInstruction(
      t_program_infos *program, int r_dest);

/* A SEQ instruction tests the content of the STATUS REGISTER. In particular,
 * a SEQ instruction sets to #1 the content of the register
 * `r_dest' if the condition Z is TRUE; otherwise the content of `r_dest' is set
 * to 0. (I.e.: r_dest will be set to #1 only if the value computed by
 * the last numeric operation returned a value equal to zero). */
extern t_axe_instruction *OLDgenSEQInstruction(
      t_program_infos *program, int r_dest);

/* A SGT instruction tests the content of the STATUS REGISTER. In particular,
 * a SGT instruction sets to #1 the content of the register
 * `r_dest' if the condition (N.V.~Z + ~N.~V.~Z) is TRUE;
 * otherwise the content of `r_dest' is set to 0. (I.e.: r_dest will be
 * set to #1 only if the value computed by the last numeric operation
 * returned a value greater than zero). */
extern t_axe_instruction *OLDgenSGTInstruction(
      t_program_infos *program, int r_dest);

/* A SLE instruction tests the content of the STATUS REGISTER. In particular,
 * a SLE instruction sets to #1 the content of the register
 * `r_dest' if the condition (Z + N.~V + ~N.V) is TRUE;
 * otherwise the content of `r_dest' is set to 0. (I.e.: r_dest will be
 * set to #1 only if the value computed by the last numeric operation
 * returned a value less than zero). */
extern t_axe_instruction *OLDgenSLEInstruction(
      t_program_infos *program, int r_dest);

/* A SLT instruction tests the content of the STATUS REGISTER. In particular,
 * a SLT instruction sets to #1 the content of the register
 * `r_dest' if the condition (N.~V + ~N.V) is TRUE;
 * otherwise the content of `r_dest' is set to 0. (I.e.: r_dest will be
 * set to #1 only if the value computed by the last numeric operation
 * returned a value less than or equal to zero). */
extern t_axe_instruction *OLDgenSLTInstruction(
      t_program_infos *program, int r_dest);

/* A SNE instruction tests the content of the STATUS REGISTER. In particular,
 * a SNE instruction sets to #1 the content of the register
 * `r_dest' if the condition ~N is TRUE;
 * otherwise the content of `r_dest' is set to 0. (I.e.: r_dest will be
 * set to #1 only if the value computed by the last numeric operation
 * returned a value different from zero). */
extern t_axe_instruction *OLDgenSNEInstruction(
      t_program_infos *program, int r_dest);

/*----------------------------------------------------
 *                   BINARY OPERATIONS
 *---------------------------------------------------*/

/* Used in order to create and assign to the current `program'
 * a ADD instruction. An example RTL representation of ADD R1 R2 R3 is:
 * R1 <-- R2 + R3.
 * `r_source1' and `r_source2' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location. `r_dest' and `r_source2'
 * are register locations that can be directly or indirectly addressed. */
extern t_axe_instruction *OLDgenADDInstruction(t_program_infos *program,
      int r_dest, int r_source1, int r_source2, int flags);

/* Used in order to create and assign to the current `program'
 * a SUB instruction. An example RTL representation of SUB R1 R2 R3 is:
 * R1 <-- R2 - R3.
 * `r_source1' and `r_source2' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location. `r_dest' and `r_source2'
 * are register locations that can be directly or indirectly addressed. */
extern t_axe_instruction *OLDgenSUBInstruction(t_program_infos *program,
      int r_dest, int r_source1, int r_source2, int flags);

/* Used in order to create and assign to the current `program'
 * an ANDLI instruction. An example RTL representation of ANDLI R1 R2 #IMM is:
 * R1 <-- R2 && IMM.
 * `r_source1' and `immediate' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location, `immediate' is an immediate
 * value. */
extern t_axe_instruction *OLDgenANDLIInstruction(
      t_program_infos *program, int r_dest, int r_source1, int immediate);

/* Used in order to create and assign to the current `program'
 * a ORLI instruction. An example RTL representation of ORLI R1 R2 #IMM is:
 * R1 <-- R2 || IMM.
 * `r_source1' and `immediate' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location, `immediate' is an immediate
 * value. */
extern t_axe_instruction *OLDgenORLIInstruction(
      t_program_infos *program, int r_dest, int r_source1, int immediate);

/* Used in order to create and assign to the current `program'
 * a EORLI instruction. An example RTL representation of EORLI R1 R2 #IMM is:
 * R1 <-- R2 XOR IMM (Where XOR is the operator: logical exclusive OR).
 * `r_source1' and `immediate' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location, `immediate' is an immediate
 * value. */
extern t_axe_instruction *OLDgenEORLIInstruction(
      t_program_infos *program, int r_dest, int r_source1, int immediate);

/* Used in order to create and assign to the current `program'
 * a ANDB instruction. An example RTL representation of ANDB R1 R2 R3 is:
 * R1 <-- R2 & R3.
 * `r_source1' and `r_source2' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location. `r_dest' and `r_source2'
 * are register locations that can be directly or indirectly addressed. */
extern t_axe_instruction *OLDgenANDInstruction(t_program_infos *program,
      int r_dest, int r_source1, int r_source2, int flags);

/* Used in order to create and assign to the current `program'
 * a ORB instruction. An example RTL representation of ORB R1 R2 R3 is:
 * R1 <-- R2 | R3.
 * `r_source1' and `r_source2' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location. `r_dest' and `r_source2'
 * are register locations that can be directly or indirectly addressed. */
extern t_axe_instruction *OLDgenORInstruction(t_program_infos *program,
      int r_dest, int r_source1, int r_source2, int flags);

/* Used in order to create and assign to the current `program'
 * a EORB instruction. An example RTL representation of EORB R1 R2 R3 is:
 * R1 <-- R2 XORB R3.
 * `r_source1' and `r_source2' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location. `r_dest' and `r_source2'
 * are register locations that can be directly or indirectly addressed. */
extern t_axe_instruction *OLDgenXORInstruction(t_program_infos *program,
      int r_dest, int r_source1, int r_source2, int flags);

/* Used in order to create and assign to the current `program'
 * a MUL instruction. An example RTL representation of MUL R1 R2 R3 is:
 * R1 <-- R2 * R3.
 * `r_source1' and `r_source2' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location. `r_dest' and `r_source2'
 * are register locations that can be directly or indirectly addressed. */
extern t_axe_instruction *OLDgenMULInstruction(t_program_infos *program,
      int r_dest, int r_source1, int r_source2, int flags);

/* Used in order to create and assign to the current `program'
 * a DIV instruction. An example RTL representation of DIV R1 R2 R3 is:
 * R1 <-- R2 / R3.
 * `r_source1' and `r_source2' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location. `r_dest' and `r_source2'
 * are register locations that can be directly or indirectly addressed. */
extern t_axe_instruction *OLDgenDIVInstruction(t_program_infos *program,
      int r_dest, int r_source1, int r_source2, int flags);

/* Used in order to create and assign to the current `program'
 * a SHL instruction. An example RTL representation of SHL R1 R2 R3 is:
 * R1 <-- R2 shifted to left by R3.
 * `r_source1' and `r_source2' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location. `r_dest' and `r_source2'
 * are register locations that can be directly or indirectly addressed. */
extern t_axe_instruction *OLDgenSLLInstruction(t_program_infos *program,
      int r_dest, int r_source1, int r_source2, int flags);

/* Used in order to create and assign to the current `program'
 * a SHR instruction. An example RTL representation of SHR R1 R2 R3 is:
 * R1 <-- R2 shifted to right by R3.
 * `r_source1' and `r_source2' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location. `r_dest' and `r_source2'
 * are register locations that can be directly or indirectly addressed. */
extern t_axe_instruction *OLDgenSRAInstruction(t_program_infos *program,
      int r_dest, int r_source1, int r_source2, int flags);

/* Used in order to create and assign to the current `program'
 * a NOTL instruction. An example RTL representation of NOTL R1 R2 is:
 * R1 <-- !R2. */
extern t_axe_instruction *OLDgenNOTLInstruction(
      t_program_infos *program, int r_dest, int r_source1);

/* Used in order to create and assign to the current `program'
 * a NOTB instruction. An example RTL representation of NOTB R1 R2 is:
 * R1 <-- ~R2. */
extern t_axe_instruction *OLDgenNOTBInstruction(
      t_program_infos *program, int r_dest, int r_source1);

/*----------------------------------------------------
 *                   TERNARY OPERATIONS
 *---------------------------------------------------*/

/* Used in order to create and assign to the current `program'
 * a ANDL instruction. An example RTL representation of ANDL R1 R2 R3 is:
 * R1 <-- R2 && R3.
 * `r_source1' and `r_source2' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location. `r_dest' and `r_source2'
 * are register locations that can be directly or indirectly addressed. */
extern t_axe_instruction *OLDgenANDLInstruction(t_program_infos *program,
      int r_dest, int r_source1, int r_source2, int flags);

/* Used in order to create and assign to the current `program'
 * a ORL instruction. An example RTL representation of ORL R1 R2 R3 is:
 * R1 <-- R2 || R3.
 * `r_source1' and `r_source2' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location. `r_dest' and `r_source2'
 * are register locations that can be directly or indirectly addressed. */
extern t_axe_instruction *OLDgenORLInstruction(t_program_infos *program,
      int r_dest, int r_source1, int r_source2, int flags);

/* Used in order to create and assign to the current `program'
 * a EORL instruction. An example RTL representation of EORL R1 R2 R3 is:
 * R1 <-- R2 XORL R3.
 * `r_source1' and `r_source2' are the two operands of the binary numeric
 * comparison. `r_dest' is a register location. `r_dest' and `r_source2'
 * are register locations that can be directly or indirectly addressed. */
extern t_axe_instruction *OLDgenEORLInstruction(t_program_infos *program,
      int r_dest, int r_source1, int r_source2, int flags);

/* Used in order to create and assign to the current `program'
 * a NEG instruction. An example RTL representation of NEG R1 R2 is:
 * as follows: R1 <-- (-R2).
 * `r_source' is the only operand for this instruction.
 * `r_dest' is a register location. `r_dest' and `r_source'
 * are register locations that can be directly or indirectly addressed. */
extern t_axe_instruction *OLDgenNEGInstruction(
      t_program_infos *program, int r_dest, int r_source, int flags);

/* This instruction is reserved for future implementation. */
extern t_axe_instruction *OLDgenSPCLInstruction(t_program_infos *program,
      int r_dest, int r_source1, int r_source2, int flags);

/*----------------------------------------------------
 *                   JUMP INSTRUCTIONS
 *---------------------------------------------------*/

/* create a branch true instruction. By executing this instruction the control
 * is always passed to either the instruction with the label `label' associated
 * with, or (if `label' is a NULL pointer) to the explicit `address' */
extern t_axe_instruction *OLDgenBTInstruction(
      t_program_infos *program, t_axe_label *label, int addr);

/* create a branch false instruction. By executing this instruction the control
 * is always passed to the next instruction in the program
 * (i.e.: the instruction pointed by PC + 1). */
extern t_axe_instruction *OLDgenBFInstruction(
      t_program_infos *program, t_axe_label *label, int addr);

/* create an unsigned "branch on higher than" instruction. According to the
 * value of the status register, the branch will be taken if the expression
 * ~(C + Z) is TRUE. */
extern t_axe_instruction *OLDgenBHIInstruction(
      t_program_infos *program, t_axe_label *label, int addr);

/* create an unsigned "branch on less than (or equal)" instruction. According
 * to the value of the status register, the branch will be taken if the
 * expression (C + Z) is TRUE. */
extern t_axe_instruction *OLDgenBLSInstruction(
      t_program_infos *program, t_axe_label *label, int addr);

/* create a "branch on carry clear" instruction. If the bit `C' of the
 * status register is not set, then the branch is taken. */
extern t_axe_instruction *OLDgenBCCInstruction(
      t_program_infos *program, t_axe_label *label, int addr);

/* create a "branch on carry set" instruction. If the bit `C' of the
 * status register is set, then the branch is taken. */
extern t_axe_instruction *OLDgenBCSInstruction(
      t_program_infos *program, t_axe_label *label, int addr);

/* create a "branch on not equal" instruction. If the bit `Z' of the
 * status register is not set, then the branch is taken. */
extern t_axe_instruction *OLDgenBNEInstruction(
      t_program_infos *program, t_axe_label *label, int addr);

/* create a "branch on equal" instruction. If the bit `Z' of the
 * status register is set, then the branch is taken. */
extern t_axe_instruction *OLDgenBEQInstruction(
      t_program_infos *program, t_axe_label *label, int addr);

/* create a "branch on overflow clear" instruction. If the bit `V' of the
 * status register is not set, then the branch is taken. */
extern t_axe_instruction *OLDgenBVCInstruction(
      t_program_infos *program, t_axe_label *label, int addr);

/* create a "branch on overflow set" instruction. If the bit `V' of the
 * status register is set, then the branch is taken. */
extern t_axe_instruction *OLDgenBvsInstruction(
      t_program_infos *program, t_axe_label *label, int addr);

/* create a "branch on plus (i.e. positive)" instruction. If the bit `N' of the
 * status register is not set, then the branch is taken. */
extern t_axe_instruction *OLDgenBPLInstruction(
      t_program_infos *program, t_axe_label *label, int addr);

/* create a "branch on minus (i.e. negative)" instruction. If the bit `N' of the
 * status register is set, then the branch is taken. */
extern t_axe_instruction *OLDgenBMIInstruction(
      t_program_infos *program, t_axe_label *label, int addr);

/* create a "branch on greater or equal" instruction. According to the value
 * of the status register, the branch will be taken if the expression
 * (N.V + ~N.~V) is TRUE. */
extern t_axe_instruction *OLDgenBGEInstruction(
      t_program_infos *program, t_axe_label *label, int addr);

/* create a "branch on less than" instruction. According to the value
 * of the status register, the branch will be taken if the expression
 * (N.~V + ~N.V) is TRUE. */
extern t_axe_instruction *OLDgenBLTInstruction(
      t_program_infos *program, t_axe_label *label, int addr);

/* create a "branch on greater than" instruction. According to the value
 * of the status register, the branch will be taken if the expression
 * (N.V.~Z + ~N.~V.~Z) is TRUE. */
extern t_axe_instruction *OLDgenBGTInstruction(
      t_program_infos *program, t_axe_label *label, int addr);

/* create a "branch on less than or equal" instruction. According to the value
 * of the status register, the branch will be taken if the expression
 * (Z + N.~V + ~N.V) is TRUE. */
extern t_axe_instruction *OLDgenBLEInstruction(
      t_program_infos *program, t_axe_label *label, int addr);


#endif
