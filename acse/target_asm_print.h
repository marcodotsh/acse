/*
 * Daniele Cattaneo
 * Politecnico di Milano, 2020
 *
 * target_asm_print.h
 * Formal Languages & Compilers Machine, 2007-2020
 *
 * Generation of the output assembly program.
 */

#ifndef TARGET_ASM_PRINT_H
#define TARGET_ASM_PRINT_H

#include "program.h"
#include "errors.h"

/** Convert an opcode ID to a string. */
extern const char *opcodeToString(int opcode);

/** Convert a register to a string. The result string is dynamically allocated
 * and must be freed. Returns NULL on error. */
extern char *registerIDToString(int regID, int machineRegIDs);

/** Print the specified instruction to a file */
extern int printInstruction(t_instruction *inst, FILE *fp, int machineRegIDs);

/** write the assembly for the given program */
extern void writeAssembly(t_program *program);

#endif
