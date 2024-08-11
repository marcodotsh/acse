#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <stdbool.h>
#include <stddef.h>
#include "memory.h"

typedef int t_dbgResult;
enum {
  DBG_RESULT_CONTINUE,
  DBG_RESULT_EXIT
};

typedef int t_dbgBreakpointId;
#define DBG_BREAKPOINT_INVALID ((t_dbgBreakpointId) - 1)

typedef void *t_dbgEnumBreakpointState;
#define DBG_ENUM_BREAKPOINT_START ((t_dbgEnumBreakpointState)NULL)
#define DBG_ENUM_BREAKPOINT_STOP ((t_dbgEnumBreakpointState)NULL)


bool dbgEnable(void);
bool dbgGetEnabled(void);
bool dbgDisable(void);
void dbgRequestEnter(void);

int dbgPrintf(const char *format, ...);

t_dbgBreakpointId dbgAddBreakpoint(t_memAddress address);
bool dbgRemoveBreakpoint(t_dbgBreakpointId brkId);
t_memAddress dbgGetBreakpoint(t_dbgBreakpointId brkId);
t_dbgEnumBreakpointState dbgEnumerateBreakpoints(t_dbgEnumBreakpointState state,
    t_dbgBreakpointId *outId, t_memAddress *outAddress);

t_dbgResult dbgTick(void);

#endif
