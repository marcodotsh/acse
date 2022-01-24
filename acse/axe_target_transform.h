/*
 * Daniele Cattaneo
 * Politecnico di Milano, 2020
 * 
 * axe_target_transform.h
 * Formal Languages & Compilers Machine, 2007-2020
 * 
 * Transformation passes used to abstract machine-dependent details from the
 * intermediate representation
 */

#ifndef _AXE_TARGET_TRANSFORM_H
#define _AXE_TARGET_TRANSFORM_H

#include "axe_engine.h"

/* fake opcodes for marking registers as used or defined */
#define CFLOW_OPC_DEFINE 0x10000000
#define CFLOW_OPC_USE    0x10000001

/* Perform lowering of the program to a subset of the IR which can be
 * represented as instructions of the target architecture. */
void doTargetSpecificTransformations(t_program_infos *program);

#endif
