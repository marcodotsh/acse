/// @file target_asm_print.h
/// @brief Generation of the output assembly program

#ifndef TARGET_ASM_PRINT_H
#define TARGET_ASM_PRINT_H

#include <stdbool.h>
#include <stdio.h>
#include "program.h"

/**
 * @defgroup asm_print Assembly code output
 * @brief Functions to print the assembly code of the compiled program
 *
 * At the end of compilation, the assembly code needs to be printed to a file
 * to be later processed by the assembler. These functions are used to perform
 * this task and related assembly printing operations when necessary for
 * logging purposes.
 * @{
 */

/** Format a register to a string.
 * @param regID         The register ID that needs to be formatted.
 * @param machineRegIDs True if the register ID needs to be treated as a
 *                      physical register, false if the register ID is a
 *                      temporary register.
 * @returns The newly formatted string, which is owned by the caller and needs
 *          to be freed. NULL if regID refers to a non-existent register ID. */
char *registerIDToString(t_regID regID, bool machineRegIDs);

/** Print the specified instruction to a file.
 *  @param inst          The instruction to be printed.
 *  @param fp            The stream where to print the instruction.
 *  @param machineRegIDs True if the instruction uses physical registers (i.e.
 *                       register allocation has already been performed)
 * @returns false if an error occurred while writing to the stream. */
bool printInstruction(t_instruction *inst, FILE *fp, bool machineRegIDs);

/** Write the final assembly code for the program to the specified file.
 *  @param program The program being compiled.
 *  @param fn      The path of the output file.
 *  @returns false if an error occurred while writing to the file. */
bool writeAssembly(t_program *program, const char *fn);

/**
 * @}
 */

#endif
