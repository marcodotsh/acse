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
#include "axe_debug.h"
#include "axe_errors.h"
#include "axe_utils.h"
#include "axe_gencode.h"

/*
 * LINEAR SCAN
 */

extern int errorcode;

static int compareIntervalIDs(void *varA, void *varB);
static int compareStartPoints(void *varA, void *varB);
static int compareEndPoints(void *varA, void *varB);
static t_list * updateListOfIntervals(t_list *result
            , t_cflow_Node *current_node, int counter);
static t_list * allocFreeRegisters(int regNum);
static t_list * addFreeRegister
      (t_list *registers, int regID, int position);
static int assignRegister(t_reg_allocator *RA, t_list *constraints);
static t_list * expireOldIntervals(t_reg_allocator *RA
            , t_list *active_intervals, t_live_interval *interval);
static t_list * getLiveIntervals(t_cflow_Graph *graph);
static int insertListOfIntervals(t_reg_allocator *RA, t_list *intervals);
static int insertLiveInterval(t_reg_allocator *RA, t_live_interval *interval);
static void finalizeLiveInterval (t_live_interval *interval);
static t_live_interval * allocLiveInterval(int varID, t_list *mcRegs, int startPoint, int endPoint);
static t_list * spillAtInterval(t_reg_allocator *RA
      , t_list *active_intervals, t_live_interval *interval);


/*
 * Perform a spill that allows the allocation of the given
 * interval, given the list of active live intervals
 */
t_list * spillAtInterval(t_reg_allocator *RA
      , t_list *active_intervals, t_live_interval *interval)
{
   t_list *last_element;
   t_live_interval *last_interval;
   
   /* get the last element of the list of active intervals */
   /* Precondition: if the list of active intervals is empty
    * we are working on a machine with 0 registers available
    * for the register allocation */
   if (active_intervals == NULL)
   {
      RA->bindings[interval->varID] = RA_SPILL_REQUIRED;
      return active_intervals;
   }
   
   last_element = getLastElement(active_intervals);
   last_interval = (t_live_interval *) LDATA(last_element);

   /* If the current interval ends before the last one, spill
    * the last one, otherwise spill the current interval. */
   if (last_interval->endPoint > interval->endPoint)
   {
      int attempt = RA->bindings[last_interval->varID];
      if (findElement(interval->mcRegConstraints, INTDATA(attempt))) {
         RA->bindings[interval->varID]
               = RA->bindings[last_interval->varID];
         RA->bindings[last_interval->varID] = RA_SPILL_REQUIRED;

         active_intervals = removeElement(active_intervals, last_interval);
         
         active_intervals = addSorted(active_intervals
                     , interval, compareEndPoints);
         return active_intervals;
      }
   }
      
   RA->bindings[interval->varID] = RA_SPILL_REQUIRED;
   return active_intervals;
}

/*
 * Given two live intervals, compare them by
 * the start point (find whichever starts first)
 */
int compareStartPoints(void *varA, void *varB)
{
   t_live_interval *liA;
   t_live_interval *liB;
   
   if (varA == NULL)
      return 0;

   if (varB == NULL)
      return 0;

   liA = (t_live_interval *) varA;
   liB = (t_live_interval *) varB;

   return (liA->startPoint - liB->startPoint);
}

/*
 * Given two live intervals, compare them
 * by the end point (find whichever ends first)
 */
int compareEndPoints(void *varA, void *varB)
{
   t_live_interval *liA;
   t_live_interval *liB;
   
   if (varA == NULL)
     return 0;

   if (varB == NULL)
      return 0;

   liA = (t_live_interval *) varA;
   liB = (t_live_interval *) varB;

   return (liA->endPoint - liB->endPoint);
}

/*
 * Given two live intervals, check if they
 * refer to the same interval
 */
int compareIntervalIDs(void *varA, void *varB)
{
   t_live_interval *liA;
   t_live_interval *liB;

   if (varA == NULL)
      return 0;

   if (varB == NULL)
      return 0;

   liA = (t_live_interval *) varA;
   liB = (t_live_interval *) varB;

   return (liA->varID == liB->varID);
}

/*
 * Insert a live interval in the register allocator
 */
int insertLiveInterval(t_reg_allocator *RA, t_live_interval *interval)
{
   /* test the preconditions */
   if (RA == NULL)
      return RA_INVALID_ALLOCATOR;

   if (interval == NULL)
      return RA_INVALID_INTERVAL;

   /* test if an interval for the requested variable is already inserted */
   if (findElementWithCallback(RA->live_intervals
               , interval, compareIntervalIDs) != NULL)
   {
      return RA_INTERVAL_ALREADY_INSERTED;
   }

   /* add the given interval to the list of intervals,
    * in order of starting point */
    RA->live_intervals = addSorted(RA->live_intervals
            , interval, compareStartPoints);
   
   return RA_OK;
}

/*
 * Allocate and initialize the free registers list,
 * assuming regNum general purpose registers
 */
t_list * allocFreeRegisters(int regNum)
{
   int count;
   t_list *result;

   /* initialize the local variables */
   count = 1;
   result = NULL;
   
   while(count <= regNum)
   {
      /* add a new register to the list of free registers */
      result = addFreeRegister(result, count, -1);

      /* update the `count' variable */
      count ++;
   }

   /* return the list of free registers */
   return result;
}

/*
 * Return register regID to the free list in the given position
 */
t_list * addFreeRegister(t_list *registers, int regID, int position)
{
   int *element;

   /* Allocate memory space for the reg id */
   element = (int *) malloc(sizeof(int));
   if (element == NULL)
      notifyError(AXE_OUT_OF_MEMORY);

   /* initialize element */
   (* element) = regID;
   
   /* update the list of registers */
   registers = addElement(registers, element, position);

   /* return the list of free registers */
   return registers;
}

int _compareFreeRegLI(void *freeReg, void *constraintReg)
{
   return *(int *)constraintReg == *(int *)freeReg;
}

/*
 * Get a new register from the free list
 */
int assignRegister(t_reg_allocator *RA, t_list *constraints)
{
   int regID;

   t_list *i = constraints;
   for (; i; i = LNEXT(i)) {
      regID = LINTDATA(i);
      t_list *freeReg = findElementWithCallback(RA->freeRegisters, &regID, _compareFreeRegLI);
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
   t_list *allregs = NULL;
   t_list *allregs2 = ra->freeRegisters;
   for (; allregs2; allregs2 = LNEXT(allregs2)) {
      int reg = *(int *)LDATA(allregs2);
      allregs = addElement(allregs, INTDATA(reg), -1);
   }

   t_list *i = ra->live_intervals;
   for (; i; i = LNEXT(i)) {
      t_live_interval *interval = LDATA(i);
      if (interval->mcRegConstraints)
         continue;
      interval->mcRegConstraints = cloneList(allregs);
      t_list *j = LNEXT(i);
      for (; j; j = LNEXT(j)) {
         t_live_interval *overlappingIval = LDATA(j);
         if (overlappingIval->startPoint > interval->endPoint)
            break;
         if (!overlappingIval->mcRegConstraints)
            continue;
         if (overlappingIval->startPoint == interval->endPoint) {
            /* an instruction is using interval as a source and overlappingIval
             * as a destination. Optimize the constraint order to allow
             * allocating source and destination to the same register
             * if possible. */
            interval->mcRegConstraints = optimizeRegisterSet(
               interval->mcRegConstraints, overlappingIval->mcRegConstraints);
         } else {
            interval->mcRegConstraints = subtractRegisterSets(
               interval->mcRegConstraints, overlappingIval->mcRegConstraints);
         }
      }
      assert(interval->mcRegConstraints);
   }
}

/*
 * Allocate and initialize the register allocator
 */
t_reg_allocator * initializeRegAlloc(t_cflow_Graph *graph)
{
   t_reg_allocator *result; /* the register allocator */
   t_list *intervals;
   t_list *current_cflow_var;
   t_cflow_var *cflow_var;
   int max_var_ID;
   int counter;

   /* Check preconditions: the cfg must exist */
   if (graph == NULL)
      notifyError(AXE_INVALID_CFLOW_GRAPH);

   /* allocate memory for a new instance of `t_reg_allocator' */
   result = (t_reg_allocator *) malloc(sizeof(t_reg_allocator));
   if (result == NULL)
      notifyError(AXE_OUT_OF_MEMORY);
   
   /* initialize the register allocator informations */
   /* Reserve a few registers (NUM_SPILL_REGS) to handle spills */
   result->regNum = NUM_REGISTERS - NUM_SPILL_REGS;

   /* retrieve the max identifier from each live interval */
   max_var_ID = 0;
   current_cflow_var = graph->cflow_variables;
   while (current_cflow_var != NULL)
   {
      /* fetch the data informations about a variable */
      cflow_var = (t_cflow_var *) LDATA(current_cflow_var);
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
      notifyError(AXE_OUT_OF_MEMORY);
      
   /* initialize the array of bindings */
   for (counter = 0; counter < result->varNum; counter++)
      result->bindings[counter] = RA_REGISTER_INVALID;

   /* Liveness analysis: compute the list of live intervals */
   result->live_intervals = NULL;
   intervals = getLiveIntervals(graph);

   /* Copy the liveness info into the register allocator */
   if (intervals != NULL)
   {
      if (insertListOfIntervals(result, intervals) != RA_OK)
      {
         notifyError(AXE_REG_ALLOC_ERROR);
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
 * Deallocate the register allocator data structures
 */
void finalizeRegAlloc(t_reg_allocator *RA)
{
   if (RA == NULL)
      return;

   /* If the list of live intervals is not empty,
    * deallocate its content */
   if (RA->live_intervals != NULL)
   {
      t_list *current_element;
      t_live_interval *current_interval;

      /* finalize the memory blocks associated with all
       * the live intervals */
      for (current_element = RA->live_intervals
            ;  current_element != NULL
            ;  current_element = LNEXT(current_element))
      {
         /* fetch the current interval */
         current_interval = (t_live_interval *) LDATA(current_element);
         if (current_interval != NULL)
         {
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
   if (RA->freeRegisters != NULL)
   {
      t_list *current_element;

      current_element = RA->freeRegisters;
      while (current_element != NULL)
      {
         free(LDATA(current_element));
         current_element = LNEXT(current_element);
      }

      freeList(RA->freeRegisters);
   }

   free(RA);
}

/*
 * Allocate and initialize a live interval data structure
 * with a given varID, starting and ending points
 */
t_live_interval * allocLiveInterval
               (int varID, t_list *mcRegs, int startPoint, int endPoint)
{
   t_live_interval *result;

   /* create a new instance of `t_live_interval' */
   result = malloc(sizeof(t_live_interval));
   if (result == NULL)
      notifyError(AXE_OUT_OF_MEMORY);

   /* initialize the new instance */
   result->varID = varID;
   result->mcRegConstraints = cloneList(mcRegs);
   result->startPoint = startPoint;
   result->endPoint = endPoint;

   /* return the new `t_live_interval' */
   return result;
}

/*
 * Deallocate a live interval
 */
void finalizeLiveInterval (t_live_interval *interval)
{
   if (interval == NULL)
      return;

   /* finalize the current interval */
   free(interval->mcRegConstraints);
   free(interval);
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

   for (current_element = intervals
         ;  current_element != NULL
         ;  current_element = LNEXT(current_element) )
   {
      /* Get the current live interval */
      interval = (t_live_interval *) LDATA(current_element);
      
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

/*
 * Perform live intervals computation
 */
t_list * getLiveIntervals(t_cflow_Graph *graph)
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
   while (current_bb_element != NULL)
   {
      current_block = (t_basic_block *) LDATA(current_bb_element);

      /* fetch the first node of the basic block */
      current_nd_element = current_block->nodes;
      while(current_nd_element != NULL)
      {
         current_node = (t_cflow_Node *) LDATA(current_nd_element);

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

/*
 * Update the liveness interval for the variable 'id', used or defined
 * at position 'counter'.
 */
t_list *updateVarInterval(t_cflow_var *var, int counter, t_list *intervals )
{
    t_list *element_found;
    t_live_interval *interval_found;
    t_live_interval pattern;

    if (var->ID == RA_EXCLUDED_VARIABLE || var->ID == VAR_PSW)
        return intervals;
    
    pattern.varID = var->ID;
    /* search for the current live interval */
    element_found = findElementWithCallback(intervals, &pattern, compareIntervalIDs);
    if (element_found != NULL)
    {
        interval_found = (t_live_interval *) LDATA(element_found);
        /* update the interval informations */
        if (interval_found->startPoint > counter)
            interval_found->startPoint = counter;
        if (interval_found->endPoint < counter)
            interval_found->endPoint = counter;
    }
    else
    {
        /* we have to add a new live interval */
        interval_found = allocLiveInterval(var->ID, var->mcRegWhitelist, counter, counter);
        if (interval_found == NULL)
            notifyError(AXE_OUT_OF_MEMORY);
        intervals = addElement(intervals, interval_found, -1);
    }
    
    return intervals;
}

/*
 * Use liveness information to update the list of
 * live intervals
 */
t_list * updateListOfIntervals(t_list *result
         , t_cflow_Node *current_node, int counter)
{
   t_list *current_element;
   t_cflow_var *current_var;
   int i;
   
   if (current_node == NULL)
      return result;

   current_element = current_node->in;
   while (current_element != NULL)
   {
      current_var = (t_cflow_var *) LDATA(current_element);

      result = updateVarInterval(current_var, counter, result);
      
      /* fetch the next element in the list of live variables */
      current_element = LNEXT(current_element);
   }
   
   current_element = current_node->out;
   while (current_element != NULL)
   {
      current_var = (t_cflow_var *) LDATA(current_element);

      result = updateVarInterval(current_var, counter, result);
      
      /* fetch the next element in the list of live variables */
      current_element = LNEXT(current_element);
   }

   for (i=0; i<CFLOW_MAX_DEFS; i++) {
      if (current_node->defs[i])
         result = updateVarInterval(current_node->defs[i], counter, result);
   }

   return result;
}

/*
 * Remove from active_intervals all the live intervals that end before the
 * beginning of the current live interval
 */
t_list * expireOldIntervals(t_reg_allocator *RA, t_list *active_intervals
               , t_live_interval *interval)
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
   while(current_element != NULL)
   {
      /* Get the live interval */
      current_interval = (t_live_interval *) LDATA(current_element);

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
            interval->mcRegConstraints = optimizeRegisterSet(
                  interval->mcRegConstraints, allocated);
            freeList(allocated);
         }
      }

      /* Get the next live interval */
      next_element = LNEXT(current_element);

      /* Remove the current element from the list */
      active_intervals = removeElement(active_intervals, current_interval);

      /* Free all the registers associated with the removed interval */
      RA->freeRegisters = addFreeRegister
            (RA->freeRegisters, RA->bindings[current_interval->varID], 0);

      /* Step to the next interval */
      current_element = next_element;
   }

   /* Return the updated list of active intervals */
   return active_intervals;
}

/*
 * Main register allocation function
 */
int executeLinearScan(t_reg_allocator *RA)
{
   t_list *current_element;
   t_live_interval *current_interval;
   t_list *active_intervals;
   
   /* test the preconditions */
   if (RA == NULL)   /* Register allocator created? */
      return RA_INVALID_ALLOCATOR;
   if (RA->live_intervals == NULL) /* Liveness analysis ready? */
      return RA_OK;

   /* initialize the list of active intervals */
   active_intervals = NULL;
   
   /* Iterate over the list of live intervals */   
   for( current_element = RA->live_intervals
        ;   current_element != NULL
        ;   current_element = LNEXT(current_element) )
   {
      /* Get the live interval */
      current_interval = (t_live_interval *) LDATA(current_element);

      /* Check which intervals are ended and remove 
       * them from the active set, thus freeing registers */
      active_intervals = expireOldIntervals
               (RA, active_intervals, current_interval);

      int reg = assignRegister(RA, current_interval->mcRegConstraints);

      /* If all registers are busy, perform a spill */
      if (reg == RA_SPILL_REQUIRED)
      {
         /* perform a spill */
         active_intervals = spillAtInterval
               (RA, active_intervals, current_interval);
      }
      else /* Otherwise, assign a new register to the current live interval */
      {
         RA->bindings[current_interval->varID] = reg;

         /* Add the current interval to the list of active intervals, in
          * order of ending points (to allow easier expire management) */
         active_intervals = addSorted(active_intervals
            , current_interval, compareEndPoints);
      }
   }

   /* free the list of active intervals */
   freeList(active_intervals);
   
   return RA_OK;
}


/*
 * MATERIALIZATION AND SPILL
 */

extern int cflow_errorcode;

typedef struct t_tempLabel
{
   t_axe_label *labelID;
   int regID;
} t_tempLabel;


static t_axe_instruction * _createUnary (t_program_infos *program
            , int reg, t_axe_label *label, int opcode);
static t_tempLabel * allocTempLabel(t_axe_label *labelID, int regID);
static void freeTempLabel(t_tempLabel *tempLabel);
static void finalizeListOfTempLabels(t_list *tempLabels);
static int compareTempLabels(void *valA, void *valB);

/* create new locations into the data segment in order to manage correctly
 * spilled variables */
static void updateTheDataSegment
            (t_program_infos *program, t_list *tempLabels);
static void updateTheCodeSegment
            (t_program_infos *program, t_cflow_Graph *graph);

/* update the control flow informations by unsing the result
 * of the register allocation process and a list of bindings
 * between new assembly labels and spilled variables */
static void updatCflowInfos(t_program_infos *program, t_cflow_Graph *graph
            , t_reg_allocator *RA, t_list *label_bindings);

/* this function returns a list of t_templabel containing binding
 * informations between spilled variables and labels that will point
 * to a memory block in the data segment */
static t_list * retrieveLabelBindings(t_program_infos *program, t_reg_allocator *RA);
      
int _insertLoadSpill(t_program_infos *program, int temp_register, int selected_register
            , t_cflow_Graph *graph, t_basic_block *current_block
            , t_cflow_Node *current_node, t_list *labelBindings, int before);
            
int _insertStoreSpill(t_program_infos *program, int temp_register, int selected_register
            , t_cflow_Graph *graph, t_basic_block *current_block
            , t_cflow_Node *current_node, t_list *labelBindings, int before);

void updateTheCodeSegment(t_program_infos *program, t_cflow_Graph *graph)
{
   t_list *current_bb_element;
   t_list *current_nd_element;
   t_basic_block *bblock;
   t_cflow_Node *node;
   
   /* preconditions */
   if (program == NULL)
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);

   if (graph == NULL)
      notifyError(AXE_INVALID_CFLOW_GRAPH);

   current_bb_element = graph->blocks;
   while(current_bb_element != NULL)
   {
      bblock = (t_basic_block *) LDATA(current_bb_element);

      current_nd_element = bblock->nodes;
      while(current_nd_element != NULL)
      {
         node = (t_cflow_Node *) LDATA(current_nd_element);

         program->instructions =
               addElement(program->instructions, node->instr, -1);
         
         current_nd_element = LNEXT(current_nd_element);
      }

      current_bb_element = LNEXT(current_bb_element);
   }
}

void updateTheDataSegment(t_program_infos *program, t_list *labelBindings)
{
   t_list *current_element;
   t_tempLabel *current_TL;
   t_axe_data *new_data_info;

   /* preconditions */
   if (program == NULL) {
      finalizeListOfTempLabels(labelBindings);
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);
   }

   /* initialize the value of `current_element' */
   current_element = labelBindings;
   while(current_element != NULL)
   {
      current_TL = (t_tempLabel *) LDATA(current_element);

      new_data_info = initializeData (DIR_WORD, 0, current_TL->labelID);
         
      if (new_data_info == NULL){
         finalizeListOfTempLabels(labelBindings);
         notifyError(AXE_OUT_OF_MEMORY);
      }

      /* update the list of directives */
      program->data = addElement(program->data, new_data_info, -1);
      
      current_element = LNEXT(current_element);
   }
}

int compareTempLabels(void *valA, void *valB)
{
   t_tempLabel *tlA;
   t_tempLabel *tlB;
   
   if (valA == NULL)
      return 0;
   if (valB == NULL)
      return 0;

   tlA = (t_tempLabel *) valA;
   tlB = (t_tempLabel *) valB;

   return (tlA->regID == tlB->regID);
}

t_tempLabel * allocTempLabel(t_axe_label *labelID, int regID)
{
   t_tempLabel *result;

   /* preconditions */
   if (labelID == NULL) {
      errorcode = AXE_INVALID_LABEL;
      return NULL;
   }

   /* create a new temp-label */
   result = malloc(sizeof(t_tempLabel));
   if (result == NULL) {
      errorcode = AXE_OUT_OF_MEMORY;
      return NULL;
   }
   
   /* initialize the temp label */
   result->labelID = labelID;
   result->regID = regID;

   return result;
}

void freeTempLabel(t_tempLabel *tempLabel)
{
   if (tempLabel != NULL)
      free(tempLabel);
}

void finalizeListOfTempLabels(t_list *tempLabels)
{
   t_list *current_element;
   t_tempLabel *tempLabel;

   /* test the preconditions */
   if (tempLabels == NULL)
      return;

   /* assign to current_element the address of the first
    * element of the list `tempLabels' */
   current_element = tempLabels;
   
   while(current_element != NULL)
   {
      tempLabel = (t_tempLabel *) LDATA(current_element);
      if (tempLabel != NULL)
         freeTempLabel(tempLabel);
      
      current_element = LNEXT(current_element);
   }

   /* finalize the list of elements of type `t_tempLabel' */
   freeList(tempLabels);
}

void materializeRegisterAllocation(t_program_infos *program,
  t_cflow_Graph *graph, t_reg_allocator *RA)
{
   t_list *label_bindings;

   /* retrieve a list of t_templabels for the given RA infos.*/
   label_bindings = retrieveLabelBindings(program, RA);

   /* update the content of the data segment */
   updateTheDataSegment(program, label_bindings);

   /* update the control flow graph with the reg-alloc infos. */
   updatCflowInfos(program, graph, RA, label_bindings);

   /* finalize the list of tempLabels */
   finalizeListOfTempLabels(label_bindings);

   /* erase the old code segment */
   freeList(program->instructions);
   program->instructions = NULL;

   /* update the code segment informations */
   updateTheCodeSegment(program, graph);
}

t_list * retrieveLabelBindings(t_program_infos *program, t_reg_allocator *RA)
{
   int counter;
   t_list *result;
   t_tempLabel *tlabel;
   t_axe_label *axe_label;
   
   /* preconditions */
   if (program == NULL)
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);
   if (RA == NULL)
      notifyError(AXE_INVALID_REG_ALLOC);

   /* initialize the local variable `result' */
   result = NULL;
   tlabel = NULL;

   for (counter = 0; counter < RA->varNum; counter++)
   {
      if (RA->bindings[counter] == RA_SPILL_REQUIRED)
      {
         /* retrieve a new label */
         axe_label = newLabel(program);
         if (axe_label == NULL)
            notifyError(AXE_INVALID_LABEL);

         /* create a new tempLabel */
         tlabel = allocTempLabel(axe_label, counter);

         /* add the current tlabel to the list of labelbindings */
         result = addElement(result, tlabel, -1);
      }
   }
   
   /* postcondition: return the list of bindings */
   return result;
}

void updatCflowInfos(t_program_infos *program, t_cflow_Graph *graph
            , t_reg_allocator *RA, t_list *label_bindings)
{
   /* preconditions */
   assert(program != NULL);
   assert(graph != NULL);
   assert(RA != NULL);
   
   t_list *current_bb_element = graph->blocks;
   while (current_bb_element != NULL)
   {
      int counter;

      /* spill register slots
       * each array element corresponds to one of the registers reserved for
       * the spill, ordered by ascending register number. */
      struct {
         int assignedVar;
         int needsWB;
         int inUse;
      } assignedRegisters[NUM_SPILL_REGS];

      t_basic_block *current_block = (t_basic_block *)LDATA(current_bb_element);
      assert(current_block != NULL);

      /* initialize used_Registers */
      for (counter = 0; counter < NUM_SPILL_REGS; counter ++)
      {
         assignedRegisters[counter].assignedVar = REG_INVALID;
         assignedRegisters[counter].needsWB = 0;
         assignedRegisters[counter].inUse = 0;
      }

      /* phase one
       * retrieve the list of nodes for the current basic block */
      t_list *current_nd_element = current_block->nodes;
      t_axe_instruction *current_instr;
      t_cflow_Node *current_node;
      while(current_nd_element != NULL)
      {
         /* reordering table which associates the three instruction registers
          * (in this order: dest, src1, src2) to the spill register slots.
          * For example, if used_Registers[1] == 2, the src1 register will be
          * assigned to the third spill register. */
         int used_Registers[3] = {-1, -1, -1};

         /* retrieve the data associated with the current node of the block */
         current_node = (t_cflow_Node *)LDATA(current_nd_element);

         /* fetch the current instruction */
         current_instr = current_node->instr;

         /* Test if a requested variable is already loaded into a register
          * from a previous instruction. 
          * Note: in this moment, all inUse flags are set to zero. */
         if (current_instr->reg_dest  != NULL)
         {
            if (RA->bindings[(current_instr->reg_dest)->ID]
                  == RA_SPILL_REQUIRED)
            {
               int current_row = 0;
               int found = 0;
               
               while ((current_row < NUM_SPILL_REGS) && !found)
               {
                  if (assignedRegisters[current_row].assignedVar
                           == (current_instr->reg_dest)->ID)
                  {
                     /* update the value of used_Register */
                     used_Registers[0] = current_row;

                     /* update the value of `assignedRegisters` */
                     /* set currently used flag */
                     assignedRegisters[current_row].inUse = 1;

                     /* test if a write back is needed */
                     if (! (current_instr->reg_dest)->indirect)
                     {
                        /* if the current instruction is a STORE we don't need
                         * to write back the value of reg_dest again in memory. */
                        /* Also if the current instruction is a WRITE instruction
                         * we don't have to set the flag "dirty" for the
                         * register assignedRegisters[current_row], since reg_dest
                         * is "used" but not "defined" */
                        if (  (current_instr->opcode != OPC_STORE)
                              && (current_instr->opcode != OPC_AXE_WRITE) )
                        {
                           assignedRegisters[current_row].needsWB = 1;
                        }
                        /* if the current instruction is a STORE we have to
                         * notify that the write back is happened by
                         * resetting the flag "dirty" */
                        if (current_instr->opcode == OPC_STORE)
                           assignedRegisters[current_row].needsWB = 0;
                     }

                     /* notify that the value was found */
                     found = 1;
                  }

                  current_row ++;
               }
            }
         }
         if (current_instr->reg_src1  != NULL)
         {
            if (RA->bindings[(current_instr->reg_src1)->ID]
                  == RA_SPILL_REQUIRED)
            {
               int current_row = 0;
               int found = 0;
               
               while ((current_row < NUM_SPILL_REGS) && !found)
               {
                  if (assignedRegisters[current_row].assignedVar
                           == (current_instr->reg_src1)->ID)
                  {
                     /* update the value of used_Register */
                     used_Registers[1] = current_row;

                     /* update the value of `assignedRegisters` */
                     /* set currently used flag */
                     assignedRegisters[current_row].inUse = 1;

                     /* notify that the value was found */
                     found = 1;
                  }
                  
                  current_row ++;
               }
            }
         }
         if (current_instr->reg_src2  != NULL)
         {
            if (RA->bindings[(current_instr->reg_src2)->ID]
                  == RA_SPILL_REQUIRED)
            {
               int current_row = 0;
               int found = 0;
               
               while ((current_row < NUM_SPILL_REGS) && !found)
               {
                  if (assignedRegisters[current_row].assignedVar
                           == (current_instr->reg_src2)->ID)
                  {
                     /* update the value of used_Register */
                     used_Registers[2] = current_row;

                     /* update the value of `assignedRegisters` */
                     /* set currently used flag */
                     assignedRegisters[current_row].inUse = 1;

                     /* notify that the value was found */
                     found = 1;
                  }
                  
                  current_row ++;
               }
            }
         }
         /* phase two
          * free space for the new values in the registers by writing back the
          * variables we don't need for this instruction, and then loading
          * the variables we need instead. */
         if (current_instr->reg_src1  != NULL)
         {
            if ((RA->bindings[(current_instr->reg_src1)->ID]
                  == RA_SPILL_REQUIRED) && (used_Registers[1] == -1))
            {
               int current_row = 0;
               int found = 0;
               
               while ((current_row < NUM_SPILL_REGS) && !found)
               {
                  if (assignedRegisters[current_row].inUse == 0)
                  {
                     int register_found = current_row + NUM_REGISTERS
                           + 1 - NUM_SPILL_REGS;
                     
                     if (assignedRegisters[current_row].needsWB == 1)
                     {
                        /* NEED WRITE BACK */
                        _insertStoreSpill(program, assignedRegisters[current_row].assignedVar
                              , register_found, graph, current_block
                                    , current_node, label_bindings, 1);
                     }

                     _insertLoadSpill(program, (current_instr->reg_src1)->ID
                           , register_found, graph, current_block
                                , current_node, label_bindings, 1);

                     /* update the control informations */
                     assignedRegisters[current_row].assignedVar = (current_instr->reg_src1)->ID;
                     assignedRegisters[current_row].needsWB = 0;
                     assignedRegisters[current_row].inUse = 1;
                     used_Registers[1] = current_row;

                     found = 1;
                  }
                  
                  current_row ++;
               }
               assert(found != 0);
            }
         }
         if (current_instr->reg_src2  != NULL)
         {
            if ((RA->bindings[(current_instr->reg_src2)->ID]
                  == RA_SPILL_REQUIRED) && (used_Registers[2] == -1))
            {
               int current_row = 0;
               int found = 0;

               /* reuse registers allocated in phase 2 of this same instruction
                * if possible. */
               if (current_instr->reg_src1 && (current_instr->reg_src2)->ID == (current_instr->reg_src1)->ID)
               {
                  used_Registers[2] = used_Registers[1];
                  found = 1;
               }

               while ((current_row < NUM_SPILL_REGS) && !found)
               {
                  if (assignedRegisters[current_row].inUse == 0)
                  {
                     int register_found = current_row + NUM_REGISTERS
                           + 1 - NUM_SPILL_REGS;
                     
                     if (assignedRegisters[current_row].needsWB == 1)
                     {
                        /* NEED WRITE BACK */
                        _insertStoreSpill(program, assignedRegisters[current_row].assignedVar
                              , register_found, graph, current_block
                                    , current_node, label_bindings, 1);
                     }

                     _insertLoadSpill(program, (current_instr->reg_src2)->ID
                           , register_found, graph, current_block
                                , current_node, label_bindings, 1);

                     /* update the control informations */
                     assignedRegisters[current_row].assignedVar = (current_instr->reg_src2)->ID;
                     assignedRegisters[current_row].needsWB = 0;
                     assignedRegisters[current_row].inUse = 1;
                     used_Registers[2] = current_row;
                     
                     found = 1;
                  }
                  
                  current_row ++;
               }
            }
         }
         if (current_instr->reg_dest  != NULL)
         {
            if ((RA->bindings[(current_instr->reg_dest)->ID]
                  == RA_SPILL_REQUIRED) && (used_Registers[0] == -1))
            {
               int current_row = 0;
               int found = 0;

               /* reuse registers allocated in phase 2 if possible */
               if (current_instr->reg_src2 && (current_instr->reg_src2)->ID == (current_instr->reg_dest)->ID) {
                  current_row = used_Registers[2];
                  found = 1;
               } else if (current_instr->reg_src1 && (current_instr->reg_src1)->ID == (current_instr->reg_dest)->ID) {
                  current_row = used_Registers[1];
                  found = 1;
               }
               
               while ((current_row < NUM_SPILL_REGS) && !found)
               {
                  if (assignedRegisters[current_row].inUse == 0)
                  {
                     int register_found = current_row + NUM_REGISTERS
                           + 1 - NUM_SPILL_REGS;
                     
                     if (assignedRegisters[current_row].needsWB == 1)
                     {
                        /* NEED WRITE BACK */
                        _insertStoreSpill(program, assignedRegisters[current_row].assignedVar
                              , register_found, graph, current_block
                                    , current_node, label_bindings, 1);
                     }

                     /* test if we need to load the value from register */
                     if (  (current_instr->reg_dest)->indirect
                           || (current_instr->opcode == OPC_AXE_WRITE))
                     {
                        _insertLoadSpill(program, (current_instr->reg_dest)->ID
                              , register_found, graph, current_block
                                    , current_node, label_bindings, 1);
                     }

                     /* update the control informations */
                     assignedRegisters[current_row].assignedVar = (current_instr->reg_dest)->ID;
                     assignedRegisters[current_row].needsWB = 0;
                     assignedRegisters[current_row].inUse = 1;
                     
                     found = 1;
                  }
                  else
                  {
                     current_row ++;
                  }
               }

               if (! (current_instr->reg_dest)->indirect)
               {
                  if (  (current_instr->opcode != OPC_STORE)
                        && (current_instr->opcode != OPC_AXE_WRITE) )
                  {
                     assignedRegisters[current_row].needsWB = 1;
                  }
               }
               used_Registers[0] = current_row;

               assert(found != 0);
            }
         }

         /* rewrite the register identifiers to use the appropriate spill
          * register number instead of the variable number. */
         if (current_instr->reg_dest != NULL)
         {
            if (used_Registers[0] != -1)
            {
               current_instr->reg_dest->ID = used_Registers[0] + NUM_REGISTERS
                           + 1 - NUM_SPILL_REGS;

               assignedRegisters[used_Registers[0]].inUse = 0;
            }
            else
               current_instr->reg_dest->ID =
                     RA->bindings[(current_instr->reg_dest)->ID];
         }
         if (current_instr->reg_src1 != NULL)
         {
            if (used_Registers[1] != -1)
            {
               current_instr->reg_src1->ID = used_Registers[1] + NUM_REGISTERS
                           + 1 - NUM_SPILL_REGS;
               assignedRegisters[used_Registers[1]].inUse = 0;
            }
            else
               current_instr->reg_src1->ID =
                     RA->bindings[(current_instr->reg_src1)->ID];
         }
         if (current_instr->reg_src2 != NULL)
         {
            if (used_Registers[2] != -1)
            {
               current_instr->reg_src2->ID = used_Registers[2] + NUM_REGISTERS
                           + 1 - NUM_SPILL_REGS;
               assignedRegisters[used_Registers[2]].inUse = 0;
            }
            else
               current_instr->reg_src2->ID =
                     RA->bindings[(current_instr->reg_src2)->ID];
         }

         /* retrieve the previous element */
         current_nd_element = LNEXT(current_nd_element);
      }

      int bbHasTermInstr = current_block->nodes && 
            (isJumpInstruction(current_instr) || 
            isHaltOrRetInstruction(current_instr));

      /* writeback everything at the end of the basic block */
      for (counter = 0; counter < NUM_SPILL_REGS; counter ++)
      {
         if (assignedRegisters[counter].needsWB == 1)
         {
            /* NEED WRITE BACK */
            _insertStoreSpill(program, assignedRegisters[counter].assignedVar
                     , (counter + NUM_REGISTERS + 1 - NUM_SPILL_REGS)
                           , graph, current_block
                              , current_node, label_bindings, bbHasTermInstr);
         }
      }
      
      /* retrieve the next basic block element */
      current_bb_element = LNEXT(current_bb_element);
   }
}

int _insertStoreSpill(t_program_infos *program, int temp_register, int selected_register
            , t_cflow_Graph *graph, t_basic_block *current_block
            , t_cflow_Node *current_node, t_list *labelBindings, int before)
{
   t_axe_instruction *storeInstr;
   t_cflow_Node *storeNode = NULL;
   t_list *elementFound;
   t_tempLabel pattern;
   t_tempLabel *tlabel;

   pattern.regID = temp_register;
   elementFound = findElementWithCallback (labelBindings
               , &pattern, compareTempLabels);

   if (elementFound == NULL) {
      finalizeNode(storeNode);
      errorcode = AXE_TRANSFORM_ERROR;
      return -1;
   }

   tlabel = (t_tempLabel *) LDATA(elementFound);
   assert(tlabel != NULL);

   /* create a store instruction */
   storeInstr = genSTOREInstruction(NULL, selected_register, tlabel->labelID, 0);

   /* create a node for the load instruction */
   storeNode = allocNode (graph, storeInstr);
   if (cflow_errorcode != CFLOW_OK) {
      finalizeNode(storeNode);
      finalizeInstruction(storeInstr);
      errorcode = AXE_TRANSFORM_ERROR;
      return -1;
   }

   /* test if we have to insert the node `storeNode' before `current_node'
    * inside the basic block */
   if (before == 0)
      insertNodeAfter(current_block, current_node, storeNode);
   else
      insertNodeBefore(current_block, current_node, storeNode);
   
   return 0;
}

int _insertLoadSpill(t_program_infos *program, int temp_register, int selected_register
            , t_cflow_Graph *graph, t_basic_block *block
            , t_cflow_Node *current_node, t_list *labelBindings, int before)
{
   t_axe_instruction *loadInstr;
   t_cflow_Node *loadNode = NULL;
   t_list *elementFound;
   t_tempLabel pattern;
   t_tempLabel *tlabel;

   pattern.regID = temp_register;
   elementFound = findElementWithCallback (labelBindings
               , &pattern, compareTempLabels);

   if (elementFound == NULL) {
      finalizeNode(loadNode);
      errorcode = AXE_TRANSFORM_ERROR;
      return -1;
   }

   tlabel = (t_tempLabel *) LDATA(elementFound);
   assert(tlabel != NULL);
   
   /* create a load instruction */
   loadInstr = genLOADInstruction(NULL, selected_register, tlabel->labelID, 0);

   /* create a node for the load instruction */
   loadNode = allocNode (graph, loadInstr);

   /* test if an error occurred */
   if (cflow_errorcode != CFLOW_OK) {
      finalizeNode(loadNode);
      finalizeInstruction(loadInstr);
      errorcode = AXE_TRANSFORM_ERROR;
      return -1;
   }

   if ((current_node->instr)->labelID != NULL)
   {
      /* modify the label informations */
      loadInstr->labelID = (current_node->instr)->labelID;
      (current_node->instr)->labelID = NULL;
   }
   
   if (before == 1)
      /* insert the node `loadNode' before `current_node' */
      insertNodeBefore(block, current_node, loadNode);
   else
      insertNodeAfter(block, current_node, loadNode);

   return 0;
}

