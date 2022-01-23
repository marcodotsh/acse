/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_debug.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#include "axe_debug.h"
#include "collections.h"
#include "axe_reg_alloc.h"
#include "axe_target_info.h"

#define LABEL_WIDTH (3*2)
#define INSTR_WIDTH (3*7)

static void printArrayOfVariables(t_cflow_var **array, int size, FILE *fout);
static void printListOfVariables(t_list *variables, FILE *fout);
static void printCFlowGraphVariable(t_cflow_var *var, FILE *fout);
static void printBBlockInfos(t_basic_block *block, FILE *fout, int verbose);
static void printLiveIntervals(t_list *intervals, FILE *fout);
static void printBindings(int *bindings, int numVars, FILE *fout);
static void printLabel(t_axe_label *label, int printInline, FILE *fout);
static off_t printFormPadding(off_t formBegin, int formSize, FILE *fout);

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

void printProgramInfos(t_program_infos *program, FILE *fout)
{
   fprintf(fout,"**************************\n");
   fprintf(fout,"          PROGRAM         \n");
   fprintf(fout,"**************************\n\n");

   fprintf(fout,"-----------\n");
   fprintf(fout," VARIABLES\n");
   fprintf(fout,"-----------\n");
   t_list *cur_var = program->variables;
   while (cur_var) {
      t_axe_variable *var = LDATA(cur_var);
      fprintf(fout, "[%s]\n", var->ID);

      fprintf(fout, "   type = %s", dataTypeToString(var->type));
      if (var->isArray) {
         fprintf(fout, ", array size = %d", var->arraySize);
      } else {
         fprintf(fout, ", scalar initial value = %d", var->init_val);
      }
      fprintf(fout, "\n");

      if (var->isArray) {
         fprintf(fout, "   label = ");
         printLabel(var->labelID, 0, fout);
         fprintf(fout, "\n");
      }

      fprintf(fout, "   location = ");
      int sy_error;
      int reg = getRegLocationOfVariable(program, var->ID);
      if (reg == REG_INVALID)
         fprintf(fout, "N/A");
      else
         fprintf(fout, "R%d", reg);
      fprintf(fout, "\n");

      cur_var = LNEXT(cur_var);
   }

   fprintf(fout,"\n--------------\n");
   fprintf(fout," INSTRUCTIONS\n");
   fprintf(fout,"--------------\n");
   t_list *cur_inst = program->instructions;
   while (cur_inst) {
      t_axe_instruction *instr = LDATA(cur_inst);
      debugPrintInstruction(instr, fout);
      fprintf(fout, "\n");
      cur_inst = LNEXT(cur_inst);
   }

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

   if (instr->labelID != NULL)
      printLabel(instr->labelID, 1, fout);
   formBegin = printFormPadding(formBegin, LABEL_WIDTH, fout);
   
   switch(instr->opcode)
   {
      case OPC_ADD : fprintf(fout, "ADD "); break;
      case OPC_SUB : fprintf(fout, "SUB "); break;
      case OPC_OLD_ANDL : fprintf(fout, "ANDL "); break;
      case OPC_OLD_ORL : fprintf(fout, "ORL "); break;
      case OPC_OLD_EORL : fprintf(fout, "EORL "); break;
      case OPC_AND : fprintf(fout, "ANDB "); break;
      case OPC_OR : fprintf(fout, "ORB "); break;
      case OPC_XOR : fprintf(fout, "EORB "); break;
      case OPC_MUL : fprintf(fout, "MUL "); break;
      case OPC_DIV : fprintf(fout, "DIV "); break;
      case OPC_SLL : fprintf(fout, "SHL "); break;
      case OPC_SRL : fprintf(fout, "SHR "); break;
      case OPC_OLD_ROTL : fprintf(fout, "ROTL "); break;
      case OPC_OLD_ROTR : fprintf(fout, "ROTR "); break;
      case OPC_OLD_NEG : fprintf(fout, "NEG "); break;
      case OPC_OLD_SPCL : fprintf(fout, "SPCL "); break;
      case OPC_ADDI : fprintf(fout, "ADDI "); break;
      case OPC_SUBI : fprintf(fout, "SUBI "); break;
      case OPC_OLD_ANDLI : fprintf(fout, "ANDLI "); break;
      case OPC_OLD_ORLI : fprintf(fout, "ORLI "); break;
      case OPC_OLD_EORLI : fprintf(fout, "EORLI "); break;
      case OPC_ANDI : fprintf(fout, "ANDBI "); break;
      case OPC_ORI : fprintf(fout, "ORBI "); break;
      case OPC_XORI : fprintf(fout, "EORBI "); break;
      case OPC_MULI : fprintf(fout, "MULI "); break;
      case OPC_DIVI : fprintf(fout, "DIVI "); break;
      case OPC_SLLI : fprintf(fout, "SHLI "); break;
      case OPC_SRLI : fprintf(fout, "SHRI "); break;
      case OPC_OLD_ROTLI : fprintf(fout, "ROTLI "); break;
      case OPC_OLD_ROTRI : fprintf(fout, "ROTRI "); break;
      case OPC_OLD_NOTL : fprintf(fout, "NOTL "); break;
      case OPC_OLD_NOTB : fprintf(fout, "NOTB "); break;
      case OPC_NOP : fprintf(fout, "NOP "); break;
      case OPC_OLD_MOVA : fprintf(fout, "MOVA "); break;
      case OPC_OLD_JSR : fprintf(fout, "JSR "); break;
      case OPC_OLD_RET : fprintf(fout, "RET "); break;
      case OPC_HALT : fprintf(fout, "HALT "); break;
      case OPC_OLD_SEQ : fprintf(fout, "SEQ "); break;
      case OPC_OLD_SGE : fprintf(fout, "SGE "); break;
      case OPC_OLD_SGT : fprintf(fout, "SGT "); break;
      case OPC_OLD_SLE : fprintf(fout, "SLE "); break;
      case OPC_OLD_SLT : fprintf(fout, "SLT "); break;
      case OPC_OLD_SNE : fprintf(fout, "SNE "); break;
      case OPC_OLD_BT : fprintf(fout, "BT "); break;
      case OPC_OLD_BF : fprintf(fout, "BF "); break;
      case OPC_OLD_BHI : fprintf(fout, "BHI "); break;
      case OPC_OLD_BLS : fprintf(fout, "BLS "); break;
      case OPC_OLD_BCC : fprintf(fout, "BCC "); break;
      case OPC_OLD_BCS : fprintf(fout, "BCS "); break;
      case OPC_OLD_BNE : fprintf(fout, "BNE "); break;
      case OPC_OLD_BEQ : fprintf(fout, "BEQ "); break;
      case OPC_OLD_BVC : fprintf(fout, "BVC "); break;
      case OPC_OLD_BVS : fprintf(fout, "BVS "); break;
      case OPC_OLD_BPL : fprintf(fout, "BPL "); break;
      case OPC_OLD_BMI : fprintf(fout, "BMI "); break;
      case OPC_OLD_BGE : fprintf(fout, "BGE "); break;
      case OPC_OLD_BLT : fprintf(fout, "BLT "); break;
      case OPC_OLD_BGT : fprintf(fout, "BGT "); break;
      case OPC_OLD_BLE : fprintf(fout, "BLE "); break;
      case OPC_OLD_LOAD : fprintf(fout, "LOAD "); break;
      case OPC_OLD_STORE : fprintf(fout, "STORE "); break;
      case OPC_AXE_READ : fprintf(fout, "READ "); break;
      case OPC_AXE_WRITE : fprintf(fout, "WRITE "); break;
      case OPC_INVALID : fprintf(fout, "[INVALID] ");
   }

   if (instr->reg_dest != NULL)
   {
      if (!(instr->reg_dest)->indirect)
         fprintf(fout, "R%d ", (instr->reg_dest)->ID);
      else
         fprintf(fout, "(R%d) ", (instr->reg_dest)->ID);
   }
   if (instr->reg_src1 != NULL)
   {
      if (!(instr->reg_src1)->indirect)
         fprintf(fout, "R%d ", (instr->reg_src1)->ID);
      else
         fprintf(fout, "(R%d) ", (instr->reg_src1)->ID);
      if (instr->reg_src2 != NULL)
      {
         if (!(instr->reg_src2)->indirect)
            fprintf(fout, "R%d ", (instr->reg_src2)->ID);
         else
            fprintf(fout, "(R%d) ", (instr->reg_src2)->ID);
      }
      else
         fprintf(fout, "#%d ", instr->immediate);
   }
   
   if (instr->address != NULL)
   {
      if ((instr->address)->type == LABEL_TYPE)
         printLabel(instr->address->labelID, 1, fout);
      else
         fprintf(fout, "%d", (instr->address)->addr);
   }

   if (instr->user_comment) {
      printFormPadding(formBegin, INSTR_WIDTH, fout);
      fprintf(fout, "/* %s */", instr->user_comment);
   }
}

char * dataTypeToString(int codedType)
{
   switch (codedType)
   {
      case INTEGER_TYPE : return "INTEGER";
      default : return "<INVALID_TYPE>";
   }
}

void printLabel(t_axe_label *label, int printInline, FILE *fout)
{
   if (printInline) {
      if (!label->name)
         fprintf(fout, "L%d", label->labelID);
      else
         fprintf(fout, "%s", label->name);
   } else {
      if (!label->name)
         fprintf(fout, "L%d", label->labelID);
      else
         fprintf(fout, "%s (ID=%d)", label->name, label->labelID);
   }
}

off_t printFormPadding(off_t formBegin, int formSize, FILE *fout)
{
   off_t currentLoc = ftello(fout);
   off_t padding = formSize - (currentLoc - formBegin);
   if (padding > 1) {
      off_t i;
      for (i = 0; i < padding - 1; i++) {
         putc(' ', fout);
      }
   }
   putc(' ', fout);
   return ftello(fout);
}
