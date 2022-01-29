/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_debug.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 * Debug print utility functions
 */

#ifndef _AXE_DEBUG_H
#define _AXE_DEBUG_H

#include <stdio.h>
#include "axe_cflow_graph.h"
#include "axe_reg_alloc.h"

/* In debug builds (NDEBUG not defined), prints a message on the standard
 * output like `printf'. Otherwise, does nothing and returns zero. */
extern int debugPrintf(const char *fmt, ...);

#endif
