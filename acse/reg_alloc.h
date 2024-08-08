/// @file reg_alloc.h
/// @brief Register allocation pass

#ifndef REG_ALLOC_H
#define REG_ALLOC_H

#include <stdio.h>
#include "program.h"

/**
 * @addtogroup pipeline
 * @{
 */

typedef struct t_regAllocator t_regAllocator;

t_regAllocator *newRegAllocator(t_program *program);

/** Convert temporary register identifiers to real register identifiers,
 * analyzing the live interval of each temporary register.
 * @param program The program where to allocate registers */
void doRegisterAllocation(t_regAllocator *regAlloc);

void dumpRegAllocation(t_regAllocator *regAlloc, FILE *fout);

void deleteRegAllocator(t_regAllocator *regAlloc);

/**
 * @}
 */

#endif
