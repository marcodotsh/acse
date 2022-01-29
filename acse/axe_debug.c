/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_debug.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#include <stdarg.h>
#include "axe_debug.h"
#include "collections.h"
#include "axe_reg_alloc.h"
#include "axe_target_info.h"
#include "axe_target_asm_print.h"

int debugPrintf(const char *fmt, ...)
{
#ifndef NDEBUG
   int res;
   va_list args;

   va_start(args, fmt);
   res = vprintf(fmt, args);
   va_end(args);
   return res;
#else
   return 0;
#endif
}
