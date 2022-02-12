/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * reg_alloc.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 * Register allocation pass
 */

#ifndef REG_ALLOC_H
#define REG_ALLOC_H

#include "engine.h"

/* Convert temporary register identifiers to real register identifiers,
 * analyzing the live interval of each temporary register. */
extern void doRegisterAllocation(t_program_infos *program);

#endif
