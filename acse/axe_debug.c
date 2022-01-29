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

static void printLiveIntervals(t_list *intervals, FILE *fout);
static void printBindings(int *bindings, int numVars, FILE *fout);


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

void printBindings(int *bindings, int numVars, FILE *fout)
{
   int counter;
   
   if (bindings == NULL)
      return;

   if (fout == NULL)
      return;

   fprintf(fout, "BINDINGS : \n");
   for (counter = 0; counter < numVars; counter++) {
      if (bindings[counter] != RA_SPILL_REQUIRED)
      {
         fprintf(fout, "VAR T%d will be assigned to register R%d \n"
                  , counter, bindings[counter]);
      }
      else
      {
         fprintf(fout, "VAR T%d will be spilled \n", counter);
      }
   }

   fflush(fout);
}

void printRegAllocInfos(t_reg_allocator *RA, FILE *fout)
{
   if (RA == NULL)
      return;
   if (fout == NULL)
      return;
   fprintf(fout, "\n\n*************************\n");
   fprintf(fout, "REGISTER ALLOCATION INFOS\n");
   fprintf(fout, "*************************\n");
   fprintf(fout, "AVAILABLE REGISTERS : %d \n", RA->regNum + 3);
   fprintf(fout, "USED VARIABLES : %d \n", RA->varNum);
   fprintf(fout, "-------------------------\n");
   printLiveIntervals(RA->live_intervals, fout);
   fprintf(fout, "-------------------------\n");
   printBindings(RA->bindings, RA->varNum, fout);
   fprintf(fout, "*************************\n\n");
   fflush(fout);
}

void printLiveIntervals(t_list *intervals, FILE *fout)
{
   t_list *current_element;
   t_live_interval *interval;

   /* precondition */
   if (fout == NULL)
      return;

   fprintf(fout, "LIVE_INTERVALS:\n");

   /* retireve the first element of the list */
   current_element = intervals;
   while (current_element != NULL)
   {
      interval = (t_live_interval *) LDATA(current_element);

      fprintf(fout, "\tLIVE_INTERVAL of T%d : [%d, %d]"
            , interval->varID, interval->startPoint, interval->endPoint);

      if (interval->mcRegConstraints) {
         t_list *i = interval->mcRegConstraints;
         fprintf(fout, " CONSTRAINED TO R%d", LINTDATA(i));
         i = LNEXT(i);
         for (; i; i = LNEXT(i)) {
             fprintf(fout, ", R%d", LINTDATA(i));
         }
      }

      fprintf(fout, "\n");
      
      /* retrieve the next element in the list of intervals */
      current_element = LNEXT(current_element);
   }
   fflush(fout);
}
