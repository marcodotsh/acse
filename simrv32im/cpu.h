#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include "isa.h"

typedef int t_cpuStatus;
enum {
  CPU_STATUS_OK = 0,
  CPU_STATUS_MEMORY_FAULT = -1,
  CPU_STATUS_ILL_INST_FAULT = -2,
  CPU_STATUS_ECALL_TRAP = -3,
  CPU_STATUS_EBREAK_TRAP = -4
};

t_cpuURegValue cpuGetRegister(t_cpuRegID reg);
void cpuSetRegister(t_cpuRegID reg, t_cpuURegValue value);

void cpuReset(t_cpuURegValue pcValue);
t_cpuStatus cpuTick(void);
t_cpuStatus cpuClearLastFault(void);

#endif
