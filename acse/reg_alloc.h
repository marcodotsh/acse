/// @file reg_alloc.h
/// @brief Register allocation pass

#ifndef REG_ALLOC_H
#define REG_ALLOC_H

#include <stdio.h>
#include "program.h"

/**
 * @defgroup regalloc Register Allocator
 * @brief Register Allocation functions
 *
 * Once the program has been translated to an initial assembly-like intermediate
 * language, the compiler needs to allocate each temporary register to a
 * physical register. The register allocation object performs this process by
 * using the linear scan algorithm on the program's control flow graph.
 * @{
 */

/** Opaque register allocator object. */
typedef struct t_regAllocator t_regAllocator;

/** Create a new register allocator object for the given program.
 *  @param program The program whose registers need to be allocated.
 *  @return A new register allocator object. */
t_regAllocator *newRegAllocator(t_program *program);

/** Deallocate a register allocator.
 *  @param regAlloc The register allocator object. */
void deleteRegAllocator(t_regAllocator *regAlloc);

/** Convert temporary register identifiers to real register identifiers,
 *  analyzing the live interval of each temporary register.
 *  @param regAlloc The register allocator object. */
void regallocRun(t_regAllocator *regAlloc);

/** Dump the results of register allocation to the specified file.
 *  @param regAlloc The register allocation object.
 *  @param fout     The file where to print the dump. */
void regallocDump(t_regAllocator *regAlloc, FILE *fout);

/**
 * @}
 */

#endif
