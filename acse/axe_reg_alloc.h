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
#include "collections.h"

/* constants */
#define RA_SPILL_REQUIRED -1
#define RA_REGISTER_INVALID 0
#define RA_EXCLUDED_VARIABLE 0

typedef struct t_live_interval {
   int varID;     /* a variable identifier */
   t_list *mcRegConstraints; /* list of all registers where this variable can
                              * be allocated. */
   int startPoint;  /* the index of the first instruction
                     * that make use of (or define) this variable */
   int endPoint;   /* the index of the last instruction
                    * that make use of (or define) this variable */
} t_live_interval;

typedef struct t_reg_allocator {
   t_list *live_intervals;    /* an ordered list of live intervals */
   int regNum;                /* the number of registers of the machine */
   int varNum;                /* number of variables */
   int *bindings;             /* an array of bindings of kind : varID-->register.
                               * If a certain variable X need to be spilled
                               * in memory, the value of `register' is set
                               * to the value of the macro RA_SPILL_REQUIRED */
   t_list *freeRegisters;     /* a list of free registers */
} t_reg_allocator;


/* Convert temporary register identifiers to real register identifiers,
 * analyzing the live interval of each temporary register. */
extern void doRegisterAllocation(t_program_infos *program);

#endif
