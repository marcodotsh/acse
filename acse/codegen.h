/// @file codegen.h
/// @brief Code generation functions.

#ifndef CODEGEN_H
#define CODEGEN_H

#include "program.h"

/**
 * @defgroup gencode Code Generation Functions
 * @brief Functions for adding instructions to a program.
 *
 * In ACSE, the semantic actions in the parser directly translate the
 * source code into intermediate assembly by appending new instructions inside
 * the 'program' structure.
 * The functions defined here are helpers that add a specific instruction code
 * with given parameters to the end of the program.
 * @{
 */


/// @name Register-Register Arithmetic Instructions
/// @{

/** Add a new ADD instruction at the end of the instruction list of the
 *  specified program. At runtime, an ADD instruction sums the values in the
 *  two source registers, and places the result in the destination register.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the first source register (first addend)
 *  @param rs2 Identifier of the second source register (second addend)
 *  @returns the instruction object added to the instruction list */
t_instruction *genADD(t_program *program, t_regID rd, t_regID rs1, t_regID rs2);

/** Add a new SUB instruction at the end of the instruction list of the
 *  specified program. At runtime, a SUB instruction subtracts the value in the
 *  second source register from the value in the first source register,
 *  and places the result in the destination register.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the first source register (minuend)
 *  @param rs2 Identifier of the second source register (subtrahend)
 *  @returns the instruction object added to the instruction list */
t_instruction *genSUB(t_program *program, t_regID rd, t_regID rs1, t_regID rs2);

/** Add a new AND instruction at the end of the instruction list of the
 *  specified program. At runtime, an AND instruction computes the bitwise
 *  AND (&) of the values in the source registers, and places the result in the
 *  destination register.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the first source register
 *  @param rs2 Identifier of the second source register
 *  @returns the instruction object added to the instruction list */
t_instruction *genAND(t_program *program, t_regID rd, t_regID rs1, t_regID rs2);

/** Add a new OR instruction at the end of the instruction list of the
 *  specified program. At runtime, an OR instruction computes the bitwise
 *  OR (|) of the values in the source registers, and places the result in the
 *  destination register.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the first source register
 *  @param rs2 Identifier of the second source register
 *  @returns the instruction object added to the instruction list */
t_instruction *genOR(t_program *program, t_regID rd, t_regID rs1, t_regID rs2);

/** Add a new XOR instruction at the end of the instruction list of the
 *  specified program. At runtime, a XOR instruction computes the bitwise
 *  XOR (^) of the values in the source registers, and places the result in the
 *  destination register.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the first source register
 *  @param rs2 Identifier of the second source register
 *  @returns the instruction object added to the instruction list */
t_instruction *genXOR(t_program *program, t_regID rd, t_regID rs1, t_regID rs2);

/** Add a new MUL instruction at the end of the instruction list of the
 *  specified program. At runtime, a MUL instruction multiplies the values in
 *  the source registers, and places the result in the destination register.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the first source register (first factor)
 *  @param rs2 Identifier of the second source register (second factor)
 *  @returns the instruction object added to the instruction list */
t_instruction *genMUL(t_program *program, t_regID rd, t_regID rs1, t_regID rs2);

/** Add a new DIV instruction at the end of the instruction list of the
 *  specified program. At runtime, a DIV instruction divides the value in
 *  the first source register with the value in the second source register,
 *  and places the result in the destination register. All the registers are
 *  assumed to contain signed integers.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the first source register (dividend)
 *  @param rs2 Identifier of the second source register (divisor)
 *  @returns the instruction object added to the instruction list */
t_instruction *genDIV(t_program *program, t_regID rd, t_regID rs1, t_regID rs2);

/** Add a new REM instruction at the end of the instruction list of the
 *  specified program. At runtime, a REM instruction places the remainder of the
 *  division between the first and second source registers in the destination
 *  register. All the registers are assumed to contain signed integers.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the first source register (dividend)
 *  @param rs2 Identifier of the second source register (divisor)
 *  @returns the instruction object added to the instruction list */
t_instruction *genREM(t_program *program, t_regID rd, t_regID rs1, t_regID rs2);

/** Add a new SLL instruction at the end of the instruction list of the
 *  specified program. At runtime, a SLL instruction shifts the binary value
 *  in the first source register to the left by the number of places
 *  specified by the value of the second source register.
 *    The shift amount is modulo 32; in other words, only the 5 least
 *  significant bits of the second source register are considered.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the first source register (value to shift)
 *  @param rs2 Identifier of the second source register (shift amount)
 *  @returns the instruction object added to the instruction list */
t_instruction *genSLL(t_program *program, t_regID rd, t_regID rs1, t_regID rs2);

/** Add a new SRL instruction at the end of the instruction list of the
 *  specified program. At runtime, a SRL instruction shifts the binary value
 *  in the first source register to the right by the number of places
 *  specified by the value of the second source register.
 *    The shift amount is modulo 32; in other words, only the 5 least
 *  significant bits of the second source register are considered. Additionally,
 *  the first source register is assumed to contain an unsigned integer.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the first source register (value to shift)
 *  @param rs2 Identifier of the second source register (shift amount)
 *  @returns the instruction object added to the instruction list */
t_instruction *genSRL(t_program *program, t_regID rd, t_regID rs1, t_regID rs2);

/** Add a new SRA instruction at the end of the instruction list of the
 *  specified program. At runtime, a SRA instruction shifts the binary value
 *  in the first source register to the right by the number of places
 *  specified by the value of the second source register.
 *    The shift amount is modulo 32; in other words, only the 5 least
 *  significant bits of the second source register are considered. Additionally,
 *  the first source register is assumed to contain a signed integer.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the first source register (value to shift)
 *  @param rs2 Identifier of the second source register (shift amount)
 *  @returns the instruction object added to the instruction list */
t_instruction *genSRA(t_program *program, t_regID rd, t_regID rs1, t_regID rs2);

/// @}


/// @name Register-Constant Arithmetic Instructions
/// @{

/** Add a new ADDI instruction at the end of the instruction list of the
 *  specified program. At runtime, an ADDI instruction sums the values in the
 *  source register with a constant, and places the result in the destination
 *  register.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the source register (first addend)
 *  @param immediate The constant operand (second addend)
 *  @returns the instruction object added to the instruction list */
t_instruction *genADDI(
    t_program *program, t_regID rd, t_regID rs1, int immediate);

/** Add a new SUBI instruction at the end of the instruction list of the
 *  specified program. At runtime, a SUBI instruction subtracts a constant
 *  from the value in the source register, and places the result in the
 *  destination register.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the source register (minuend)
 *  @param immediate The constant operand (subtrahend)
 *  @returns the instruction object added to the instruction list */
t_instruction *genSUBI(
    t_program *program, t_regID rd, t_regID rs1, int immediate);

/** Add a new ANDI instruction at the end of the instruction list of the
 *  specified program. At runtime, an ANDI instruction computes the bitwise
 *  AND (&) of the value in the source register and a constant, and places the
 *  result in the destination register.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the source register
 *  @param immediate The constant operand
 *  @returns the instruction object added to the instruction list */
t_instruction *genANDI(
    t_program *program, t_regID rd, t_regID rs1, int immediate);

/** Add a new ORI instruction at the end of the instruction list of the
 *  specified program. At runtime, an ORI instruction computes the bitwise
 *  OR (|) of the value in the source register and a constant, and places the
 *  result in the destination register.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the source register
 *  @param immediate The constant operand
 *  @returns the instruction object added to the instruction list */
t_instruction *genORI(
    t_program *program, t_regID rd, t_regID rs1, int immediate);

/** Add a new XORI instruction at the end of the instruction list of the
 *  specified program. At runtime, a XORI instruction computes the bitwise
 *  XOR (^) of the value in the source register and a constant, and places the
 *  result in the destination register.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the source register
 *  @param immediate The constant operand
 *  @returns the instruction object added to the instruction list */
t_instruction *genXORI(
    t_program *program, t_regID rd, t_regID rs1, int immediate);

/** Add a new MULI instruction at the end of the instruction list of the
 *  specified program. At runtime, a MULI instruction multiplies the value in
 *  the source register with a constant, and places the result in the
 *  destination register.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the source register (first factor)
 *  @param immediate The constant operand (second factor)
 *  @returns the instruction object added to the instruction list */
t_instruction *genMULI(
    t_program *program, t_regID rd, t_regID rs1, int immediate);

/** Add a new DIVI instruction at the end of the instruction list of the
 *  specified program. At runtime, a DIVI instruction divides the value in
 *  the first source register with a signed integer constant, and places the
 *  result in the destination register. The source register is assumed to
 *  contain a signed integer.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the source register (dividend)
 *  @param immediate The constant operand (divisor)
 *  @returns the instruction object added to the instruction list */
t_instruction *genDIVI(
    t_program *program, t_regID rd, t_regID rs1, int immediate);

/** Add a new REMI instruction at the end of the instruction list of the
 *  specified program. At runtime, a REM instruction places the remainder of the
 *  division between the first source register and an immediate in the
 *  destination register. All the registers are assumed to contain signed
 *  integers.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the first source register (dividend)
 *  @param immediate The constant operand (divisor)
 *  @returns the instruction object added to the instruction list */
t_instruction *genREMI(
    t_program *program, t_regID rd, t_regID rs1, int immediate);

/** Add a new SLLI instruction at the end of the instruction list of the
 *  specified program. At runtime, a SLLI instruction shifts the binary value
 *  in the source register to the left by the number of places specified by a
 *  constant amount.
 *    The shift amount is modulo 32; in other words, only the 5 least
 *  significant bits of the constant are considered.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the source register (value to shift)
 *  @param immediate The constant operand (shift amount)
 *  @returns the instruction object added to the instruction list */
t_instruction *genSLLI(
    t_program *program, t_regID rd, t_regID rs1, int immediate);

/** Add a new SRLI instruction at the end of the instruction list of the
 *  specified program. At runtime, a SRLI instruction shifts the binary value
 *  in the source register to the right by the number of places specified by a
 *  constant amount.
 *    The shift amount is modulo 32; in other words, only the 5 least
 *  significant bits of the constant are considered. Additionally, the source
 *  register is assumed to contain an unsigned integer.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the source register (value to shift)
 *  @param immediate The constant operand (shift amount)
 *  @returns the instruction object added to the instruction list */
t_instruction *genSRLI(
    t_program *program, t_regID rd, t_regID rs1, int immediate);

/** Add a new SRAI instruction at the end of the instruction list of the
 *  specified program. At runtime, a SRAI instruction shifts the binary value
 *  in the source register to the right by the number of places specified
 *  by a constant amount.
 *    The shift amount is modulo 32; in other words, only the 5 least
 *  significant bits of the constant are considered. Additionally, the source
 *  register is assumed to contain a signed integer.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the source register (value to shift)
 *  @param immediate The constant operand (shift amount)
 *  @returns the instruction object added to the instruction list */
t_instruction *genSRAI(
    t_program *program, t_regID rd, t_regID rs1, int immediate);

/// @}


/// @name Register-Register Comparison Instructions
/// @{

/** Add a new SEQ instruction at the end of the instruction list of the
 *  specified program. At runtime, a SEQ instruction sets the destination
 *  register to 1 if the values in the source registers are equal
 *  (src1 == src2).
 *  Otherwise, the destination register is set to zero.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the first source register
 *  @param rs2 Identifier of the second source register
 *  @returns the instruction object added to the instruction list */
t_instruction *genSEQ(t_program *program, t_regID rd, t_regID rs1, t_regID rs2);

/** Add a new SNE instruction at the end of the instruction list of the
 *  specified program. At runtime, a SNE instruction sets the destination
 *  register to 1 if the values in the source registers are different
 *  (src1 != src2).
 *  Otherwise, the destination register is set to zero.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the first source register
 *  @param rs2 Identifier of the second source register
 *  @returns the instruction object added to the instruction list */
t_instruction *genSNE(t_program *program, t_regID rd, t_regID rs1, t_regID rs2);

/** Add a new SLT instruction at the end of the instruction list of the
 *  specified program. At runtime, a SLT instruction sets the destination
 *  register to 1 if the value in the first source register is less than the
 *  value in the second source register (src1 < src2).
 *  Otherwise, the destination register is set to zero.
 *    The source registers are assumed to contain signed integers.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the first source register
 *  @param rs2 Identifier of the second source register
 *  @returns the instruction object added to the instruction list */
t_instruction *genSLT(t_program *program, t_regID rd, t_regID rs1, t_regID rs2);

/** Add a new SLTU instruction at the end of the instruction list of the
 *  specified program. At runtime, a SLTU instruction sets the destination
 *  register to 1 if the value in the first source register is less than the
 *  value in the second source register (src1 < src2).
 *  Otherwise, the destination register is set to zero.
 *    The source registers are assumed to contain unsigned integers.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the first source register
 *  @param rs2 Identifier of the second source register
 *  @returns the instruction object added to the instruction list */
t_instruction *genSLTU(
    t_program *program, t_regID rd, t_regID rs1, t_regID rs2);

/** Add a new SGE instruction at the end of the instruction list of the
 *  specified program. At runtime, a SGE instruction sets the destination
 *  register to 1 if the value in the first source register is greater or equal
 *  than the value in the second source register (src1 >= src2).
 *  Otherwise, the destination register is set to zero.
 *    The source registers are assumed to contain signed integers.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the first source register
 *  @param rs2 Identifier of the second source register
 *  @returns the instruction object added to the instruction list */
t_instruction *genSGE(t_program *program, t_regID rd, t_regID rs1, t_regID rs2);

/** Add a new SGEU instruction at the end of the instruction list of the
 *  specified program. At runtime, a SGEU instruction sets the destination
 *  register to 1 if the value in the first source register is greater or equal
 *  than the value in the second source register (src1 >= src2).
 *  Otherwise, the destination register is set to zero.
 *    The source registers are assumed to contain unsigned integers.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the first source register
 *  @param rs2 Identifier of the second source register
 *  @returns the instruction object added to the instruction list */
t_instruction *genSGEU(
    t_program *program, t_regID rd, t_regID rs1, t_regID rs2);

/** Add a new SGT instruction at the end of the instruction list of the
 *  specified program. At runtime, a SGT instruction sets the destination
 *  register to 1 if the value in the first source register is greater
 *  than the value in the second source register (src1 > src2).
 *  Otherwise, the destination register is set to zero.
 *    The source registers are assumed to contain signed integers.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the first source register
 *  @param rs2 Identifier of the second source register
 *  @returns the instruction object added to the instruction list */
t_instruction *genSGT(t_program *program, t_regID rd, t_regID rs1, t_regID rs2);

/** Add a new SGTU instruction at the end of the instruction list of the
 *  specified program. At runtime, a SGTU instruction sets the destination
 *  register to 1 if the value in the first source register is greater
 *  than the value in the second source register (src1 > src2).
 *  Otherwise, the destination register is set to zero.
 *    The source registers are assumed to contain unsigned integers.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the first source register
 *  @param rs2 Identifier of the second source register
 *  @returns the instruction object added to the instruction list */
t_instruction *genSGTU(
    t_program *program, t_regID rd, t_regID rs1, t_regID rs2);

/** Add a new SLE instruction at the end of the instruction list of the
 *  specified program. At runtime, a SLE instruction sets the destination
 *  register to 1 if the value in the first source register is lesser or equal
 *  than the value in the second source register (src1 <= src2).
 *  Otherwise, the destination register is set to zero.
 *    The source registers are assumed to contain signed integers.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the first source register
 *  @param rs2 Identifier of the second source register
 *  @returns the instruction object added to the instruction list */
t_instruction *genSLE(t_program *program, t_regID rd, t_regID rs1, t_regID rs2);

/** Add a new SLEU instruction at the end of the instruction list of the
 *  specified program. At runtime, a SLEU instruction sets the destination
 *  register to 1 if the value in the first source register is lesser or equal
 *  than the value in the second source register (src1 <= src2).
 *  Otherwise, the destination register is set to zero.
 *    The source registers are assumed to contain unsigned integers.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the first source register
 *  @param rs2 Identifier of the second source register
 *  @returns the instruction object added to the instruction list */
t_instruction *genSLEU(
    t_program *program, t_regID rd, t_regID rs1, t_regID rs2);

/// @}


/// @name Register-Constant Comparison Instructions
/// @{

/** Add a new SEQI instruction at the end of the instruction list of the
 *  specified program. At runtime, an SEQI instruction sets the destination
 *  register to 1 if the value in the source register is equal to a constant
 *  (src1 == immediate).
 *  Otherwise, the destination register is set to zero.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the source register
 *  @param immediate The constant operand
 *  @returns the instruction object added to the instruction list */
t_instruction *genSEQI(
    t_program *program, t_regID rd, t_regID rs1, int immediate);

/** Add a new SNEI instruction at the end of the instruction list of the
 *  specified program. At runtime, an SNEI instruction sets the destination
 *  register to 1 if the value in the source register is different from a
 *  constant (src1 != immediate).
 *  Otherwise, the destination register is set to zero.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the source register
 *  @param immediate The constant operand
 *  @returns the instruction object added to the instruction list */
t_instruction *genSNEI(
    t_program *program, t_regID rd, t_regID rs1, int immediate);

/** Add a new SLTI instruction at the end of the instruction list of the
 *  specified program. At runtime, an SLTI instruction sets the destination
 *  register to 1 if the value in the source register is less than a constant
 *  (src1 < immediate).
 *  Otherwise, the destination register is set to zero.
 *    The source register and the constant are assumed to be signed.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the source register
 *  @param immediate The constant operand
 *  @returns the instruction object added to the instruction list */
t_instruction *genSLTI(
    t_program *program, t_regID rd, t_regID rs1, int immediate);

/** Add a new SLTIU instruction at the end of the instruction list of the
 *  specified program. At runtime, an SLTIU instruction sets the destination
 *  register to 1 if the value in the source register is less than a constant
 *  (src1 < immediate).
 *  Otherwise, the destination register is set to zero.
 *    The source register and the constant are assumed to be unsigned.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the source register
 *  @param immediate The constant operand
 *  @returns the instruction object added to the instruction list */
t_instruction *genSLTIU(
    t_program *program, t_regID rd, t_regID rs1, int immediate);

/** Add a new SGEI instruction at the end of the instruction list of the
 *  specified program. At runtime, an SGEI instruction sets the destination
 *  register to 1 if the value in the source register is greater or equal than
 *  a constant (src1 >= immediate).
 *  Otherwise, the destination register is set to zero.
 *    The source register and the constant are assumed to be signed.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the source register
 *  @param immediate The constant operand
 *  @returns the instruction object added to the instruction list */
t_instruction *genSGEI(
    t_program *program, t_regID rd, t_regID rs1, int immediate);

/** Add a new SGEIU instruction at the end of the instruction list of the
 *  specified program. At runtime, an SGEIU instruction sets the destination
 *  register to 1 if the value in the source register is greater or equal than
 *  a constant (src1 >= immediate).
 *  Otherwise, the destination register is set to zero.
 *    The source register and the constant are assumed to be unsigned.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the source register
 *  @param immediate The constant operand
 *  @returns the instruction object added to the instruction list */
t_instruction *genSGEIU(
    t_program *program, t_regID rd, t_regID rs1, int immediate);

/** Add a new SGTI instruction at the end of the instruction list of the
 *  specified program. At runtime, an SGTI instruction sets the destination
 *  register to 1 if the value in the source register is greater than a constant
 *  (src1 > immediate).
 *  Otherwise, the destination register is set to zero.
 *    The source register and the constant are assumed to be signed.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the source register
 *  @param immediate The constant operand
 *  @returns the instruction object added to the instruction list */
t_instruction *genSGTI(
    t_program *program, t_regID rd, t_regID rs1, int immediate);

/** Add a new SGTIU instruction at the end of the instruction list of the
 *  specified program. At runtime, an SGTIU instruction sets the destination
 *  register to 1 if the value in the source register is greater than a constant
 *  (src1 >= immediate).
 *  Otherwise, the destination register is set to zero.
 *    The source register and the constant are assumed to be unsigned.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the source register
 *  @param immediate The constant operand
 *  @returns the instruction object added to the instruction list */
t_instruction *genSGTIU(
    t_program *program, t_regID rd, t_regID rs1, int immediate);

/** Add a new SLEI instruction at the end of the instruction list of the
 *  specified program. At runtime, an SLEI instruction sets the destination
 *  register to 1 if the value in the source register is lesser or equal than a
 *  constant (src1 <= immediate).
 *  Otherwise, the destination register is set to zero.
 *    The source register and the constant are assumed to be signed.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the source register
 *  @param immediate The constant operand
 *  @returns the instruction object added to the instruction list */
t_instruction *genSLEI(
    t_program *program, t_regID rd, t_regID rs1, int immediate);

/** Add a new SLEIU instruction at the end of the instruction list of the
 *  specified program. At runtime, an SLEIU instruction sets the destination
 *  register to 1 if the value in the source register is lesser or equal than a
 *  constant (src1 <= immediate).
 *  Otherwise, the destination register is set to zero.
 *    The source register and the constant are assumed to be unsigned.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param rs1 Identifier of the source register
 *  @param immediate The constant operand
 *  @returns the instruction object added to the instruction list */
t_instruction *genSLEIU(
    t_program *program, t_regID rd, t_regID rs1, int immediate);

/// @}


/// @name Jump and Branch Instructions
/// @{

/** Add a new J instruction at the end of the instruction list of the
 *  specified program. At runtime, a J instruction unconditionally transfers the
 *  control flow to the instruction identified by a given label
 *  (in brief, jumps or branches to the label).
 *  @param program The program where the instruction will be added
 *  @param label The label where to jump at runtime
 *  @returns the instruction object added to the instruction list */
t_instruction *genJ(t_program *program, t_label *label);

/** Add a new BEQ instruction at the end of the instruction list of the
 *  specified program. At runtime, a BEQ instruction branches to the given label
 *  if and only if the two source registers are equal.
 *  @param program The program where the instruction will be added
 *  @param rs1 Identifier of the first source register
 *  @param rs2 Identifier of the second source register
 *  @param label The label where to jump at runtime
 *  @returns the instruction object added to the instruction list */
t_instruction *genBEQ(
    t_program *program, t_regID rs1, t_regID rs2, t_label *label);

/** Add a new BNE instruction at the end of the instruction list of the
 *  specified program. At runtime, a BNE instruction branches to the given label
 *  if and only if the two source registers are not equal.
 *  @param program The program where the instruction will be added
 *  @param rs1 Identifier of the first source register
 *  @param rs2 Identifier of the second source register
 *  @param label The label where to jump at runtime
 *  @returns the instruction object added to the instruction list */
t_instruction *genBNE(
    t_program *program, t_regID rs1, t_regID rs2, t_label *label);

/** Add a new BLT instruction at the end of the instruction list of the
 *  specified program. At runtime, a BLT instruction branches to the given label
 *  if and only if the value in the first source register is less than the
 *  value in the second source register (src1 < src2).
 *    The source registers are assumed to contain signed integers.
 *  @param program The program where the instruction will be added
 *  @param rs1 Identifier of the first source register
 *  @param rs2 Identifier of the second source register
 *  @param label The label where to jump at runtime
 *  @returns the instruction object added to the instruction list */
t_instruction *genBLT(
    t_program *program, t_regID rs1, t_regID rs2, t_label *label);

/** Add a new BLTU instruction at the end of the instruction list of the
 *  specified program. At runtime, a BLTU instruction branches to the given
 *  label if and only if the value in the first source register is less than the
 *  value in the second source register (src1 < src2).
 *    The source registers are assumed to contain unsigned integers.
 *  @param program The program where the instruction will be added
 *  @param rs1 Identifier of the first source register
 *  @param rs2 Identifier of the second source register
 *  @param label The label where to jump at runtime
 *  @returns the instruction object added to the instruction list */
t_instruction *genBLTU(
    t_program *program, t_regID rs1, t_regID rs2, t_label *label);

/** Add a new BGE instruction at the end of the instruction list of the
 *  specified program. At runtime, a BGE instruction branches to the given
 *  label if and only if the value in the first source register is greater or
 *  equal than the value in the second source register (src1 >= src2).
 *    The source registers are assumed to contain signed integers.
 *  @param program The program where the instruction will be added
 *  @param rs1 Identifier of the first source register
 *  @param rs2 Identifier of the second source register
 *  @param label The label where to jump at runtime
 *  @returns the instruction object added to the instruction list */
t_instruction *genBGE(
    t_program *program, t_regID rs1, t_regID rs2, t_label *label);

/** Add a new BGEU instruction at the end of the instruction list of the
 *  specified program. At runtime, a BGEU instruction branches to the given
 *  label if and only if the value in the first source register is greater or
 *  equal than the value in the second source register (src1 >= src2).
 *    The source registers are assumed to contain unsigned integers.
 *  @param program The program where the instruction will be added
 *  @param rs1 Identifier of the first source register
 *  @param rs2 Identifier of the second source register
 *  @param label The label where to jump at runtime
 *  @returns the instruction object added to the instruction list */
t_instruction *genBGEU(
    t_program *program, t_regID rs1, t_regID rs2, t_label *label);

/** Add a new BGT instruction at the end of the instruction list of the
 *  specified program. At runtime, a BGT instruction branches to the given
 *  label if and only if the value in the first source register is greater
 *  than the value in the second source register (src1 > src2).
 *    The source registers are assumed to contain signed integers.
 *  @param program The program where the instruction will be added
 *  @param rs1 Identifier of the first source register
 *  @param rs2 Identifier of the second source register
 *  @param label The label where to jump at runtime
 *  @returns the instruction object added to the instruction list */
t_instruction *genBGT(
    t_program *program, t_regID rs1, t_regID rs2, t_label *label);

/** Add a new BGTU instruction at the end of the instruction list of the
 *  specified program. At runtime, a BGTU instruction branches to the given
 *  label if and only if the value in the first source register is greater
 *  than the value in the second source register (src1 > src2).
 *    The source registers are assumed to contain unsigned integers.
 *  @param program The program where the instruction will be added
 *  @param rs1 Identifier of the first source register
 *  @param rs2 Identifier of the second source register
 *  @param label The label where to jump at runtime
 *  @returns the instruction object added to the instruction list */
t_instruction *genBGTU(
    t_program *program, t_regID rs1, t_regID rs2, t_label *label);

/** Add a new BLE instruction at the end of the instruction list of the
 *  specified program. At runtime, a BLE instruction branches to the given
 *  label if and only if the value in the first source register is lesser or
 *  equal than the value in the second source register (src1 <= src2).
 *    The source registers are assumed to contain signed integers.
 *  @param program The program where the instruction will be added
 *  @param rs1 Identifier of the first source register
 *  @param rs2 Identifier of the second source register
 *  @param label The label where to jump at runtime
 *  @returns the instruction object added to the instruction list */
t_instruction *genBLE(
    t_program *program, t_regID rs1, t_regID rs2, t_label *label);

/** Add a new BLE instruction at the end of the instruction list of the
 *  specified program. At runtime, a BLE instruction branches to the given
 *  label if and only if the value in the first source register is lesser or
 *  equal than the value in the second source register (src1 <= src2).
 *    The source registers are assumed to contain unsigned integers.
 *  @param program The program where the instruction will be added
 *  @param rs1 Identifier of the first source register
 *  @param rs2 Identifier of the second source register
 *  @param label The label where to jump at runtime
 *  @returns the instruction object added to the instruction list */
t_instruction *genBLEU(
    t_program *program, t_regID rs1, t_regID rs2, t_label *label);

/// @}


/// @name Memory Load/Store Instructions
/// @{

/** Add a new LI instruction at the end of the instruction list of the
 *  specified program. At runtime, a LI instruction loads a specified constant
 *  in the destination register.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param immediate The constant to load
 *  @returns the instruction object added to the instruction list */
t_instruction *genLI(t_program *program, t_regID rd, int immediate);

/** Add a new LA instruction at the end of the instruction list of the
 *  specified program. At runtime, a LA instruction loads the address specified
 *  by a label into the destination register
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param label The label whose address needs to be loaded
 *  @returns the instruction object added to the instruction list */
t_instruction *genLA(t_program *program, t_regID rd, t_label *label);

/** Add a new LW instruction at the end of the instruction list of the
 *  specified program. At runtime, a LW instruction loads a 32-bit word stored
 *  in memory at the address specified by the source register plus an offset
 *  given by a constant (thus the address is rs1 + immediate).
 *  The loaded value is put into the destination register.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param immediate The constant operand (address offset)
 *  @param rs1 Identifier of the source register (base address)
 *  @returns the instruction object added to the instruction list */
t_instruction *genLW(
    t_program *program, t_regID rd, int immediate, t_regID rs1);

/** Add a new LW instruction at the end of the instruction list of the
 *  specified program. At runtime, a LW instruction stores the 32-bit word
 *  contained in the second source register to memory.
 *  The destination address is specified by the first source register, plus an
 *  offset given by a constant (thus the address is rs1 + immediate).
 *  @param program The program where the instruction will be added
 *  @param rs2 Identifier of the second source register (value to store)
 *  @param immediate The constant operand (address offset)
 *  @param rs1 Identifier of the first source register (base address)
 *  @returns the instruction object added to the instruction list
 *  @note The order of the registers in the function signature is inverted with
 *        respect to the actual encoding, just like in the assembly listing
 *        format as described by the RISC-V specification. */
t_instruction *genSW(
    t_program *program, t_regID rs2, int immediate, t_regID rs1);

/** Add a new "global LW" instruction at the end of the instruction list of the
 *  specified program. At runtime, a "global LW" instruction loads a 32-bit
 *  word at the address specified by a label to the destination register.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @param label The label (points to a 32-bit word)
 *  @returns the instruction object added to the instruction list */
t_instruction *genLWGlobal(t_program *program, t_regID rd, t_label *label);

/** Add a new "global SW" instruction at the end of the instruction list of the
 *  specified program. At runtime, a "global SW" instruction stores the 32-bit
 *  word contained in the first source register to the address specified by a
 *  label.
 *  @param program The program where the instruction will be added
 *  @param rs1 Identifier of the source register (value to store)
 *  @param label The label (points to a 32-bit word)
 *  @param rtemp Identifier of an otherwise unused register whose value will be
 *               modified by the instruction during its operation.
 *  @returns the instruction object added to the instruction list
 *  @note At assembly time, the "global SW" instruction translates to a sequence
 *        of multiple instructions which use rtemp to store the address of the
 *        label. This is the reason why rtemp is needed. */
t_instruction *genSWGlobal(
    t_program *program, t_regID rs1, t_label *label, t_regID rtemp);

/// @}


/// @name Variable/array accesses
/// @{

/** Generate instructions that load the content of a scalar variable into a
 * register.
 * @param program The program where the variable belongs.
 * @param var     The symbol object that refers to the variable.
 * @returns The identifier of the register that (at runtime) will contain the
 *          value of the variable loaded from memory. */
t_regID genLoadVariable(t_program *program, t_symbol *var);

/** Generate instructions that store the content of a register into a variable.
 * @param program The program where the variable belongs.
 * @param var     The symbol object that refers to the variable.
 * @param reg     The register whose value needs to be assigned. */
void genStoreRegisterToVariable(t_program *program, t_symbol *var, t_regID reg);

/** Generate instructions that store a constant into a variable.
 * @param program The program where the variable belongs.
 * @param var     The symbol object that refers to the variable.
 * @param val     The constant value which needs to be assigned. */
void genStoreConstantToVariable(t_program *program, t_symbol *var, int val);

/** Generate instructions that load the content of an array element into a
 * register.
 * @param program The program where the array belongs.
 * @param array   The symbol object that refers to the array.
 * @param rIdx    The identifier of the register that will contain the index
 *                into the array.
 * @returns The identifier of the register that (at runtime) will contain the
 *          value of the array element loaded from memory. */
t_regID genLoadArrayElement(t_program *program, t_symbol *array, t_regID rIdx);

/** Generate instructions that store the content of a register into an array
 * element.
 * @param program The program where the array belongs.
 * @param array   The symbol object that refers to the array.
 * @param rIdx    The identifier of the register that will contain the index
 *                into the array.
 * @param rData   The identifier of the register that will contain the value
 *                to be stored. */
void genStoreRegisterToArrayElement(
    t_program *program, t_symbol *array, t_regID rIdx, t_regID rVal);

/** Generate instructions that store the content of a register into an array
 * element.
 * @param program The program where the array belongs.
 * @param array   The symbol object that refers to the array.
 * @param rIdx    The identifier of the register that will contain the index
 *                into the array.
 * @param val     The value to be stored. */
void genStoreConstantToArrayElement(
    t_program *program, t_symbol *array, t_regID rIdx, int val);

/// @}


/// @name System Calls
/// @{

/** Add a new Exit0 syscall instruction at the end of the instruction list of
 *  the specified program. At runtime, this instruction terminates the program
 *  with an exit code of zero.
 *  @param program The program where the instruction will be added
 *  @returns the instruction object added to the instruction list
 *  @note During the target-specific transformation passes, ACSE replaces
 *        syscall instructions with a sequence of lower-level instructions that
 *        use ECALL to transfer control to the supervisor/operating system. */
t_instruction *genExit0Syscall(t_program *program);

/** Add a new ReadInt syscall instruction at the end of the instruction list of
 *  the specified program. At runtime, this instruction reads an integer from
 *  standard input, storing that integer in the given destination register.
 *  @param program The program where the instruction will be added
 *  @param rd Identifier of the destination register
 *  @returns the instruction object added to the instruction list
 *  @note During the target-specific transformation passes, ACSE replaces
 *        syscall instructions with a sequence of lower-level instructions that
 *        use ECALL to transfer control to the supervisor/operating system. */
t_instruction *genReadIntSyscall(t_program *program, t_regID rd);

/** Add a new PrintInt syscall instruction at the end of the instruction list of
 *  the specified program. At runtime, this instruction writes the integer
 *  currently stored in the source register to the standard output stream.
 *  @param program The program where the instruction will be added
 *  @param rs1 Identifier of the source register (integer to print)
 *  @returns the instruction object added to the instruction list
 *  @note During the target-specific transformation passes, ACSE replaces
 *        syscall instructions with a sequence of lower-level instructions that
 *        use ECALL to transfer control to the supervisor/operating system. */
t_instruction *genPrintIntSyscall(t_program *program, t_regID rs1);

/** Add a new PrintChar syscall instruction at the end of the instruction list
 *  of the specified program. At runtime, this instruction writes a character
 *  whose ASCII encoding is stored in the source register to the standard output
 *  stream.
 *  @param program The program where the instruction will be added
 *  @param rs1 Identifier of the source register (ASCII character to print)
 *  @returns the instruction object added to the instruction list
 *  @note During the target-specific transformation passes, ACSE replaces
 *        syscall instructions with a sequence of lower-level instructions that
 *        use ECALL to transfer control to the supervisor/operating system. */
t_instruction *genPrintCharSyscall(t_program *program, t_regID rs1);

/// @}


/// @name Other Instructions
/// @{

/** Add a new NOP instruction at the end of the instruction list of the
 *  specified program. At runtime, a NOP instruction doesn't do anything (apart
 *  from incrementing the program counter like any other instruction).
 *  @param program The program where the instruction will be added
 *  @returns the instruction object added to the instruction list */
t_instruction *genNOP(t_program *program);

/** Add a new ECALL instruction at the end of the instruction list of the
 *  specified program. At runtime, a ECALL instruction temporarily transfers
 *  program control to a supervisor or operating system.
 *  @param program The program where the instruction will be added
 *  @returns the instruction object added to the instruction list
 *  @note This function is only used internally by the target-specific
 *        transformation pass, it is not useful outside of that context. */
t_instruction *genECALL(t_program *program);

/** Add a new EBREAK instruction at the end of the instruction list of the
 *  specified program. At runtime, a EBREAK instruction temporarily stops
 *  program execution for debugging purposes.
 *  @param program The program where the instruction will be added
 *  @returns the instruction object added to the instruction list
 *  @note This function is only used internally by the target-specific
 *        transformation pass, it is not useful outside of that context. */
t_instruction *genEBREAK(t_program *program);

/// @}

/**
 * @}
 */


#endif
