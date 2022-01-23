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


#define TARGET_NAME "RISC-V_RV32IM"

/* Number of registers for this target (excluding REG_0) */
#define NUM_REGISTERS 31
/* Number of registers to reserve for spilled temporaries. Shall be equal to
 * the maximum number of unique register operands in a single instruction. */
#define NUM_SPILL_REGS 3

/* Number of bytes for each memory address */
#define TARGET_PTR_GRANULARITY 4

/*
 * TARGET OPCODES
 *
 * These are the opcode IDs used internally by ACSE to identify the various
 * instructions.
 *   Some opcodes are labeled "pseudo" if they need to be
 * transformed to a sequence of other non-pseudo opcodes before emitting the
 * output assembly file. (Other pseudo opcodes are handled by the assembler,
 * those are not marked as "pseudo" here.)
 */

/*   Arithmetic */
#define OPC_ADD       0  /* rd = rs1 +  rs2              */
#define OPC_SUB       1  /* rd = rs1 -  rs2              */
#define OPC_AND       5  /* rd = rs1 &  rs2              */
#define OPC_OR        6  /* rd = rs1 |  rs2              */
#define OPC_XOR       7  /* rd = rs1 ^  rs2              */
#define OPC_MUL       8  /* rd = rs1 *  rs2              */
#define OPC_DIV       9  /* rd = rs1 /  rs2              */
#define OPC_SLL      10  /* rd = rs1 << rs2              */
#define OPC_SRL      11  /* rd = rs1 >> rs2 (logical)    */
#define OPC_SRA    1000  /* rd = rs1 >> rs2 (arithmetic) */

/*   Arithmetic with immediate */
#define OPC_ADDI     16  /* rd = rs1 +  imm                       */
#define OPC_SUBI     17  /* rd = rs1 -  imm                       */
#define OPC_ANDI     21  /* rd = rs1 &  imm                       */
#define OPC_ORI      22  /* rd = rs1 |  imm                       */
#define OPC_XORI     23  /* rd = rs1 ^  imm                       */
#define OPC_MULI     24  /* rd = rs1 *  imm              (pseudo) */
#define OPC_DIVI     25  /* rd = rs1 /  imm              (pseudo) */
#define OPC_SLLI     26  /* rd = rs1 << imm                       */
#define OPC_SRLI     27  /* rd = rs1 >> imm (logical)             */
#define OPC_SRAI   1100  /* rd = rs1 >> imm (arithmetic)          */

/*   Comparison */
#define OPC_SEQ    1200  /* rd = rs1 == rs2            (pseudo) */
#define OPC_SNE    1201  /* rd = rs1 == rs2            (pseudo) */
#define OPC_SLT    1202  /* rd = rs1 <  rs2 (signed)            */
#define OPC_SLTU   1203  /* rd = rs1 <  rs2 (unsigned)          */
#define OPC_SGE    1204  /* rd = rs1 >= rs2 (signed)   (pseudo) */
#define OPC_SGEU   1205  /* rd = rs1 >= rs2 (unsigned) (pseudo) */
#define OPC_SGT    1206  /* rd = rs1 >  rs2 (signed)   (pseudo) */
#define OPC_SGTU   1207  /* rd = rs1 >  rs2 (unsigned) (pseudo) */
#define OPC_SLE    1208  /* rd = rs1 <= rs2 (signed)   (pseudo) */
#define OPC_SLEU   1209  /* rd = rs1 <= rs2 (unsigned) (pseudo) */

/*   Compare-And-Branch */
#define OPC_BEQ    1300  /* if (rs1 == rs2) goto addr;                     */
#define OPC_BNE    1301  /* if (rs1 == rs2) goto addr;                     */
#define OPC_BLT    1302  /* if (rs1 <  rs2) goto addr; (signed)            */
#define OPC_BLTU   1303  /* if (rs1 <  rs2) goto addr; (unsigned)          */
#define OPC_BGE    1304  /* if (rs1 >= rs2) goto addr; (signed)            */
#define OPC_BGEU   1305  /* if (rs1 >= rs2) goto addr; (unsigned)          */
#define OPC_BGT    1306  /* if (rs1 >  rs2) goto addr; (signed)   (pseudo) */
#define OPC_BGTU   1307  /* if (rs1 >  rs2) goto addr; (unsigned) (pseudo) */
#define OPC_BLE    1308  /* if (rs1 <= rs2) goto addr; (signed)   (pseudo) */
#define OPC_BLEU   1309  /* if (rs1 <= rs2) goto addr; (unsigned) (pseudo) */

/*   Load/Store */
#define OPC_LW     1400  /* rd = *(int *)(addr + rs1)  */
#define OPC_SW     1401  /* *(int *)(addr + rs1) = rs2 */
#define OPC_LI     1402  /* rd = imm                   */
#define OPC_LA     1403  /* rd = addr                  */

/*   Other */
#define OPC_NOP      32 // pseudo
#define OPC_ECALL  1500
#define OPC_EBREAK 1501

/* Syscall opcodes */
#define OPC_HALT        36
#define OPC_AXE_READ    61
#define OPC_AXE_WRITE   62

/* Opcodes to remove */
#define OPC_OLD_ANDL 2
#define OPC_OLD_ORL 3
#define OPC_OLD_EORL 4
#define OPC_OLD_ROTL 12
#define OPC_OLD_ROTR 13
#define OPC_OLD_NEG 14
#define OPC_OLD_SPCL 15
#define OPC_OLD_ANDLI 18
#define OPC_OLD_ORLI 19
#define OPC_OLD_EORLI 20
#define OPC_OLD_ROTLI 28
#define OPC_OLD_ROTRI 29
#define OPC_OLD_NOTL 30
#define OPC_OLD_NOTB 31
#define OPC_OLD_MOVA 33
#define OPC_OLD_JSR 34
#define OPC_OLD_RET 35
#define OPC_OLD_SEQ 37
#define OPC_OLD_SGE 38
#define OPC_OLD_SGT 39
#define OPC_OLD_SLE 40
#define OPC_OLD_SLT 41
#define OPC_OLD_SNE 42
#define OPC_OLD_BT 43
#define OPC_OLD_BF 44
#define OPC_OLD_BHI 45
#define OPC_OLD_BLS 46
#define OPC_OLD_BCC 47
#define OPC_OLD_BCS 48
#define OPC_OLD_BNE 49
#define OPC_OLD_BEQ 50
#define OPC_OLD_BVC 51
#define OPC_OLD_BVS 52
#define OPC_OLD_BPL 53
#define OPC_OLD_BMI 54
#define OPC_OLD_BGE 55
#define OPC_OLD_BLT 56
#define OPC_OLD_BGT 57
#define OPC_OLD_BLE 58
#define OPC_OLD_LOAD 59
#define OPC_OLD_STORE 60


/* Returns 1 if `instr` is a jump (branch) instruction. */
extern int isJumpInstruction(t_axe_instruction *instr);

/* Returns 1 if `instr` is a unconditional jump instruction (BT, BF) */
extern int isUnconditionalJump(t_axe_instruction *instr);

/* Returns 1 if `instr` is either the HALT instruction or the RET
 * instruction. */
extern int isHaltOrRetInstruction(t_axe_instruction *instr);

/* Returns 1 if the instruction uses/defines the PSW (flags) register, 0
 * otherwise. Always returns zero on architectures that do not have a
 * flags register. */
extern int instructionUsesPSW(t_axe_instruction *instr);
extern int instructionDefinesPSW(t_axe_instruction *instr);

#endif


