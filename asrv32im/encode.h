#ifndef ENCODE_H
#define ENCODE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "object.h"

#define MAX_EXP_FACTOR 2

size_t encGetInstrLength(t_instruction instr);
int encExpandPseudoInstruction(
    t_instruction instr, t_instruction res[MAX_EXP_FACTOR]);
bool encResolveImmediates(t_instruction *instr, uint32_t pc);
bool encPhysicalInstruction(t_instruction instr, uint32_t pc, t_data *out);

#endif
