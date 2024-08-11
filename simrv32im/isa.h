#ifndef ISA_H
#define ISA_H

#include <stddef.h>
#include <stdint.h>

#define ISA_XSIZE (32)
typedef int32_t t_isaSXSize;
typedef uint32_t t_isaUXSize;
typedef int32_t t_isaInt;
typedef uint32_t t_isaUInt;

typedef t_isaUXSize t_cpuURegValue;
typedef t_isaSXSize t_cpuSRegValue;

typedef unsigned int t_cpuRegID;
enum {
  CPU_REG_X0 = 0,
  CPU_REG_ZERO = CPU_REG_X0,
  CPU_REG_X1,
  CPU_REG_RA = CPU_REG_X1,
  CPU_REG_X2,
  CPU_REG_SP = CPU_REG_X2,
  CPU_REG_X3,
  CPU_REG_GP = CPU_REG_X3,
  CPU_REG_X4,
  CPU_REG_TP = CPU_REG_X4,
  CPU_REG_X5,
  CPU_REG_T0 = CPU_REG_X5,
  CPU_REG_X6,
  CPU_REG_T1 = CPU_REG_X6,
  CPU_REG_X7,
  CPU_REG_R2 = CPU_REG_X7,
  CPU_REG_X8,
  CPU_REG_FP = CPU_REG_X8,
  CPU_REG_S0 = CPU_REG_X8,
  CPU_REG_X9,
  CPU_REG_S1 = CPU_REG_X9,
  CPU_REG_X10,
  CPU_REG_A0 = CPU_REG_X10,
  CPU_REG_X11,
  CPU_REG_A1 = CPU_REG_X11,
  CPU_REG_X12,
  CPU_REG_A2 = CPU_REG_X12,
  CPU_REG_X13,
  CPU_REG_A3 = CPU_REG_X13,
  CPU_REG_X14,
  CPU_REG_A4 = CPU_REG_X14,
  CPU_REG_X15,
  CPU_REG_A5 = CPU_REG_X15,
  CPU_REG_X16,
  CPU_REG_A6 = CPU_REG_X16,
  CPU_REG_X17,
  CPU_REG_A7 = CPU_REG_X17,
  CPU_REG_X18,
  CPU_REG_S2 = CPU_REG_X18,
  CPU_REG_X19,
  CPU_REG_S3 = CPU_REG_X19,
  CPU_REG_X20,
  CPU_REG_S4 = CPU_REG_X20,
  CPU_REG_X21,
  CPU_REG_S5 = CPU_REG_X21,
  CPU_REG_X22,
  CPU_REG_S6 = CPU_REG_X22,
  CPU_REG_X23,
  CPU_REG_S7 = CPU_REG_X23,
  CPU_REG_X24,
  CPU_REG_S8 = CPU_REG_X24,
  CPU_REG_X25,
  CPU_REG_S9 = CPU_REG_X25,
  CPU_REG_X26,
  CPU_REG_S10 = CPU_REG_X26,
  CPU_REG_X27,
  CPU_REG_S11 = CPU_REG_X27,
  CPU_REG_X28,
  CPU_REG_T3 = CPU_REG_X28,
  CPU_REG_X29,
  CPU_REG_T4 = CPU_REG_X29,
  CPU_REG_X30,
  CPU_REG_T5 = CPU_REG_X30,
  CPU_REG_X31,
  CPU_REG_T6 = CPU_REG_X31,
  CPU_REG_PC
};

static inline uint32_t sext32(uint32_t x, int amt)
{
  amt = amt % 32;
  if (amt == 0)
    return x;
  
  uint32_t unsValue = (uint32_t)x;
  if (x & (uint32_t)((uint32_t)1 << (amt - 1))) {
    uint32_t ext = (uint32_t)-1 << amt;
    unsValue |= ext;
  }
  return unsValue;
}

static inline uint32_t sra32(uint32_t x, int amt)
{
  amt = amt % 32;
  uint32_t shifted = (uint32_t)((uint32_t)x >> amt);
  return sext32(shifted, 32 - amt);
}

static inline uint32_t bits32(uint32_t x, int a, int b)
{
  uint32_t mask = (uint32_t)((uint32_t)1 << (b - a)) - 1;
  return (uint32_t)(x >> a) & mask;
}

#define SRA(x, amt) (sra32(x, amt))
#define SEXT(x, s) (sext32(x, s))
#define BITS(x, a, b) (bits32(x, a, b))

#define ISA_INST_OPCODE(x) BITS(x, 0, 7)
#define ISA_INST_RD(x) BITS(x, 7, 12)
#define ISA_INST_FUNCT3(x) BITS(x, 12, 15)
#define ISA_INST_RS1(x) BITS(x, 15, 20)
#define ISA_INST_RS2(x) BITS(x, 20, 25)
#define ISA_INST_FUNCT7(x) BITS(x, 25, 32)
#define ISA_INST_I_IMM12(x) BITS(x, 20, 32)
#define ISA_INST_I_IMM12_SEXT(x) SEXT(ISA_INST_I_IMM12(x), 12)
#define ISA_INST_S_IMM12(x) (BITS(x, 7, 12) | (BITS(x, 25, 32) << 5))
#define ISA_INST_S_IMM12_SEXT(x) SEXT(ISA_INST_S_IMM12(x), 12)
#define ISA_INST_B_IMM13(x)                                                 \
  ((BITS(x, 7, 8) << 11) | (BITS(x, 8, 12) << 1) | (BITS(x, 25, 31) << 5) | \
      (BITS(x, 31, 32) << 12))
#define ISA_INST_B_IMM13_SEXT(x) SEXT(ISA_INST_B_IMM13(x), 13)
#define ISA_INST_U_IMM20(x) BITS(x, 12, 32)
#define ISA_INST_U_IMM20_SEXT(x) SEXT(ISA_INST_U_IMM20(x), 20)
#define ISA_INST_J_IMM21(x)                            \
  ((BITS(x, 12, 20) << 12) | (BITS(x, 20, 21) << 11) | \
      (BITS(x, 21, 31) << 1) | (BITS(x, 31, 32) << 20))
#define ISA_INST_J_IMM21_SEXT(x) SEXT(ISA_INST_J_IMM21(x), 21)

#define ISA_INST_OPCODE_CODE(x) (((x) << 2) | 3)
#define ISA_INST_OPCODE_LOAD ISA_INST_OPCODE_CODE(0x00)
#define ISA_INST_OPCODE_OPIMM ISA_INST_OPCODE_CODE(0x04)
#define ISA_INST_OPCODE_AUIPC ISA_INST_OPCODE_CODE(0x05)
#define ISA_INST_OPCODE_STORE ISA_INST_OPCODE_CODE(0x08)
#define ISA_INST_OPCODE_OP ISA_INST_OPCODE_CODE(0x0C)
#define ISA_INST_OPCODE_LUI ISA_INST_OPCODE_CODE(0x0D)
#define ISA_INST_OPCODE_BRANCH ISA_INST_OPCODE_CODE(0x18)
#define ISA_INST_OPCODE_JALR ISA_INST_OPCODE_CODE(0x19)
#define ISA_INST_OPCODE_JAL ISA_INST_OPCODE_CODE(0x1B)
#define ISA_INST_OPCODE_SYSTEM ISA_INST_OPCODE_CODE(0x1C)


int isaDisassemble(uint32_t instr, char *out, size_t bufsz);

#endif
