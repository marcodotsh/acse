/// @file target_transform.h
/// @brief Transformation pass for lowering target machine details

#ifndef TARGET_TRANSFORM_H
#define TARGET_TRANSFORM_H

#include "program.h"

/**
 * @addtogroup target
 * @{
 */

/** Perform lowering of the program to a subset of the IR which can be
 *  represented as instructions of the target architecture.
 *  @param program The program that needs to be transformed. The transformation
 *                 is performed in-place. */
void doTargetSpecificTransformations(t_program *program);

/**
 * @}
 */

#endif
