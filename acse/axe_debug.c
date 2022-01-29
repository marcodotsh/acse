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

static void debugPrintInstruction(t_axe_instruction *instr, FILE *fout);
static void printArrayOfVariables(t_cflow_var **array, int size, FILE *fout);
static void printListOfVariables(t_list *variables, FILE *fout);
static void printCFlowGraphVariable(t_cflow_var *var, FILE *fout);
static void printBBlockInfos(t_basic_block *block, FILE *fout, int verbose);
static void printLiveIntervals(t_list *intervals, FILE *fout);
static void printBindings(int *bindings, int numVars, FILE *fout);
static void debugPrintLabel(t_axe_label *label, FILE *fout);


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

void printBBlockInfos(t_basic_block *block, FILE *fout, int verbose)
{
   t_list *current_element;
   t_cflow_Node *current_node;
   int count;
   
   /* preconditions */
   if (block == NULL)
      return;
   if (fout == NULL)
      return;

   fprintf(fout,"NUMBER OF PREDECESSORS : %d \n"
         , getLength(block->pred) );
   fprintf(fout,"NUMBER OF SUCCESSORS : %d \n"
         , getLength(block->succ) );
   fprintf(fout,"NUMBER OF INSTRUCTIONS : %d \n"
         , getLength(block->nodes) );

   count = 1;
   current_element = block->nodes;
   while(current_element != NULL)
   {
      current_node = (t_cflow_Node *) LDATA(current_element);
      fprintf(fout,"\t%d.  ", count);
      debugPrintInstruction(current_node->instr, fout);
      if (verbose != 0)
      {
         fprintf(fout, "\n\t\t\tDEFS = [");
         printArrayOfVariables(current_node->defs, CFLOW_MAX_DEFS, fout);
         fprintf(fout, "]");
         fprintf(fout, "\n\t\t\tUSES = [");
         printArrayOfVariables(current_node->uses, CFLOW_MAX_USES, fout);
         fprintf(fout, "]");

         fprintf(fout, "\n\t\t\tLIVE IN = [");
         printListOfVariables(current_node->in, fout);
         fprintf(fout, "]");
         fprintf(fout, "\n\t\t\tLIVE OUT = [");
         printListOfVariables(current_node->out, fout);
         fprintf(fout, "]");
      }
      
      fprintf(fout, "\n");
      count++;
      current_element = LNEXT(current_element);
   }
   fflush(fout);
}

void printArrayOfVariables(t_cflow_var **array, int size, FILE *fout)
{
   int foundVariables = 0;
   int i;
   
   for (i=0; i<size; i++) {
      if (!(array[i]))
         continue;
         
      if (foundVariables > 0)
         fprintf(fout, ", ");
         
      printCFlowGraphVariable(array[i], fout);
      foundVariables++;
   }
   
   fflush(fout);
}

void printListOfVariables(t_list *variables, FILE *fout)
{
   t_list *current_element;
   t_cflow_var *current_variable;
   
   if (variables == NULL)
      return;
   if (fout == NULL)
      return;

   current_element = variables;
   while(current_element != NULL)
   {
      current_variable = (t_cflow_var *) LDATA(current_element);
      printCFlowGraphVariable(current_variable, fout);
      if (LNEXT(current_element) != NULL)
         fprintf(fout, ", ");
      
      current_element = LNEXT(current_element);
   }
   fflush(fout);
}

void printCFlowGraphVariable(t_cflow_var *var, FILE *fout)
{
   if (var->ID == VAR_PSW)
      fprintf(fout, "PSW");
   else if (var->ID == VAR_UNDEFINED)
      fprintf(fout, "<!UNDEF!>");
   else
      fprintf(fout, "R%d", var->ID);
}

void printGraphInfos(t_cflow_Graph *graph, FILE *fout, int verbose)
{
   int counter;
   t_list *current_element;
   t_basic_block *current_bblock;
   
   /* preconditions */
   if (graph == NULL)
      return;
   if (fout == NULL)
      return;

   /* initialization of the local variables */
   counter = 1;
   
   fprintf(fout,"NOTE : Temporary registers are considered as\n"
                "       variables of the intermediate language. \n");
#if CFLOW_ALWAYS_LIVEIN_R0 == (1)
   fprintf(fout,"       Variable \'R0\' (that refers to the \n"
                "       physical register \'R0\') is always \n"
                "       considered LIVE-IN for each node of \n"
                "       a basic block. \n"
                "       Thus, in the following control flow graph, \n"
                "       \'R0\' will never appear as LIVE-IN or LIVE-OUT\n"
                "       variable for a statement.\n\n"
                "       If you want to consider \'R0\' as\n"
                "       a normal variable, you have to set\n"
                "       to 0 the value of the macro CFLOW_ALWAYS_LIVEIN_R0\n"
                "       defined in \"cflow_constants.h\".\n\n");
#endif
   fprintf(fout,"\n");
   fprintf(fout,"**************************\n");
   fprintf(fout,"     CONTROL FLOW GRAPH   \n");
   fprintf(fout,"**************************\n");
   fprintf(fout,"NUMBER OF BASIC BLOCKS : %d \n"
         , getLength(graph->blocks));
   fprintf(fout,"NUMBER OF USED VARIABLES : %d \n"
         , getLength(graph->cflow_variables));
   fprintf(fout,"--------------------------\n");
   fprintf(fout,"START BASIC BLOCK INFOS.  \n");
   fprintf(fout,"--------------------------\n");

   /* initialize `current_block' */
   current_element = graph->blocks;
   while(current_element != NULL)
   {
      current_bblock = (t_basic_block *) LDATA(current_element);
      fprintf(fout,"[BLOCK %d] \n", counter);
      printBBlockInfos(current_bblock, fout, verbose);
      if (LNEXT(current_element) != NULL)
         fprintf(fout,"--------------------------\n");
      else
         fprintf(fout,"**************************\n");

      counter++;
      current_element = LNEXT(current_element);
   }
   
   fprintf(fout,"\n\n");
   fflush(fout);
}

void debugPrintInstruction(t_axe_instruction *instr, FILE *fout)
{
   off_t formBegin = ftello(fout);

   /* preconditions */
   if (fout == NULL)
      return;
   
   if (instr == NULL)
   {
      fprintf(fout, "[NULL]\n");
      return;
   }

   printInstruction(instr, fout, 0);
}

char * dataTypeToString(int codedType)
{
   switch (codedType)
   {
      case INTEGER_TYPE : return "INTEGER";
      default : return "<INVALID_TYPE>";
   }
}

void debugPrintLabel(t_axe_label *label, FILE *fout)
{
   char *labelName = getLabelName(label);
   fprintf(fout, "%s (ID=%d)", labelName, label->labelID);
   free(labelName);
}
