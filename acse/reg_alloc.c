/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 *
 * reg_alloc.c
 * Formal Languages & Compilers Machine, 2007/2008
 *
 */

#include <assert.h>
#include "reg_alloc.h"
#include "target_info.h"
#include "errors.h"
#include "utils.h"
#include "gencode.h"
#include "engine.h"
#include "list.h"
#include "cflow_graph.h"
#include "io_manager.h"
#include "target_asm_print.h"
#include "errors.h"

extern t_io_infos *file_infos;

#define MAX_INSTR_ARGS 3

/* constants */
#define RA_SPILL_REQUIRED -1
#define RA_REGISTER_INVALID 0
#define RA_EXCLUDED_VARIABLE 0


typedef struct t_live_interval {
   int varID;                /* a variable identifier */
   t_list *mcRegConstraints; /* list of all registers where this variable can
                              * be allocated. */
   int startPoint;           /* the index of the first instruction
                              * that make use of (or define) this variable */
   int endPoint;             /* the index of the last instruction
                              * that make use of (or define) this variable */
} t_live_interval;

typedef struct t_reg_allocator {
   t_list *live_intervals; /* an ordered list of live intervals */
   int varNum;             /* number of variables */
   int *bindings;          /* an array of bindings of kind : varID-->register.
                            * If a certain variable X need to be spilled
                            * in memory, the value of `register' is set
                            * to the value of the macro RA_SPILL_REQUIRED */
   t_list *freeRegisters;  /* a list of free registers */
} t_reg_allocator;

typedef struct t_tempLabel {
   t_axe_label *label;
   int regID;
} t_tempLabel;

/* Structure representing the current state of an instruction argument during
 * the spill materialization process */
typedef struct t_spillInstrArgState {
   /* The register argument structure */
   t_axe_register *reg;
   /* If the register is a destination register */
   int isDestination;
   /* The spill register index where the argument will be materialized, or
    * -1 otherwise. */
   int spillSlot;
} t_spillInstrRegState;

/* Structure representing the current state of a spill-reserved register */
typedef struct t_spillRegState {
   /* virtual register ID associated to this spill register */
   int assignedVar;
   /* non-zero if at least one of the instructions wrote something new into
    * the spill register, and the value has not been written to the spill
    * memory location yet. */
   int needsWB;
} t_spillRegState;

/* spill register slots */
typedef struct t_spillState {
   /* each array element corresponds to one of the registers reserved for
    * the spill, ordered by ascending register number. */
   t_spillRegState regs[NUM_SPILL_REGS];
} t_spillState;


/*
 * Register allocation algorithm
 */

/* Allocate and initialize a live interval data structure with a given varID,
 * starting and ending points */
t_live_interval *allocLiveInterval(
      int varID, t_list *mcRegs, int startPoint, int endPoint, int *error)
{
   t_live_interval *result;

   /* create a new instance of `t_live_interval' */
   result = malloc(sizeof(t_live_interval));
   if (result == NULL) {
      *error = AXE_OUT_OF_MEMORY;
      return NULL;
   }

   /* initialize the new instance */
   result->varID = varID;
   result->mcRegConstraints = cloneList(mcRegs);
   result->startPoint = startPoint;
   result->endPoint = endPoint;

   /* return the new `t_live_interval' */
   return result;
}

/* Deallocate a live interval */
void finalizeLiveInterval(t_live_interval *interval)
{
   if (interval == NULL)
      return;

   /* finalize the current interval */
   free(interval->mcRegConstraints);
   free(interval);
}

/* Given two live intervals, compare them by the start point (find whichever
 * starts first) */
int compareStartPoints(void *varA, void *varB)
{
   t_live_interval *liA = (t_live_interval *)varA;
   t_live_interval *liB = (t_live_interval *)varB;

   if (varA == NULL)
      return 0;
   if (varB == NULL)
      return 0;

   return liA->startPoint - liB->startPoint;
}

/* Given two live intervals, compare them by the end point (find whichever
 * ends first) */
int compareEndPoints(void *varA, void *varB)
{
   t_live_interval *liA = (t_live_interval *)varA;
   t_live_interval *liB = (t_live_interval *)varB;

   if (varA == NULL)
      return 0;
   if (varB == NULL)
      return 0;

   return liA->endPoint - liB->endPoint;
}

/* Given two live intervals, check if they refer to the same interval */
int compareIntervalIDs(void *varA, void *varB)
{
   t_live_interval *liA = (t_live_interval *)varA;
   t_live_interval *liB = (t_live_interval *)varB;

   if (varA == NULL)
      return 0;
   if (varB == NULL)
      return 0;

   return liA->varID == liB->varID;
}

/* Update the liveness interval for the variable 'id', used or defined
 * at position 'counter'. Returns an error code. */
int updateVarInterval(t_cflow_var *var, int counter, t_list **intervals)
{
   t_list *element_found;
   t_live_interval *interval_found;
   t_live_interval pattern;
   int error;

   if (var->ID == RA_EXCLUDED_VARIABLE || var->ID == VAR_PSW)
      return AXE_OK;

   pattern.varID = var->ID;
   /* search for the current live interval */
   element_found =
         findElementWithCallback(*intervals, &pattern, compareIntervalIDs);
   if (element_found != NULL) {
      interval_found = (t_live_interval *)LDATA(element_found);
      /* update the interval informations */
      if (interval_found->startPoint > counter)
         interval_found->startPoint = counter;
      if (interval_found->endPoint < counter)
         interval_found->endPoint = counter;
   } else {
      /* we have to add a new live interval */
      interval_found =
            allocLiveInterval(var->ID, var->mcRegWhitelist, counter, counter, &error);
      if (interval_found == NULL)
         return error;
      *intervals = addElement(*intervals, interval_found, -1);
   }

   return AXE_OK;
}

/* Use liveness information to update the list of live intervals. Returns an
 * error code. */
int updateListOfIntervals(
      t_list **result, t_cflow_Node *current_node, int counter)
{
   t_list *current_element;
   t_cflow_var *current_var;
   int i, error;

   if (current_node == NULL)
      return AXE_OK;

   current_element = current_node->in;
   while (current_element != NULL) {
      current_var = (t_cflow_var *)LDATA(current_element);

      error = updateVarInterval(current_var, counter, result);
      if (error != AXE_OK)
         return error;

      /* fetch the next element in the list of live variables */
      current_element = LNEXT(current_element);
   }

   current_element = current_node->out;
   while (current_element != NULL) {
      current_var = (t_cflow_var *)LDATA(current_element);

      error = updateVarInterval(current_var, counter, result);
      if (error != AXE_OK)
         return error;

      /* fetch the next element in the list of live variables */
      current_element = LNEXT(current_element);
   }

   for (i = 0; i < CFLOW_MAX_DEFS; i++) {
      if (current_node->defs[i]) {
         error = updateVarInterval(current_node->defs[i], counter, result);
         if (error != AXE_OK)
            return error;
      }
   }

   return AXE_OK;
}

int getLiveIntervalsNodeCallback(
      t_basic_block *block, t_cflow_Node *node, int nodeIndex, void *context)
{
   t_list **list = (t_list **)context;
   return updateListOfIntervals(list, node, nodeIndex);
}

void deleteLiveIntervals(t_list *intervals)
{
   t_list *current_element;
   t_live_interval *current_interval;

   if (intervals == NULL)
      return;

   /* finalize the memory blocks associated with all
    * the live intervals */
   for (current_element = intervals; current_element != NULL;
         current_element = LNEXT(current_element)) {
      /* fetch the current interval */
      current_interval = (t_live_interval *)LDATA(current_element);
      if (current_interval != NULL) {
         /* finalize the memory block associated with
          * the current interval */
         finalizeLiveInterval(current_interval);
      }
   }

   /* deallocate the list of intervals */
   freeList(intervals);
}

/* Perform live intervals computation. Returns an error code. */
int getLiveIntervals(t_cflow_Graph *graph, t_list **result)
{
   int error;

   /* initialize the result list */
   *result = NULL;

   /* build the list of intervals one instruction at a time */
   error = iterateCFGNodes(graph, (void *)result, getLiveIntervalsNodeCallback);
   if (error != AXE_OK) {
      deleteLiveIntervals(*result);
      *result = NULL;
      return error;
   }

   return AXE_OK;
}

/* Insert a live interval in the register allocator. Returns 0 on success,
 * -1 if the interval was already inserted in the list. */
int insertLiveInterval(t_reg_allocator *RA, t_live_interval *interval)
{
   /* test the preconditions */
   assert(RA != NULL);
   assert(interval != NULL);

   /* test if an interval for the requested variable is already inserted */
   if (findElementWithCallback(
             RA->live_intervals, interval, compareIntervalIDs) != NULL) {
      return -1;
   }

   /* add the given interval to the list of intervals,
    * in order of starting point */
   RA->live_intervals =
         addSorted(RA->live_intervals, interval, compareStartPoints);

   return 0;
}

/* Insert all elements of the intervals list into
 * the register allocator data structure. */
void insertListOfIntervals(t_reg_allocator *RA, t_list *intervals)
{
   t_list *current_element;
   t_live_interval *interval;
   int ra_errorcode;

   /* preconditions */
   assert(RA != NULL);
   assert(intervals != NULL);

   for (current_element = intervals; current_element != NULL;
         current_element = LNEXT(current_element)) {
      /* Get the current live interval */
      interval = (t_live_interval *)LDATA(current_element);
      assert(interval != NULL);

      /* insert a new live interval */
      ra_errorcode = insertLiveInterval(RA, interval);
      assert(ra_errorcode == 0 && "bug: at least one duplicate live interval");
   }
}

t_list *subtractRegisterSets(t_list *a, t_list *b)
{
   for (; b; b = LNEXT(b)) {
      a = removeElement(a, LDATA(b));
   }
   return a;
}

/* Move the elements in list `a` which are also contained in list `b` to the
 * front of the list. */
t_list *optimizeRegisterSet(t_list *a, t_list *b)
{
   for (; b; b = LNEXT(b)) {
      t_list *old;
      if ((old = findElement(a, LDATA(b)))) {
         a = removeElementLink(a, old);
         a = addElement(a, LDATA(b), 0);
      }
   }
   return a;
}

void initializeRegisterConstraints(t_reg_allocator *ra)
{
   t_list *i, *j;

   /* Initialize the register constraint set on all variables that don't have
    * one. */
   i = ra->live_intervals;
   for (; i; i = LNEXT(i)) {
      t_live_interval *interval = LDATA(i);
      if (interval->mcRegConstraints)
         continue;
      interval->mcRegConstraints = getListOfGenPurposeRegisters();

      /* Scan the variables that are alive together with this variable */
      j = LNEXT(i);
      for (; j; j = LNEXT(j)) {
         t_live_interval *overlappingIval = LDATA(j);
         if (overlappingIval->startPoint > interval->endPoint)
            break;
         if (!overlappingIval->mcRegConstraints)
            continue;
         if (overlappingIval->startPoint == interval->endPoint) {
            /* An instruction is using interval as a source and overlappingIval
             * as a destination. Optimize the constraint order to allow
             * allocating source and destination to the same register
             * if possible. */
            interval->mcRegConstraints =
                  optimizeRegisterSet(interval->mcRegConstraints,
                        overlappingIval->mcRegConstraints);
         } else {
            /* Another variable (defined after this one) wants to be allocated
             * to a restricted set of registers. Punch a hole in the current
             * variable's set of allowed registers to ensure that this is
             * possible. */
            interval->mcRegConstraints =
                  subtractRegisterSets(interval->mcRegConstraints,
                        overlappingIval->mcRegConstraints);
         }
      }
   }
}

int handleCallerSaveRegistersNodeCallback(
      t_basic_block *block, t_cflow_Node *node, int nodeIndex, void *context)
{
   t_reg_allocator *ra = (t_reg_allocator *)context;
   t_list *li_ival;
   t_list *clobbered_regs;
   int i;

   if (!isCallInstruction(node->instr))
      return 0;

   clobbered_regs = getListOfCallerSaveRegisters();
   for (i = 0; i < CFLOW_MAX_DEFS; i++) {
      if (node->defs[i] != NULL)
         clobbered_regs = subtractRegisterSets(
               clobbered_regs, node->defs[i]->mcRegWhitelist);
   }
   for (i = 0; i < CFLOW_MAX_USES; i++) {
      if (node->uses[i] != NULL)
         clobbered_regs = subtractRegisterSets(
               clobbered_regs, node->uses[i]->mcRegWhitelist);
   }

   li_ival = ra->live_intervals;
   while (li_ival) {
      t_live_interval *ival = LDATA(li_ival);

      if (ival->startPoint <= nodeIndex && nodeIndex <= ival->endPoint) {
         ival->mcRegConstraints =
               subtractRegisterSets(ival->mcRegConstraints, clobbered_regs);
      }

      li_ival = LNEXT(li_ival);
   }

   return 0;
}

/* Restrict register constraints in order to avoid register corrupted by
 * function calls. */
void handleCallerSaveRegisters(t_reg_allocator *ra, t_cflow_Graph *cfg)
{
   iterateCFGNodes(cfg, (void *)ra, handleCallerSaveRegistersNodeCallback);
}

int compareFreeRegLI(void *freeReg, void *constraintReg)
{
   return INTDATA(constraintReg) == INTDATA(freeReg);
}

/* Get a new register from the free list */
int assignRegister(t_reg_allocator *RA, t_list *constraints)
{
   int regID;
   t_list *i;

   if (constraints == NULL)
      return RA_SPILL_REQUIRED;

   for (i = constraints; i; i = LNEXT(i)) {
      t_list *freeReg;

      regID = LINTDATA(i);
      freeReg = findElementWithCallback(
            RA->freeRegisters, INTDATA(regID), compareFreeRegLI);
      if (freeReg) {
         RA->freeRegisters = removeElementLink(RA->freeRegisters, freeReg);
         return regID;
      }
   }

   return RA_SPILL_REQUIRED;
}

/* Perform a spill that allows the allocation of the given
 * interval, given the list of active live intervals */
t_list *spillAtInterval(
      t_reg_allocator *RA, t_list *active_intervals, t_live_interval *interval)
{
   t_list *last_element;
   t_live_interval *last_interval;

   /* get the last element of the list of active intervals */
   /* Precondition: if the list of active intervals is empty
    * we are working on a machine with 0 registers available
    * for the register allocation */
   if (active_intervals == NULL) {
      RA->bindings[interval->varID] = RA_SPILL_REQUIRED;
      return active_intervals;
   }

   last_element = getLastElement(active_intervals);
   last_interval = (t_live_interval *)LDATA(last_element);

   /* If the current interval ends before the last one, spill
    * the last one, otherwise spill the current interval. */
   if (last_interval->endPoint > interval->endPoint) {
      int attempt = RA->bindings[last_interval->varID];
      if (findElement(interval->mcRegConstraints, INTDATA(attempt))) {
         RA->bindings[interval->varID] = RA->bindings[last_interval->varID];
         RA->bindings[last_interval->varID] = RA_SPILL_REQUIRED;

         active_intervals = removeElement(active_intervals, last_interval);

         active_intervals =
               addSorted(active_intervals, interval, compareEndPoints);
         return active_intervals;
      }
   }

   RA->bindings[interval->varID] = RA_SPILL_REQUIRED;
   return active_intervals;
}

/* Remove from active_intervals all the live intervals that end before the
 * beginning of the current live interval */
t_list *expireOldIntervals(
      t_reg_allocator *RA, t_list *active_intervals, t_live_interval *interval)
{
   t_list *current_element;
   t_list *next_element;
   t_live_interval *current_interval;

   assert(RA != NULL);
   assert(interval != NULL);

   /* No active intervals, bail out! */
   if (active_intervals == NULL)
      return NULL;

   /* Iterate over the set of active intervals */
   current_element = active_intervals;
   while (current_element != NULL) {
      /* Get the live interval */
      current_interval = (t_live_interval *)LDATA(current_element);

      /* If the considered interval ends before the beginning of
       * the current live interval, we don't need to keep track of
       * it anymore; otherwise, this is the first interval we must
       * still take into account when assigning registers. */
      if (current_interval->endPoint > interval->startPoint)
         return active_intervals;

      /* when current_interval->endPoint == interval->startPoint,
       * the variable associated to current_interval is being used by the
       * instruction that defines interval. As a result, we can allocate
       * interval to the same reg as current_interval. */
      if (current_interval->endPoint == interval->startPoint) {
         int curIntReg = RA->bindings[current_interval->varID];
         if (curIntReg >= 0) {
            t_list *allocated = addElement(NULL, INTDATA(curIntReg), 0);
            interval->mcRegConstraints =
                  optimizeRegisterSet(interval->mcRegConstraints, allocated);
            freeList(allocated);
         }
      }

      /* Get the next live interval */
      next_element = LNEXT(current_element);

      /* Remove the current element from the list */
      active_intervals = removeElement(active_intervals, current_interval);

      /* Free all the registers associated with the removed interval */
      RA->freeRegisters = addElement(RA->freeRegisters,
            INTDATA(RA->bindings[current_interval->varID]), 0);

      /* Step to the next interval */
      current_element = next_element;
   }

   /* Return the updated list of active intervals */
   return active_intervals;
}

/* Deallocate the register allocator data structures */
void finalizeRegAlloc(t_reg_allocator *RA)
{
   if (RA == NULL)
      return;

   /* Free the live intervals */
   deleteLiveIntervals(RA->live_intervals);

   /* Free memory used for the variable/register bindings */
   free(RA->bindings);
   freeList(RA->freeRegisters);

   free(RA);
}

/* Allocate and initialize the register allocator */
t_reg_allocator *initializeRegAlloc(t_cflow_Graph *graph, int *error)
{
   t_reg_allocator *result; /* the register allocator */
   t_list *intervals;
   t_list *current_cflow_var;
   t_cflow_var *cflow_var;
   int max_var_ID;
   int counter;
   int error2;

   /* Check preconditions: the cfg must exist */
   assert(graph != NULL);

   /* allocate memory for a new instance of `t_reg_allocator' */
   result = (t_reg_allocator *)calloc(1, sizeof(t_reg_allocator));
   if (result == NULL) {
      if (error)
         *error = AXE_OUT_OF_MEMORY;
      return NULL;
   }

   /* initialize the register allocator informations */

   /* retrieve the max identifier from each live interval */
   max_var_ID = 0;
   current_cflow_var = graph->cflow_variables;
   while (current_cflow_var != NULL) {
      /* fetch the data informations about a variable */
      cflow_var = (t_cflow_var *)LDATA(current_cflow_var);
      assert(cflow_var != NULL);

      /* update the value of max_var_ID */
      max_var_ID = MAX(max_var_ID, cflow_var->ID);

      /* retrieve the next variable */
      current_cflow_var = LNEXT(current_cflow_var);
   }
   result->varNum = max_var_ID + 1; /* +1 to count R0 */

   /* Assuming there are some variables to associate to regs,
    * allocate space for the binding array, and initialize it */

   /*alloc memory for the array of bindings */
   result->bindings = (int *)malloc(sizeof(int) * result->varNum);
   if (result->bindings == NULL) {
      finalizeRegAlloc(result);
      if (error)
         *error = AXE_OUT_OF_MEMORY;
      return NULL;
   }

   /* initialize the array of bindings */
   for (counter = 0; counter < result->varNum; counter++)
      result->bindings[counter] = RA_REGISTER_INVALID;

   /* Liveness analysis: compute the list of live intervals */
   error2 = getLiveIntervals(graph, &intervals);
   if (error2 != AXE_OK) {
      finalizeRegAlloc(result);
      if (error)
         *error = error2;
      return NULL;
   }

   /* Copy the liveness info into the register allocator, sorting it in the
    * process. */
   if (intervals != NULL) {
      insertListOfIntervals(result, intervals);
      /* deallocate memory used to hold the results of the
       * liveness analysis */
      freeList(intervals);
   } else {
      result->live_intervals = NULL;
   }

   /* create a list of freeRegisters */
   result->freeRegisters = getListOfGenPurposeRegisters();

   /* Initialize register constraints */
   initializeRegisterConstraints(result);
   handleCallerSaveRegisters(result, graph);

   /* return the new register allocator */
   return result;
}

/* Main register allocation function */
void executeLinearScan(t_reg_allocator *RA)
{
   t_list *current_element;
   t_live_interval *current_interval;
   t_list *active_intervals;
   int reg;

   /* test the preconditions */
   assert(RA != NULL);
   /* Liveness analysis ready? */
   if (RA->live_intervals == NULL)
      return;

   /* initialize the list of active intervals */
   active_intervals = NULL;

   /* Iterate over the list of live intervals */
   for (current_element = RA->live_intervals; current_element != NULL;
         current_element = LNEXT(current_element)) {
      /* Get the live interval */
      current_interval = (t_live_interval *)LDATA(current_element);

      /* Check which intervals are ended and remove
       * them from the active set, thus freeing registers */
      active_intervals =
            expireOldIntervals(RA, active_intervals, current_interval);

      reg = assignRegister(RA, current_interval->mcRegConstraints);

      /* If all registers are busy, perform a spill */
      if (reg == RA_SPILL_REQUIRED) {
         /* perform a spill */
         active_intervals =
               spillAtInterval(RA, active_intervals, current_interval);
      } else {
         /* Otherwise, assign a new register to the current live interval */
         RA->bindings[current_interval->varID] = reg;

         /* Add the current interval to the list of active intervals, in
          * order of ending points (to allow easier expire management) */
         active_intervals =
               addSorted(active_intervals, current_interval, compareEndPoints);
      }
   }

   /* free the list of active intervals */
   freeList(active_intervals);
}


/*
 * Materialization
 */

int compareTempLabels(void *valA, void *valB)
{
   t_tempLabel *tlA = (t_tempLabel *)valA;
   t_tempLabel *tlB = (t_tempLabel *)valB;

   if (valA == NULL)
      return 0;
   if (valB == NULL)
      return 0;

   return tlA->regID == tlB->regID;
}

t_tempLabel *allocTempLabel(t_axe_label *label, int regID, int *error)
{
   t_tempLabel *result;

   /* create a new temp-label */
   result = malloc(sizeof(t_tempLabel));
   if (result == NULL) {
      if (error)
         *error = AXE_OUT_OF_MEMORY;
      return NULL;
   }

   /* initialize the temp label */
   result->label = label;
   result->regID = regID;
   return result;
}

void finalizeListOfTempLabels(t_list *tempLabels)
{
   t_list *current_element;
   t_tempLabel *tempLabel;

   /* test the preconditions */
   if (tempLabels == NULL)
      return;

   /* free all the list data elements */
   current_element = tempLabels;
   while (current_element != NULL) {
      tempLabel = (t_tempLabel *)LDATA(current_element);
      free(tempLabel);

      current_element = LNEXT(current_element);
   }

   /* free the list links */
   freeList(tempLabels);
}

/* For each spilled variable, this function statically allocates memory for
 * that variable, returns a list of t_templabel structures mapping the
 * spilled variables and the label that points to the allocated memory block.
 * Returns an error code. */
int materializeSpillMemory(t_program_infos *program, t_reg_allocator *RA, t_list **out)
{
   int counter, error;
   t_list *result;
   t_tempLabel *tlabel;
   t_axe_label *axe_label;
   t_axe_data *new_data_info;

   /* preconditions */
   assert(program != NULL);
   assert(RA != NULL);

   /* initialize the local variable `result' */
   result = NULL;
   tlabel = NULL;

   /* allocate some memory for all spilled temporary variables */
   for (counter = 0; counter < RA->varNum; counter++) {
      if (RA->bindings[counter] != RA_SPILL_REQUIRED)
         continue;

      /* retrieve a new label */
      axe_label = newLabel(program);
      if (axe_label == NULL) {
         finalizeListOfTempLabels(result);
         return AXE_OUT_OF_MEMORY;
      }

      /* create a new tempLabel */
      tlabel = allocTempLabel(axe_label, counter, &error);
      if (tlabel == NULL) {
         finalizeListOfTempLabels(result);
         return error;
      }

      /* statically allocate some room for the spilled variable by
       * creating a new .WORD directive and making the label point to it. */
      new_data_info = initializeData(DIR_WORD, 0, axe_label);
      if (new_data_info == NULL) {
         finalizeListOfTempLabels(result);
         return error;
      }

      /* update the list of directives */
      program->data = addElement(program->data, new_data_info, -1);

      /* add the current tlabel to the list of labelbindings */
      result = addElement(result, tlabel, -1);
   }

   /* postcondition: return the list of bindings */
   *out = result;
   return AXE_OK;
}

int genStoreSpillVariable(int temp_register, int selected_register,
      t_cflow_Graph *graph, t_basic_block *current_block,
      t_cflow_Node *current_node, t_list *labelBindings, int before)
{
   t_axe_instruction *storeInstr;
   t_cflow_Node *storeNode = NULL;
   t_list *elementFound;
   t_tempLabel pattern;
   t_tempLabel *tlabel;
   int error;

   pattern.regID = temp_register;
   elementFound =
         findElementWithCallback(labelBindings, &pattern, compareTempLabels);

   if (elementFound == NULL)
      return AXE_INVALID_CFLOW_GRAPH;

   tlabel = (t_tempLabel *)LDATA(elementFound);
   assert(tlabel != NULL);

   /* create a store instruction */
   storeInstr = genSWGlobalInstruction(NULL, selected_register, tlabel->label);
   if (storeInstr == NULL)
      return AXE_OUT_OF_MEMORY;

   /* create a node for the load instruction */
   storeNode = allocNode(graph, storeInstr, &error);
   if (storeNode == NULL)
      return error;

   /* test if we have to insert the node `storeNode' before `current_node'
    * inside the basic block */
   if (before) {
      insertNodeBefore(current_block, current_node, storeNode);
   } else {
      insertNodeAfter(current_block, current_node, storeNode);
   }

   return AXE_OK;
}

int genLoadSpillVariable(int temp_register, int selected_register,
      t_cflow_Graph *graph, t_basic_block *block, t_cflow_Node *current_node,
      t_list *labelBindings, int before)
{
   t_axe_instruction *loadInstr;
   t_cflow_Node *loadNode = NULL;
   t_list *elementFound;
   t_tempLabel pattern;
   t_tempLabel *tlabel;
   int error;

   pattern.regID = temp_register;
   elementFound =
         findElementWithCallback(labelBindings, &pattern, compareTempLabels);

   if (elementFound == NULL)
      return AXE_INVALID_CFLOW_GRAPH;

   tlabel = (t_tempLabel *)LDATA(elementFound);
   assert(tlabel != NULL);

   /* create a load instruction */
   loadInstr = genLWGlobalInstruction(NULL, selected_register, tlabel->label);
   if (loadInstr == NULL)
      return AXE_OUT_OF_MEMORY;

   /* create a node for the load instruction */
   loadNode = allocNode(graph, loadInstr, &error);
   if (loadNode == NULL)
      return error;

   if (before) {
      /* insert the node `loadNode' before `current_node' */
      insertNodeBefore(block, current_node, loadNode);
      /* if the `current_node' instruction has a label, move it to the new
       * load instruction */
      if ((current_node->instr)->label != NULL) {
         loadInstr->label = (current_node->instr)->label;
         (current_node->instr)->label = NULL;
      }
   } else {
      insertNodeAfter(block, current_node, loadNode);
   }

   return AXE_OK;
}

int materializeRegAllocInBBForInstructionNode(t_cflow_Graph *graph,
      t_basic_block *current_block, t_spillState *state,
      t_cflow_Node *current_node, t_reg_allocator *RA, t_list *label_bindings)
{
   int error;
   t_axe_instruction *instr;
   int current_arg, current_row, num_args;
   /* The elements in this array indicate whether the corresponding spill
    * register will be used or not by this instruction */
   int spillSlotInUse[NUM_SPILL_REGS] = {0};
   /* This array stores whether each argument of the instruction is allocated
    * to a spill register or not.
    * For example, if argState[1].spillSlot == 2, the argState[1].reg register
    * will be materialized to the third spill register. */
   t_spillInstrRegState argState[MAX_INSTR_ARGS];

   /* fetch the current instruction */
   instr = current_node->instr;

   /* initialize the array of arguments to the instruction */
   num_args = 0;
   if (instr->reg_dest) {
      argState[num_args].reg = instr->reg_dest;
      argState[num_args].isDestination = !instr->reg_dest->indirect;
      argState[num_args].spillSlot = -1;
      num_args++;
   }
   if (instr->reg_src1) {
      argState[num_args].reg = instr->reg_src1;
      argState[num_args].isDestination = 0;
      argState[num_args].spillSlot = -1;
      num_args++;
   }
   if (instr->reg_src2) {
      argState[num_args].reg = instr->reg_src2;
      argState[num_args].isDestination = 0;
      argState[num_args].spillSlot = -1;
      num_args++;
   }

   /* Test if a requested variable is already loaded into a register
    * from a previous instruction. */
   for (current_arg = 0; current_arg < num_args; current_arg++) {
      if (RA->bindings[argState[current_arg].reg->ID] != RA_SPILL_REQUIRED)
         continue;

      for (current_row = 0; current_row < NUM_SPILL_REGS; current_row++) {
         if (state->regs[current_row].assignedVar !=
               argState[current_arg].reg->ID)
            continue;

         /* update the value of used_Register */
         argState[current_arg].spillSlot = current_row;

         /* update the value of `assignedRegisters` */
         /* set currently used flag */
         spillSlotInUse[current_row] = 1;

         /* test if a write back is needed. Writebacks are needed
          * when an instruction modifies a spilled register. */
         if (argState[current_arg].isDestination)
            state->regs[current_row].needsWB = 1;

         /* a slot was found, stop searching */
         break;
      }
   }

   /* Find a slot for all other variables. Write back the variable associated
    * with the slot if necessary. */
   for (current_arg = 0; current_arg < num_args; current_arg++) {
      int other_arg, alreadyFound;

      if (RA->bindings[argState[current_arg].reg->ID] != RA_SPILL_REQUIRED)
         continue;
      if (argState[current_arg].spillSlot != -1)
         continue;

      /* Check if we already have found a slot for this variable */
      alreadyFound = 0;
      for (other_arg = 0; other_arg < current_arg && !alreadyFound;
            other_arg++) {
         if (argState[current_arg].reg->ID == argState[other_arg].reg->ID) {
            argState[current_arg].spillSlot = argState[other_arg].spillSlot;
            alreadyFound = 1;
         }
      }
      /* No need to do anything else in this case, the state of the
       * spill slot is already up to date */
      if (alreadyFound)
         continue;

      /* Otherwise a slot one by iterating through the slots available */
      for (current_row = 0; current_row < NUM_SPILL_REGS; current_row++) {
         if (spillSlotInUse[current_row] == 0)
            break;
      }
      /* If we don't find anything, we don't have enough spill registers!
       * This should never happen, bail out! */
      if (current_row == NUM_SPILL_REGS)
         return AXE_REG_ALLOC_ERROR;

      /* If needed, write back the old variable that was assigned to this
       * slot before reassigning it */
      if (state->regs[current_row].needsWB == 1) {
         error = genStoreSpillVariable(state->regs[current_row].assignedVar,
               getSpillRegister(current_row), graph, current_block,
               current_node, label_bindings, 1);
         if (error != AXE_OK)
            return error;
      }

      /* Update the state of this spill slot */
      spillSlotInUse[current_row] = 1;
      argState[current_arg].spillSlot = current_row;
      state->regs[current_row].assignedVar = argState[current_arg].reg->ID;
      state->regs[current_row].needsWB = argState[current_arg].isDestination;

      /* Load the value of the variable in the spill register if not a
       * destination of the instruction */
      if (!argState[current_arg].isDestination) {
         error = genLoadSpillVariable(argState[current_arg].reg->ID,
               getSpillRegister(current_row), graph, current_block,
               current_node, label_bindings, 1);
         if (error != AXE_OK)
            return error;
      }
   }

   /* rewrite the register identifiers to use the appropriate
    * register number instead of the variable number. */
   for (current_arg = 0; current_arg < num_args; current_arg++) {
      t_axe_register *curReg = argState[current_arg].reg;
      if (argState[current_arg].spillSlot == -1) {
         /* normal case */
         curReg->ID = RA->bindings[curReg->ID];
      } else {
         /* spilled register case */
         curReg->ID = getSpillRegister(argState[current_arg].spillSlot);
      }
   }

   return AXE_OK;
}

int materializeRegAllocInBB(t_cflow_Graph *graph, t_basic_block *current_block,
      t_reg_allocator *RA, t_list *label_bindings)
{
   int error, counter, bbHasTermInstr;
   t_list *current_nd_element;
   t_cflow_Node *current_node;
   t_spillState state;

   assert(current_block != NULL);

   /* initialize the state for this block */
   for (counter = 0; counter < NUM_SPILL_REGS; counter++) {
      state.regs[counter].assignedVar = REG_INVALID;
      state.regs[counter].needsWB = 0;
   }

   /* iterate through the instructions in the block */
   current_nd_element = current_block->nodes;
   while (current_nd_element != NULL) {
      current_node = (t_cflow_Node *)LDATA(current_nd_element);

      /* Change the register IDs of the argument of the instruction accoring
       * to the given register allocation. Generate load and stores for spilled
       * registers */
      error = materializeRegAllocInBBForInstructionNode(
            graph, current_block, &state, current_node, RA, label_bindings);
      if (error)
         return error;

      current_nd_element = LNEXT(current_nd_element);
   }

   bbHasTermInstr = current_block->nodes &&
         (isJumpInstruction(current_node->instr) ||
               isHaltOrRetInstruction(current_node->instr));

   /* writeback everything at the end of the basic block */
   for (counter = 0; counter < NUM_SPILL_REGS; counter++) {
      if (state.regs[counter].needsWB == 0)
         continue;
      error = genStoreSpillVariable(state.regs[counter].assignedVar,
            getSpillRegister(counter), graph, current_block, current_node,
            label_bindings, bbHasTermInstr);
      if (error)
         return error;
   }

   return AXE_OK;
}

/* update the control flow informations by unsing the result
 * of the register allocation process and a list of bindings
 * between new assembly labels and spilled variables */
int materializeRegAllocInCFG(
      t_cflow_Graph *graph, t_reg_allocator *RA, t_list *label_bindings)
{
   int error;
   t_list *current_bb_element;

   /* preconditions */
   assert(graph != NULL);
   assert(RA != NULL);

   current_bb_element = graph->blocks;
   while (current_bb_element != NULL) {
      t_basic_block *current_block = (t_basic_block *)LDATA(current_bb_element);

      error = materializeRegAllocInBB(graph, current_block, RA, label_bindings);
      if (error)
         return error;

      /* retrieve the next basic block element */
      current_bb_element = LNEXT(current_bb_element);
   }

   return AXE_OK;
}

/* Replace the variable identifiers in the instructions of the CFG with the
 * register assignments in the register allocator. Materialize spilled
 * variables to the scratch registers. All new instructions are inserted
 * in the CFG. Synchronize the list of instructions with the newly
 * modified program. */
int materializeRegisterAllocation(
      t_program_infos *program, t_cflow_Graph *graph, t_reg_allocator *RA)
{
   t_list *label_bindings;
   int error;

   /* retrieve a list of t_templabels for the given RA infos and
    * update the content of the data segment. */
   error = materializeSpillMemory(program, RA, &label_bindings);
   if (error != AXE_OK)
      return error;

   /* update the control flow graph with the reg-alloc infos. */
   error = materializeRegAllocInCFG(graph, RA, label_bindings);
   finalizeListOfTempLabels(label_bindings);
   if (error != AXE_OK)
      return error;

   /* update the code segment informations */
   updateProgramFromCFG(program, graph);

   return AXE_OK;
}


/*
 *  Debug print utilities
 */

void printBindings(int *bindings, int numVars, FILE *fout)
{
   int counter;

   if (bindings == NULL)
      return;
   if (fout == NULL)
      return;

   for (counter = 1; counter < numVars; counter++) {
      if (bindings[counter] == RA_SPILL_REQUIRED) {
         fprintf(fout, "Variable T%-3d will be spilled\n", counter);
      } else if (bindings[counter] == RA_REGISTER_INVALID) {
         fprintf(fout, "Variable T%-3d has not been assigned to any register\n",
               counter);
      } else {
         char *reg = registerIDToString(bindings[counter], 1);
         fprintf(fout, "Variable T%-3d is assigned to register %s\n", counter,
               reg);
         free(reg);
      }
   }

   fflush(fout);
}

void printLiveIntervals(t_list *intervals, FILE *fout)
{
   t_list *current_element;
   t_live_interval *interval;
   t_list *i;

   /* precondition */
   if (fout == NULL)
      return;

   /* retireve the first element of the list */
   current_element = intervals;
   while (current_element != NULL) {
      interval = (t_live_interval *)LDATA(current_element);

      fprintf(fout, "[T%-3d] Live interval: [%3d, %3d]\n", interval->varID,
            interval->startPoint, interval->endPoint);
      fprintf(fout, "       Constraint set: {");

      for (i=interval->mcRegConstraints; i!=NULL; i=LNEXT(i)) {
         char *reg;
         
         reg = registerIDToString(LINTDATA(i), 1);
         fprintf(fout, "%s", reg);
         free(reg);

         if (LNEXT(i) != NULL)
            fprintf(fout, ", ");
      }
      fprintf(fout, "}\n");

      /* retrieve the next element in the list of intervals */
      current_element = LNEXT(current_element);
   }
   fflush(fout);
}

void printRegAllocInfos(t_reg_allocator *RA, FILE *fout)
{
   if (RA == NULL)
      return;
   if (fout == NULL)
      return;
   
   fprintf(fout, "*************************\n");
   fprintf(fout, "   REGISTER ALLOCATION   \n");
   fprintf(fout, "*************************\n\n");

   fprintf(fout, "------------\n");
   fprintf(fout, " STATISTICS \n");
   fprintf(fout, "------------\n");
   fprintf(fout, "Number of available registers: %d\n", NUM_GP_REGS);
   fprintf(fout, "Number of used variables: %d\n\n", RA->varNum);

   fprintf(fout, "----------------\n");
   fprintf(fout, " LIVE INTERVALS \n");
   fprintf(fout, "----------------\n");
   printLiveIntervals(RA->live_intervals, fout);
   fprintf(fout, "\n");

   fprintf(fout, "----------------------------\n");
   fprintf(fout, " VARIABLE/REGISTER BINDINGS \n");
   fprintf(fout, "----------------------------\n");
   printBindings(RA->bindings, RA->varNum, fout);

   fflush(fout);
}


/*
 * Wrapper function
 */

void doRegisterAllocation(t_program_infos *program)
{
   t_cflow_Graph *graph = NULL;
   t_reg_allocator *RA = NULL;
   int error = AXE_OK;

   assert(program != NULL);
   assert(file_infos != NULL);

   /* create the control flow graph */
   graph = createFlowGraph(program, &error);
   if (graph == NULL)
      fatalError(error);
#ifndef NDEBUG
   printGraphInfos(graph, file_infos->cfg_1, 0);
#endif

   performLivenessAnalysis(graph);
#ifndef NDEBUG
   printGraphInfos(graph, file_infos->cfg_2, 1);
#endif

   /* execute the linear scan algorithm */
   RA = initializeRegAlloc(graph, &error);
   if (RA == NULL)
      fatalError(error);
   
   executeLinearScan(RA);
#ifndef NDEBUG
   printRegAllocInfos(RA, file_infos->reg_alloc_output);
#endif

   /* apply changes to the program informations by using the informations
    * of the register allocation process */
   error = materializeRegisterAllocation(program, graph, RA);
   if (error != AXE_OK)
      fatalError(error);

   finalizeRegAlloc(RA);
   finalizeGraph(graph);
}
