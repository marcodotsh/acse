#include <stdio.h>
#include <inttypes.h>
#include "supervisor.h"
#include "memory.h"
#include "debugger.h"


const t_memAddress svStackTop = 0x80000000;
t_memAddress svStackBottom;
t_isaInt svExitCode;


t_svError svInit(void)
{
  svStackBottom = svStackTop - SV_STACK_PAGE_SIZE;
  t_memError merr = memMapArea(svStackBottom, SV_STACK_PAGE_SIZE, NULL);
  if (merr != MEM_NO_ERROR)
    return SV_MEMORY_ERROR;
  cpuSetRegister(CPU_REG_SP, svStackTop - 4);
  return SV_NO_ERROR;
}


void svExpandStack(void)
{
  t_memAddress faultAddr = memGetLastFaultAddress();
  if (faultAddr < svStackBottom &&
      faultAddr >= (svStackBottom - SV_STACK_PAGE_SIZE)) {
    svStackBottom -= SV_STACK_PAGE_SIZE;
    memMapArea(svStackBottom, SV_STACK_PAGE_SIZE, NULL);
  }
}


enum {
  SV_SYSCALL_PRINT_INT = 1,
  SV_SYSCALL_READ_INT = 5,
  SV_SYSCALL_EXIT_0 = 10,
  SV_SYSCALL_PRINT_CHAR = 11,
  SV_SYSCALL_READ_CHAR = 12,
  SV_SYSCALL_EXIT = 93
};

t_svStatus svHandleEnvCall(void)
{
  t_cpuURegValue syscallId = cpuGetRegister(CPU_REG_A7);
  int32_t ret;

  switch (syscallId) {
    case SV_SYSCALL_PRINT_INT:
      fprintf(stdout, "%d", cpuGetRegister(CPU_REG_A0));
      break;
    case SV_SYSCALL_READ_INT:
      fputs("int value? >", stdout);
      fscanf(stdin, "%" PRId32, &ret);
      cpuSetRegister(CPU_REG_A0, (t_cpuURegValue)ret);
      break;
    case SV_SYSCALL_EXIT_0:
      svExitCode = 0;
      return SV_STATUS_TERMINATED;
    case SV_SYSCALL_PRINT_CHAR:
      putchar((int)cpuGetRegister(CPU_REG_A0));
      break;
    case SV_SYSCALL_READ_CHAR:
      ret = getchar();
      cpuSetRegister(CPU_REG_A0, (t_cpuURegValue)ret);
      break;
    case SV_SYSCALL_EXIT:
      svExitCode = (int)cpuGetRegister(CPU_REG_A0);
      return SV_STATUS_TERMINATED;
    default:
      return SV_STATUS_INVALID_SYSCALL;
  }

  return SV_STATUS_RUNNING;
}


t_isaInt svGetExitCode(void)
{
  return svExitCode;
}


t_svStatus svVMTick(void)
{
  t_svStatus status = SV_STATUS_RUNNING;

  t_dbgResult dbgRes = dbgTick();
  if (dbgRes == DBG_RESULT_EXIT) {
    status = SV_STATUS_KILLED;
  } else {
    t_cpuStatus cpuStatus = cpuTick();
    if (cpuStatus == CPU_STATUS_MEMORY_FAULT) {
      svExpandStack();
      cpuClearLastFault();
      cpuStatus = cpuTick();
    }

    if (cpuStatus == CPU_STATUS_ECALL_TRAP) {
      status = svHandleEnvCall();
      if (status == SV_STATUS_RUNNING)
        cpuClearLastFault();
    } else if (cpuStatus == CPU_STATUS_EBREAK_TRAP) {
      if (dbgGetEnabled()) {
        dbgRequestEnter();
        cpuClearLastFault();
      }
    } else if (cpuStatus == CPU_STATUS_ILL_INST_FAULT)
      status = SV_STATUS_ILL_INST_FAULT;
    else if (cpuStatus == CPU_STATUS_MEMORY_FAULT)
      status = SV_STATUS_MEMORY_FAULT;
  }

  return status;
}
