/*
 * Daniele Cattaneo
 * Politecnico di Milano, 2020
 * 
 * axe_target_info.h
 * Formal Languages & Compilers Machine, 2007-2020
 * 
 * Properties of the target machine
 */

#ifndef _AXE_TARGET_INFO_H
#define _AXE_TARGET_INFO_H

#include "axe_engine.h"


#define TARGET_NAME "MACE"

/* Number of registers for this target (excluding REG_0) */
#define NUM_REGISTERS 31
/* Number of registers to reserve for spilled temporaries. Shall be equal to
 * the maximum number of unique register operands in a single instruction. */
#define NUM_SPILL_REGS 3

/* Number of bytes for each memory address */
#define TARGET_PTR_GRANULARITY 4

/* Target opcodes */
#define OPC_ADD 0
#define OPC_SUB 1
#define OPC_ANDL 2
#define OPC_ORL 3
#define OPC_EORL 4
#define OPC_ANDB 5
#define OPC_ORB 6
#define OPC_EORB 7
#define OPC_MUL 8
#define OPC_DIV 9
#define OPC_SHL 10
#define OPC_SHR 11
#define OPC_ROTL 12
#define OPC_ROTR 13
#define OPC_NEG 14
#define OPC_SPCL 15
#define OPC_ADDI 16
#define OPC_SUBI 17
#define OPC_ANDLI 18
#define OPC_ORLI 19
#define OPC_EORLI 20
#define OPC_ANDBI 21
#define OPC_ORBI 22
#define OPC_EORBI 23
#define OPC_MULI 24
#define OPC_DIVI 25
#define OPC_SHLI 26
#define OPC_SHRI 27
#define OPC_ROTLI 28
#define OPC_ROTRI 29
#define OPC_NOTL 30
#define OPC_NOTB 31
#define OPC_NOP 32
#define OPC_MOVA 33
#define OPC_JSR 34
#define OPC_RET 35
#define OPC_HALT 36
#define OPC_SEQ 37
#define OPC_SGE 38
#define OPC_SGT 39
#define OPC_SLE 40
#define OPC_SLT 41
#define OPC_SNE 42
#define OPC_BT 43
#define OPC_BF 44
#define OPC_BHI 45
#define OPC_BLS 46
#define OPC_BCC 47
#define OPC_BCS 48
#define OPC_BNE 49
#define OPC_BEQ 50
#define OPC_BVC 51
#define OPC_BVS 52
#define OPC_BPL 53
#define OPC_BMI 54
#define OPC_BGE 55
#define OPC_BLT 56
#define OPC_BGT 57
#define OPC_BLE 58
#define OPC_LOAD 59
#define OPC_STORE 60
#define OPC_AXE_READ 61
#define OPC_AXE_WRITE 62

/* Returns 1 if `instr` is a jump (branch) instruction. */
extern int isJumpInstruction(t_axe_instruction *instr);

/* Returns 1 if `instr` is a unconditional jump instruction (BT, BF) */
extern int isUnconditionalJump(t_axe_instruction *instr);

/* Returns 1 if `instr` is either the HALT instruction or the RET
 * instruction. */
extern int isHaltOrRetInstruction(t_axe_instruction *instr);

#endif


