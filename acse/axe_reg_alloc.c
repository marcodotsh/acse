/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 *
 * axe_reg_alloc.c
 * Formal Languages & Compilers Machine, 2007/2008
 *
 */

#include <assert.h>
#include "axe_reg_alloc.h"
#include "axe_target_info.h"
#include "axe_errors.h"
#include "axe_utils.h"
#include "axe_gencode.h"
#include "axe_engine.h"
#include "collections.h"
#include "axe_cflow_graph.h"
#include "axe_io_manager.h"

extern int errorcode;
extern int cflow_errorcode;
extern t_io_infos *file_infos;

#define MAX_INSTR_ARGS 3

/* errorcodes */
#define RA_OK 0
#define RA_INVALID_ALLOCATOR 1
#define RA_INVALID_INTERVAL 2
#define RA_INTERVAL_ALREADY_INSERTED 3
#define RA_INVALID_NUMBER_OF_REGISTERS 4

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
   int regNum;             /* the number of registers of the machine */
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
      int varID, t_list *mcRegs, int startPoint, int endPoint)
{
   t_live_interval *result;

   /* create a new instance of `t_live_interval' */
   result = malloc(sizeof(t_live_interval));
   if (result == NULL)
      fatalError(AXE_OUT_OF_MEMORY);

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
 * at position 'counter'. */
t_list *updateVarInterval(t_cflow_var *var, int counter, t_list *intervals)
{
   t_list *element_found;
   t_live_interval *interval_found;
   t_live_interval pattern;

   if (var->ID == RA_EXCLUDED_VARIABLE || var->ID == VAR_PSW)
      return intervals;

   pattern.varID = var->ID;
   /* search for the current live interval */
   element_found =
         findElementWithCallback(intervals, &pattern, compareIntervalIDs);
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
            allocLiveInterval(var->ID, var->mcRegWhitelist, counter, counter);
      if (interval_found == NULL)
         fatalError(AXE_OUT_OF_MEMORY);
      intervals = addElement(intervals, interval_found, -1);
   }

   return intervals;
}

/* Use liveness information to update the list of live intervals */
t_list *updateListOfIntervals(
      t_list *result, t_cflow_Node *current_node, int counter)
{
   t_list *current_element;
   t_cflow_var *current_var;
   int i;

   if (current_node == NULL)
      return result;

   current_element = current_node->in;
   while (current_element != NULL) {
      current_var = (t_cflow_var *)LDATA(current_element);

      result = updateVarInterval(current_var, counter, result);

      /* fetch the next element in the list of live variables */
      current_element = LNEXT(current_element);
   }

   current_element = current_node->out;
   while (current_element != NULL) {
      current_var = (t_cflow_var *)LDATA(current_element);

      result = updateVarInterval(current_var, counter, result);

      /* fetch the next element in the list of live variables */
      current_element = LNEXT(current_element);
   }

   for (i = 0; i < CFLOW_MAX_DEFS; i++) {
      if (current_node->defs[i])
         result = updateVarInterval(current_node->defs[i], counter, result);
   }

   return result;
}

/* Perform live intervals computation */
t_list *getLiveIntervals(t_cflow_Graph *graph)
{
   t_list *current_bb_element;
   t_list *current_nd_element;
   t_basic_block *current_block;
   t_cflow_Node *current_node;
   t_list *result;
   int counter;

   /* preconditions */
   if (graph == NULL)
      return NULL;

   if (graph->blocks == NULL)
      return NULL;

   /* initialize the local variable `result' */
   result = NULL;

   /* intialize the instruction counter */
   counter = 0;

   /* fetch the first basic block */
   current_bb_element = graph->blocks;
   while (current_bb_element != NULL) {
      current_block = (t_basic_block *)LDATA(current_bb_element);

      /* fetch the first node of the basic block */
      current_nd_element = current_block->nodes;
      while (current_nd_element != NULL) {
         current_node = (t_cflow_Node *)LDATA(current_nd_element);

         /* update the live intervals with the liveness informations */
         result = updateListOfIntervals(result, current_node, counter);

         /* fetch the next node in the basic block */
         counter++;
         current_nd_element = LNEXT(current_nd_element);
      }

      /* fetch the next element in the list of basic blocks */
      current_bb_element = LNEXT(current_bb_element);
   }

   return result;
}

/* Insert a live interval in the register allocator */
int insertLiveInterval(t_reg_allocator *RA, t_live_interval *interval)
{
   /* test the preconditions */
   if (RA == NULL)
      return RA_INVALID_ALLOCATOR;

   if (interval == NULL)
      return RA_INVALID_INTERVAL;

   /* test if an interval for the requested variable is already inserted */
   if (findElementWithCallback(
             RA->live_intervals, interval, compareIntervalIDs) != NULL) {
      return RA_INTERVAL_ALREADY_INSERTED;
   }

   /* add the given interval to the list of intervals,
    * in order of starting point */
   RA->live_intervals =
         addSorted(RA->live_intervals, interval, compareStartPoints);

   return RA_OK;
}

/*
 * Insert all elements of the intervals list into
 * the register allocator data structure
 */
int insertListOfIntervals(t_reg_allocator *RA, t_list *intervals)
{
   t_list *current_element;
   t_live_interval *interval;
   int ra_errorcode;

   /* preconditions */
   if (RA == NULL)
      return RA_INVALID_ALLOCATOR;
   if (intervals == NULL)
      return RA_OK;

   for (current_element = intervals; current_element != NULL;
         current_element = LNEXT(current_element)) {
      /* Get the current live interval */
      interval = (t_live_interval *)LDATA(current_element);

      if (interval == NULL)
         return RA_INVALID_INTERVAL;

      /* insert a new live interval */
      ra_errorcode = insertLiveInterval(RA, interval);

      /* test if an error occurred */
      if (ra_errorcode != RA_OK)
         return ra_errorcode;
   }

   return RA_OK;
}

/* Return register regID to the free list in the given position */
t_list *addFreeRegister(t_list *registers, int regID, int position)
{
   int *element;

   /* Allocate memory space for the reg id */
   element = (int *)malloc(sizeof(int));
   if (element == NULL)
      fatalError(AXE_OUT_OF_MEMORY);

   /* initialize element */
   (*element) = regID;

   /* update the list of registers */
   registers = addElement(registers, element, position);

   /* return the list of free registers */
   return registers;
}

/* Allocate and initialize the free registers list,
 * assuming regNum general purpose registers */
t_list *allocFreeRegisters(int regNum)
{
   int count;
   t_list *result;

   /* initialize the local variables */
   count = 1;
   result = NULL;

   while (count <= regNum) {
      /* add a new register to the list of free registers */
      result = addFreeRegister(result, count, -1);

      /* update the `count' variable */
      count++;
   }

   /* return the list of free registers */
   return result;
}

int compareFreeRegLI(void *freeReg, void *constraintReg)
{
   return *(int *)constraintReg == *(int *)freeReg;
}

/* Get a new register from the free list */
int assignRegister(t_reg_allocator *RA, t_list *constraints)
{
   int regID;

   t_list *i = constraints;
   for (; i; i = LNEXT(i)) {
      t_list *freeReg;

      regID = LINTDATA(i);
      freeReg = findElementWithCallback(
            RA->freeRegisters, &regID, compareFreeRegLI);
      if (freeReg) {
         free(LDATA(freeReg));
         RA->freeRegisters = removeElementLink(RA->freeRegisters, freeReg);
         return regID;
      }
   }

   return RA_SPILL_REQUIRED;
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
   t_list *i, *j, *allregs, *allregs2;

   /* Create a list of all free registers we are allowed to use */
   allregs = NULL;
   allregs2 = ra->freeRegisters;
   for (; allregs2; allregs2 = LNEXT(allregs2)) {
      int reg = *(int *)LDATA(allregs2);
      if (!isSpecialRegister(reg))
         allregs = addElement(allregs, INTDATA(reg), -1);
   }

   /* Initialize the register constraint set on all variables that don't have
    * one. */
   i = ra->live_intervals;
   for (; i; i = LNEXT(i)) {
      t_live_interval *interval = LDATA(i);
      if (interval->mcRegConstraints)
         continue;
      interval->mcRegConstraints = cloneList(allregs);

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
      assert(interval->mcRegConstraints);
   }

   freeList(allregs);
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

   /* Check for valid register allocator and set of active intervals */
   if (active_intervals == NULL)
      return NULL;
   if (RA == NULL)
      return NULL;
   if (interval == NULL)
      return active_intervals;

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
      RA->freeRegisters = addFreeRegister(
            RA->freeRegisters, RA->bindings[current_interval->varID], 0);

      /* Step to the next interval */
      current_element = next_element;
   }

   /* Return the updated list of active intervals */
   return active_intervals;
}

/* Allocate and initialize the register allocator */
t_reg_allocator *initializeRegAlloc(t_cflow_Graph *graph)
{
   t_reg_allocator *result; /* the register allocator */
   t_list *intervals;
   t_list *current_cflow_var;
   t_cflow_var *cflow_var;
   int max_var_ID;
   int counter;

   /* Check preconditions: the cfg must exist */
   if (graph == NULL)
      fatalError(AXE_INVALID_CFLOW_GRAPH);

   /* allocate memory for a new instance of `t_reg_allocator' */
   result = (t_reg_allocator *)malloc(sizeof(t_reg_allocator));
   if (result == NULL)
      fatalError(AXE_OUT_OF_MEMORY);

   /* initialize the register allocator informations */
   /* Reserve a few registers (NUM_SPILL_REGS) to handle spills */
   result->regNum = NUM_REGISTERS;

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

   /* test if an error occurred */
   if (result->bindings == NULL)
      fatalError(AXE_OUT_OF_MEMORY);

   /* initialize the array of bindings */
   for (counter = 0; counter < result->varNum; counter++)
      result->bindings[counter] = RA_REGISTER_INVALID;

   /* Liveness analysis: compute the list of live intervals */
   result->live_intervals = NULL;
   intervals = getLiveIntervals(graph);

   /* Copy the liveness info into the register allocator */
   if (intervals != NULL) {
      if (insertListOfIntervals(result, intervals) != RA_OK) {
         fatalError(AXE_REG_ALLOC_ERROR);
      }

      /* deallocate memory used to hold the results of the
       * liveness analysis */
      freeList(intervals);
   }

   /* create a list of freeRegisters */
   if (result->regNum > 0)
      result->freeRegisters = allocFreeRegisters(result->regNum);
   else
      result->freeRegisters = NULL;

   initializeRegisterConstraints(result);

   /* return the new register allocator */
   return result;
}

/*
 * Main register allocation function
 */
int executeLinearScan(t_reg_allocator *RA)
{
   t_list *current_element;
   t_live_interval *current_interval;
   t_list *active_intervals;
   int reg;

   /* test the preconditions */
   if (RA == NULL) /* Register allocator created? */
      return RA_INVALID_ALLOCATOR;
   if (RA->live_intervals == NULL) /* Liveness analysis ready? */
      return RA_OK;

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
      } else /* Otherwise, assign a new register to the current live interval */
      {
         RA->bindings[current_interval->varID] = reg;

         /* Add the current interval to the list of active intervals, in
          * order of ending points (to allow easier expire management) */
         active_intervals =
               addSorted(active_intervals, current_interval, compareEndPoints);
      }
   }

   /* free the list of active intervals */
   freeList(active_intervals);

   return RA_OK;
}

/*
 * Deallocate the register allocator data structures
 */
void finalizeRegAlloc(t_reg_allocator *RA)
{
   if (RA == NULL)
      return;

   /* If the list of live intervals is not empty,
    * deallocate its content */
   if (RA->live_intervals != NULL) {
      t_list *current_element;
      t_live_interval *current_interval;

      /* finalize the memory blocks associated with all
       * the live intervals */
      for (current_element = RA->live_intervals; current_element != NULL;
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
      freeList(RA->live_intervals);
   }

   /* Free memory used for the variable/register bindings */
   if (RA->bindings != NULL)
      free(RA->bindings);
   if (RA->freeRegisters != NULL) {
      t_list *current_element;

      current_element = RA->freeRegisters;
      while (current_element != NULL) {
         free(LDATA(current_element));
         current_element = LNEXT(current_element);
      }

      freeList(RA->freeRegisters);
   }

   free(RA);
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

t_tempLabel *allocTempLabel(t_axe_label *label, int regID)
{
   t_tempLabel *result;

   /* create a new temp-label */
   result = malloc(sizeof(t_tempLabel));
   if (result == NULL)
      fatalError(AXE_OUT_OF_MEMORY);

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
 * spilled variables and the label that points to the allocated memory block. */
t_list *materializeSpillMemory(t_program_infos *program, t_reg_allocator *RA)
{
   int counter;
   t_list *result;
   t_tempLabel *tlabel;
   t_axe_label *axe_label;
   t_axe_data *new_data_info;

   /* preconditions */
   if (program == NULL)
      fatalError(AXE_PROGRAM_NOT_INITIALIZED);
   if (RA == NULL)
      fatalError(AXE_INVALID_REG_ALLOC);

   /* initialize the local variable `result' */
   result = NULL;
   tlabel = NULL;

   /* allocate some memory for all spilled temporary variables */
   for (counter = 0; counter < RA->varNum; counter++) {
      if (RA->bindings[counter] != RA_SPILL_REQUIRED)
         continue;

      /* retrieve a new label */
      axe_label = newLabel(program);
      if (axe_label == NULL)
         fatalError(AXE_INVALID_LABEL);

      /* create a new tempLabel */
      tlabel = allocTempLabel(axe_label, counter);

      /* statically allocate some room for the spilled variable by
       * creating a new .WORD directive and making the label point to it. */
      new_data_info = initializeData(DIR_WORD, 0, axe_label);
      if (!new_data_info)
         fatalError(AXE_OUT_OF_MEMORY);

      /* update the list of directives */
      program->data = addElement(program->data, new_data_info, -1);

      /* add the current tlabel to the list of labelbindings */
      result = addElement(result, tlabel, -1);
   }

   /* postcondition: return the list of bindings */
   return result;
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

   pattern.regID = temp_register;
   elementFound =
         findElementWithCallback(labelBindings, &pattern, compareTempLabels);

   if (elementFound == NULL)
      fatalError(AXE_TRANSFORM_ERROR);

   tlabel = (t_tempLabel *)LDATA(elementFound);
   assert(tlabel != NULL);

   /* create a store instruction */
   storeInstr = genSWGlobalInstruction(NULL, selected_register, tlabel->label);

   /* create a node for the load instruction */
   storeNode = allocNode(graph, storeInstr);
   if (cflow_errorcode != CFLOW_OK)
      fatalError(AXE_TRANSFORM_ERROR);

   /* test if we have to insert the node `storeNode' before `current_node'
    * inside the basic block */
   if (before) {
      insertNodeBefore(current_block, current_node, storeNode);
   } else {
      insertNodeAfter(current_block, current_node, storeNode);
   }
   return 0;
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

   pattern.regID = temp_register;
   elementFound =
         findElementWithCallback(labelBindings, &pattern, compareTempLabels);

   if (elementFound == NULL)
      fatalError(AXE_TRANSFORM_ERROR);

   tlabel = (t_tempLabel *)LDATA(elementFound);
   assert(tlabel != NULL);

   /* create a load instruction */
   loadInstr = genLWGlobalInstruction(NULL, selected_register, tlabel->label);

   /* create a node for the load instruction */
   loadNode = allocNode(graph, loadInstr);

   /* test if an error occurred */
   if (cflow_errorcode != CFLOW_OK)
      fatalError(AXE_TRANSFORM_ERROR);

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
   return 0;
}

void materializeRegAllocInBBForInstructionNode(t_cflow_Graph *graph,
      t_basic_block *current_block, t_spillState *state,
      t_cflow_Node *current_node, t_reg_allocator *RA, t_list *label_bindings)
{
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
         fatalError(AXE_UNKNOWN_ERROR);

      /* If needed, write back the old variable that was assigned to this
       * slot before reassigning it */
      if (state->regs[current_row].needsWB == 1) {
         genStoreSpillVariable(state->regs[current_row].assignedVar,
               getSpillRegister(current_row), graph, current_block,
               current_node, label_bindings, 1);
      }

      /* Update the state of this spill slot */
      spillSlotInUse[current_row] = 1;
      argState[current_arg].spillSlot = current_row;
      state->regs[current_row].assignedVar = argState[current_arg].reg->ID;
      state->regs[current_row].needsWB = argState[current_arg].isDestination;

      /* Load the value of the variable in the spill register if not a
       * destination of the instruction */
      if (!argState[current_arg].isDestination) {
         genLoadSpillVariable(argState[current_arg].reg->ID,
               getSpillRegister(current_row), graph, current_block,
               current_node, label_bindings, 1);
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
}

void materializeRegAllocInBB(t_cflow_Graph *graph, t_basic_block *current_block,
      t_reg_allocator *RA, t_list *label_bindings)
{
   int counter, bbHasTermInstr;
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
      materializeRegAllocInBBForInstructionNode(
            graph, current_block, &state, current_node, RA, label_bindings);

      current_nd_element = LNEXT(current_nd_element);
   }

   bbHasTermInstr = current_block->nodes &&
         (isJumpInstruction(current_node->instr) ||
               isHaltOrRetInstruction(current_node->instr));

   /* writeback everything at the end of the basic block */
   for (counter = 0; counter < NUM_SPILL_REGS; counter++) {
      if (state.regs[counter].needsWB == 0)
         continue;
      genStoreSpillVariable(state.regs[counter].assignedVar,
            getSpillRegister(counter), graph, current_block, current_node,
            label_bindings, bbHasTermInstr);
   }
}

/* update the control flow informations by unsing the result
 * of the register allocation process and a list of bindings
 * between new assembly labels and spilled variables */
void materializeRegAllocInCFG(
      t_cflow_Graph *graph, t_reg_allocator *RA, t_list *label_bindings)
{
   t_list *current_bb_element;

   /* preconditions */
   assert(graph != NULL);
   assert(RA != NULL);

   current_bb_element = graph->blocks;
   while (current_bb_element != NULL) {
      t_basic_block *current_block = (t_basic_block *)LDATA(current_bb_element);

      materializeRegAllocInBB(graph, current_block, RA, label_bindings);

      /* retrieve the next basic block element */
      current_bb_element = LNEXT(current_bb_element);
   }
}

/* Replace the variable identifiers in the instructions of the CFG with the
 * register assignments in the register allocator. Materialize spilled
 * variables to the scratch registers. All new instructions are inserted
 * in the CFG. Synchronize the list of instructions with the newly
 * modified program. */
void materializeRegisterAllocation(
      t_program_infos *program, t_cflow_Graph *graph, t_reg_allocator *RA)
{
   t_list *label_bindings;

   /* retrieve a list of t_templabels for the given RA infos and
    * update the content of the data segment. */
   label_bindings = materializeSpillMemory(program, RA);

   /* update the control flow graph with the reg-alloc infos. */
   materializeRegAllocInCFG(graph, RA, label_bindings);

   /* finalize the list of tempLabels */
   finalizeListOfTempLabels(label_bindings);

   /* update the code segment informations */
   updateTheCodeSegment(program, graph);
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

   fprintf(fout, "BINDINGS : \n");
   for (counter = 0; counter < numVars; counter++) {
      if (bindings[counter] != RA_SPILL_REQUIRED) {
         fprintf(fout, "VAR T%d will be assigned to register R%d \n", counter,
               bindings[counter]);
      } else {
         fprintf(fout, "VAR T%d will be spilled \n", counter);
      }
   }

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
   while (current_element != NULL) {
      interval = (t_live_interval *)LDATA(current_element);

      fprintf(fout, "\tLIVE_INTERVAL of T%d : [%d, %d]", interval->varID,
            interval->startPoint, interval->endPoint);

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


/*
 * Wrapper function
 */

void doRegisterAllocation(t_program_infos *program)
{
   t_cflow_Graph *graph;
   t_reg_allocator *RA;

   /* create the control flow graph */
   debugPrintf("Creating a control flow graph.\n");
   graph = createFlowGraph(program->instructions);
   checkConsistency();

#ifndef NDEBUG
   assert(program != NULL);
   assert(file_infos != NULL);
   printGraphInfos(graph, file_infos->cfg_1, 0);
#endif

   debugPrintf("Executing a liveness analysis on the intermediate code\n");
   performLivenessAnalysis(graph);
   checkConsistency();

#ifndef NDEBUG
   printGraphInfos(graph, file_infos->cfg_2, 1);
#endif

   debugPrintf(
         "Performing register allocation using the linear scan algorithm.\n");

   /* initialize the register allocator by using the control flow
    * informations stored into the control flow graph */
   RA = initializeRegAlloc(graph);

   /* execute the linear scan algorithm */
   executeLinearScan(RA);

#ifndef NDEBUG
   printRegAllocInfos(RA, file_infos->reg_alloc_output);
#endif

   /* apply changes to the program informations by using the informations
    * of the register allocation process */
   debugPrintf("Updating the control flow informations.\n");
   materializeRegisterAllocation(program, graph, RA);

   finalizeRegAlloc(RA);
   finalizeGraph(graph);
}
