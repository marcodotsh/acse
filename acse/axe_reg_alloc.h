/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_reg_alloc.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 * Register allocation pass
 */

#ifndef _AXE_REG_ALLOC_H
#define _AXE_REG_ALLOC_H

#include "axe_engine.h"

/* Convert temporary register identifiers to real register identifiers,
 * analyzing the live interval of each temporary register.
 * Returns AXE_OK if successful, an error code otherwise. */
extern int doRegisterAllocation(t_program_infos *program);

#endif
