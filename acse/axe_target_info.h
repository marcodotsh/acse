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

/* Number of bytes for each memory address */
#define TARGET_PTR_GRANULARITY 1


/* Register names */
enum {
   REG_ZERO = REG_0,
   REG_RA, REG_SP, REG_GP, REG_TP,
   REG_T0, REG_T1, REG_T2,
   REG_S0, REG_S1,
   REG_A0, REG_A1, REG_A2, REG_A3, REG_A4, REG_A5, REG_A6, REG_A7, 
   REG_S2, REG_S3, REG_S4, REG_S5, REG_S6,
   REG_S7, REG_S8, REG_S9, REG_S10, REG_S11,
   REG_T3, REG_T4, REG_T5, REG_T6,
   NUM_REGISTERS
};

/* Number of general-purpose registers for this target usable by the register
 * allocator. */
#define NUM_GP_REGS    (REG_S11-REG_T0+1)
/* Number of registers available for spilled temporaries. Should be equal to
 * the maximum number of unique register operands in a single instruction. */
#define NUM_SPILL_REGS (REG_T5-REG_T3+1)

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
#define OPC_ADD       0  /* rd = rs1 +  rs2                                  */
#define OPC_SUB       1  /* rd = rs1 -  rs2                                  */
#define OPC_AND       5  /* rd = rs1 &  rs2                                  */
#define OPC_OR        6  /* rd = rs1 |  rs2                                  */
#define OPC_XOR       7  /* rd = rs1 ^  rs2                                  */
#define OPC_MUL       8  /* rd = rs1 *  rs2                                  */
#define OPC_DIV       9  /* rd = rs1 /  rs2                                  */
#define OPC_SLL      10  /* rd = rs1 << rs2                                  */
#define OPC_SRL    1000  /* rd = rs1 >> rs2            (logical)             */
#define OPC_SRA      11  /* rd = rs1 >> rs2            (arithmetic)          */
           
/*   Arithmetic with immediate */           
#define OPC_ADDI     16  /* rd = rs1 +  imm                                  */
#define OPC_SUBI     17  /* rd = rs1 -  imm                         (pseudo) */
#define OPC_ANDI     21  /* rd = rs1 &  imm                                  */
#define OPC_ORI      22  /* rd = rs1 |  imm                                  */
#define OPC_XORI     23  /* rd = rs1 ^  imm                                  */
#define OPC_MULI     24  /* rd = rs1 *  imm                         (pseudo) */
#define OPC_DIVI     25  /* rd = rs1 /  imm                         (pseudo) */
#define OPC_SLLI     26  /* rd = rs1 << imm                                  */
#define OPC_SRLI   1100  /* rd = rs1 >> imm            (logical)             */
#define OPC_SRAI     27  /* rd = rs1 >> imm            (arithmetic)          */

/*   Comparison */
#define OPC_SEQ    1200  /* rd = rs1 == rs2                         (pseudo) */
#define OPC_SNE    1201  /* rd = rs1 != rs2                         (pseudo) */
#define OPC_SLT    1202  /* rd = rs1 <  rs2            (signed)              */
#define OPC_SLTU   1203  /* rd = rs1 <  rs2            (unsigned)            */
#define OPC_SGE    1204  /* rd = rs1 >= rs2            (signed)     (pseudo) */
#define OPC_SGEU   1205  /* rd = rs1 >= rs2            (unsigned)   (pseudo) */
#define OPC_SGT    1206  /* rd = rs1 >  rs2            (signed)     (pseudo) */
#define OPC_SGTU   1207  /* rd = rs1 >  rs2            (unsigned)   (pseudo) */
#define OPC_SLE    1208  /* rd = rs1 <= rs2            (signed)     (pseudo) */
#define OPC_SLEU   1209  /* rd = rs1 <= rs2            (unsigned)   (pseudo) */
            
/*   Comparison with immediate */             
#define OPC_SEQI   1210  /* rd = rs1 == imm                         (pseudo) */
#define OPC_SNEI   1211  /* rd = rs1 != imm                         (pseudo) */
#define OPC_SLTI   1212  /* rd = rs1 <  imm            (signed)              */
#define OPC_SLTIU  1213  /* rd = rs1 <  imm            (unsigned)            */
#define OPC_SGEI   1214  /* rd = rs1 >= imm            (signed)     (pseudo) */
#define OPC_SGEIU  1215  /* rd = rs1 >= imm            (unsigned)   (pseudo) */
#define OPC_SGTI   1216  /* rd = rs1 >  imm            (signed)     (pseudo) */
#define OPC_SGTIU  1217  /* rd = rs1 >  imm            (unsigned)   (pseudo) */
#define OPC_SLEI   1218  /* rd = rs1 <= imm            (signed)     (pseudo) */
#define OPC_SLEIU  1219  /* rd = rs1 <= imm            (unsigned)   (pseudo) */

/*   Jump/Branch */
#define OPC_J      1600  /* goto addr;                                       */
#define OPC_BEQ    1300  /* if (rs1 == rs2) goto addr;                       */
#define OPC_BNE    1301  /* if (rs1 == rs2) goto addr;                       */
#define OPC_BLT    1302  /* if (rs1 <  rs2) goto addr; (signed)              */
#define OPC_BLTU   1303  /* if (rs1 <  rs2) goto addr; (unsigned)            */
#define OPC_BGE    1304  /* if (rs1 >= rs2) goto addr; (signed)              */
#define OPC_BGEU   1305  /* if (rs1 >= rs2) goto addr; (unsigned)            */
#define OPC_BGT    1306  /* if (rs1 >  rs2) goto addr; (signed)     (pseudo) */
#define OPC_BGTU   1307  /* if (rs1 >  rs2) goto addr; (unsigned)   (pseudo) */
#define OPC_BLE    1308  /* if (rs1 <= rs2) goto addr; (signed)     (pseudo) */
#define OPC_BLEU   1309  /* if (rs1 <= rs2) goto addr; (unsigned)   (pseudo) */

/*   Load/Store */
#define OPC_LI     1400  /* rd = imm                                         */
#define OPC_LA       33  /* rd = addr                                        */
#define OPC_LW     1401  /* rd = *(int *)(immediate + rs1)                   */
#define OPC_SW     1402  /* *(int *)(immediate + rs1) = rs2                  */
#define OPC_LW_G   1403  /* rd = *(int *)label                      (pseudo) */
#define OPC_SW_G   1404  /* *(int *)label = rs2                     (pseudo) */

/*   Other */
#define OPC_NOP      32
#define OPC_ECALL  1500
#define OPC_EBREAK 1501

/* Syscall opcodes */
#define OPC_HALT        36
#define OPC_AXE_READ    61
#define OPC_AXE_WRITE   62


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

/* Returns a register ID suitable for spill operations. The maximum index
 * is always bounded by NUM_SPILL_REGS. */
extern int getSpillRegister(int i);
/* Returns the list of register IDs available for the register allocator,
 * sorted in order of priority. */
extern t_list *getListOfGenPurposeRegisters(void);

#endif


