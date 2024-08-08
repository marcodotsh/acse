/// @file target_asm_print.h
/// @brief Generation of the output assembly program.

#ifndef TARGET_ASM_PRINT_H
#define TARGET_ASM_PRINT_H

#include <stdbool.h>
#include "program.h"

/** Convert an opcode ID to a string. */
const char *opcodeToString(int opcode);

/** Convert a register to a string. The result string is dynamically allocated
 * and must be freed. Returns NULL on error. */
char *registerIDToString(t_regID regID, bool machineRegIDs);

/** Print the specified instruction to a file */
int printInstruction(t_instruction *inst, FILE *fp, bool machineRegIDs);

/**
 * @addtogroup pipeline
 * @{
 */

/** write the assembly for the given program */
bool writeAssembly(t_program *program, const char *fn);

/**
 * @}
 */

#endif
