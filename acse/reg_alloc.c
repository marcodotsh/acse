/// @file reg_alloc.c

#include <assert.h>
#include <stdbool.h>
#include "reg_alloc.h"
#include "target_info.h"
#include "acse.h"
#include "utils.h"
#include "gencode.h"
#include "program.h"
#include "list.h"
#include "cflow_graph.h"
#include "target_asm_print.h"

#define MAX_INSTR_ARGS 3

/* constants */
#define RA_SPILL_REQUIRED    ((t_regID)(-1))
#define RA_REGISTER_INVALID  ((t_regID)(0))
#define RA_EXCLUDED_VARIABLE ((t_regID)(0))


typedef struct t_liveInterval {
  t_regID varID;                /* a variable identifier */
  t_listNode *mcRegConstraints; /* list of all registers where this variable
                                 * can be allocated. */
  int startPoint;               /* the index of the first instruction
                                 * that make use of (or define) this variable */
  int endPoint;                 /* the index of the last instruction
                                 * that make use of (or define) this variable */
} t_liveInterval;

typedef struct t_regAllocator {
  t_listNode *live_intervals; /* an ordered list of live intervals */
  int varNum;                 /* number of variables */
  t_regID *bindings;          /* an array of bindings of kind : varID-->register.
                               * If a certain variable X need to be spilled
                               * in memory, the value of `register' is set
                               * to the value of the macro RA_SPILL_REQUIRED */
  t_listNode *freeRegisters; /* a list of free registers */
} t_regAllocator;

typedef struct t_tempLabel {
  t_label *label;
  t_regID regID;
} t_tempLabel;

/* Structure representing the current state of an instruction argument during
 * the spill materialization process */
typedef struct t_spillInstrArgState {
  /* The register argument structure */
  t_instrArg *reg;
  /* If the register is a destination register */
  int isDestination;
  /* The spill register index where the argument will be materialized, or
   * -1 otherwise. */
  int spillSlot;
} t_spillInstrRegState;

/* Structure representing the current state of a spill-reserved register */
typedef struct t_spillRegState {
  /* virtual register ID associated to this spill register */
  t_regID assignedVar;
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
t_liveInterval *newLiveInterval(
    t_regID varID, t_listNode *mcRegs, int startPoint, int endPoint)
{
  /* create a new instance of `t_liveInterval' */
  t_liveInterval *result = malloc(sizeof(t_liveInterval));
  if (result == NULL)
    fatalError("out of memory");

  /* initialize the new instance */
  result->varID = varID;
  result->mcRegConstraints = listClone(mcRegs);
  result->startPoint = startPoint;
  result->endPoint = endPoint;

  /* return the new `t_liveInterval' */
  return result;
}

/* Deallocate a live interval */
void deleteLiveInterval(t_liveInterval *interval)
{
  if (interval == NULL)
    return;

  /* finalize the current interval */
  free(interval->mcRegConstraints);
  free(interval);
}

/* Given two live intervals, compare them by the start point (find whichever
 * starts first) */
int compareLiveIntStartPoints(void *varA, void *varB)
{
  t_liveInterval *liA = (t_liveInterval *)varA;
  t_liveInterval *liB = (t_liveInterval *)varB;

  if (varA == NULL)
    return 0;
  if (varB == NULL)
    return 0;

  return liA->startPoint - liB->startPoint;
}

/* Given two live intervals, compare them by the end point (find whichever
 * ends first) */
int compareLiveIntEndPoints(void *varA, void *varB)
{
  t_liveInterval *liA = (t_liveInterval *)varA;
  t_liveInterval *liB = (t_liveInterval *)varB;

  if (varA == NULL)
    return 0;
  if (varB == NULL)
    return 0;

  return liA->endPoint - liB->endPoint;
}

/* Given two live intervals, check if they refer to the same interval */
bool compareLiveIntIDs(void *varA, void *varB)
{
  t_liveInterval *liA = (t_liveInterval *)varA;
  t_liveInterval *liB = (t_liveInterval *)varB;

  if (varA == NULL)
    return false;
  if (varB == NULL)
    return false;

  return liA->varID == liB->varID;
}

/* Update the liveness interval for the variable 'ID', used or defined
 * at position 'counter'. Returns an error code. */
void updateVarInterval(t_cfgReg *var, int counter, t_listNode **intervals)
{
  t_listNode *element_found;
  t_liveInterval *interval_found;
  t_liveInterval pattern;

  if (var->ID == RA_EXCLUDED_VARIABLE)
    return;

  pattern.varID = var->ID;
  /* search for the current live interval */
  element_found =
      listFindWithCallback(*intervals, &pattern, compareLiveIntIDs);
  if (element_found != NULL) {
    interval_found = (t_liveInterval *)element_found->data;
    /* update the interval informations */
    if (interval_found->startPoint > counter)
      interval_found->startPoint = counter;
    if (interval_found->endPoint < counter)
      interval_found->endPoint = counter;
  } else {
    /* we have to add a new live interval */
    interval_found =
        newLiveInterval(var->ID, var->mcRegWhitelist, counter, counter);
    *intervals = listInsert(*intervals, interval_found, -1);
  }
}

/* Use liveness information to update the list of live intervals. Returns an
 * error code. */
void updateListOfIntervals(
    t_listNode **result, t_cfgNode *current_node, int counter)
{
  t_listNode *current_element;
  t_cfgReg *current_var;
  int i;

  if (current_node == NULL)
    return;

  current_element = current_node->in;
  while (current_element != NULL) {
    current_var = (t_cfgReg *)current_element->data;

    updateVarInterval(current_var, counter, result);

    /* fetch the next element in the list of live variables */
    current_element = current_element->next;
  }

  current_element = current_node->out;
  while (current_element != NULL) {
    current_var = (t_cfgReg *)current_element->data;

    updateVarInterval(current_var, counter, result);

    /* fetch the next element in the list of live variables */
    current_element = current_element->next;
  }

  for (i = 0; i < CFG_MAX_DEFS; i++) {
    if (current_node->defs[i]) {
      updateVarInterval(current_node->defs[i], counter, result);
    }
  }
}

int getLiveIntervalsNodeCallback(
    t_basicBlock *block, t_cfgNode *node, int nodeIndex, void *context)
{
  t_listNode **list = (t_listNode **)context;
  updateListOfIntervals(list, node, nodeIndex);
  return 0;
}

void deleteLiveIntervals(t_listNode *intervals)
{
  t_listNode *current_element;
  t_liveInterval *current_interval;

  if (intervals == NULL)
    return;

  /* finalize the memory blocks associated with all
   * the live intervals */
  for (current_element = intervals; current_element != NULL;
       current_element = current_element->next) {
    /* fetch the current interval */
    current_interval = (t_liveInterval *)current_element->data;
    if (current_interval != NULL) {
      /* finalize the memory block associated with
       * the current interval */
      deleteLiveInterval(current_interval);
    }
  }

  /* deallocate the list of intervals */
  deleteList(intervals);
}

/* Perform live intervals computation. Returns an error code. */
void getLiveIntervals(t_cfg *graph, t_listNode **result)
{
  // build the list of intervals one instruction at a time
  *result = NULL;
  cfgIterateNodes(graph, (void *)result, getLiveIntervalsNodeCallback);
}

/* Insert a live interval in the register allocator. Returns true on success,
 * false if the interval was already inserted in the list. */
bool insertLiveInterval(t_regAllocator *RA, t_liveInterval *interval)
{
  // test if an interval for the requested variable is already inserted
  if (listFindWithCallback(
          RA->live_intervals, interval, compareLiveIntIDs) != NULL) {
    return false;
  }

  // add the given interval to the list, in order of starting point
  RA->live_intervals =
      listInsertSorted(RA->live_intervals, interval, compareLiveIntStartPoints);

  return true;
}

/* Insert all elements of the intervals list into
 * the register allocator data structure. */
void insertListOfIntervals(t_regAllocator *RA, t_listNode *intervals)
{
  for (t_listNode *current_element = intervals; current_element != NULL;
       current_element = current_element->next) {
    /* Get the current live interval */
    t_liveInterval *interval = (t_liveInterval *)current_element->data;

    /* insert a new live interval */
    bool ok = insertLiveInterval(RA, interval);
    assert(ok && "bug: at least one duplicate live interval");
  }
}

t_listNode *subtractRegisterSets(t_listNode *a, t_listNode *b)
{
  for (; b; b = b->next) {
    a = listFindAndRemove(a, b->data);
  }
  return a;
}

/* Move the elements in list `a` which are also contained in list `b` to the
 * front of the list. */
t_listNode *optimizeRegisterSet(t_listNode *a, t_listNode *b)
{
  for (; b; b = b->next) {
    t_listNode *old;
    if ((old = listFind(a, b->data))) {
      a = listRemoveNode(a, old);
      a = listInsert(a, b->data, 0);
    }
  }
  return a;
}

void initializeRegisterConstraints(t_regAllocator *ra)
{
  t_listNode *i, *j;

  /* Initialize the register constraint set on all variables that don't have
   * one. */
  i = ra->live_intervals;
  for (; i; i = i->next) {
    t_liveInterval *interval = i->data;
    if (interval->mcRegConstraints)
      continue;
    interval->mcRegConstraints = getListOfGenPurposeRegisters();

    /* Scan the variables that are alive together with this variable */
    j = i->next;
    for (; j; j = j->next) {
      t_liveInterval *overlappingIval = j->data;
      if (overlappingIval->startPoint > interval->endPoint)
        break;
      if (!overlappingIval->mcRegConstraints)
        continue;
      if (overlappingIval->startPoint == interval->endPoint) {
        /* An instruction is using interval as a source and overlappingIval
         * as a destination. Optimize the constraint order to allow
         * allocating source and destination to the same register
         * if possible. */
        interval->mcRegConstraints = optimizeRegisterSet(
            interval->mcRegConstraints, overlappingIval->mcRegConstraints);
      } else {
        /* Another variable (defined after this one) wants to be allocated
         * to a restricted set of registers. Punch a hole in the current
         * variable's set of allowed registers to ensure that this is
         * possible. */
        interval->mcRegConstraints = subtractRegisterSets(
            interval->mcRegConstraints, overlappingIval->mcRegConstraints);
      }
    }
  }
}

int handleCallerSaveRegistersNodeCallback(
    t_basicBlock *block, t_cfgNode *node, int nodeIndex, void *context)
{
  t_regAllocator *ra = (t_regAllocator *)context;
  t_listNode *li_ival;
  t_listNode *clobbered_regs;
  int i;

  if (!isCallInstruction(node->instr))
    return 0;

  clobbered_regs = getListOfCallerSaveRegisters();
  for (i = 0; i < CFG_MAX_DEFS; i++) {
    if (node->defs[i] != NULL)
      clobbered_regs =
          subtractRegisterSets(clobbered_regs, node->defs[i]->mcRegWhitelist);
  }
  for (i = 0; i < CFG_MAX_USES; i++) {
    if (node->uses[i] != NULL)
      clobbered_regs =
          subtractRegisterSets(clobbered_regs, node->uses[i]->mcRegWhitelist);
  }

  li_ival = ra->live_intervals;
  while (li_ival) {
    t_liveInterval *ival = li_ival->data;

    if (ival->startPoint <= nodeIndex && nodeIndex <= ival->endPoint) {
      ival->mcRegConstraints =
          subtractRegisterSets(ival->mcRegConstraints, clobbered_regs);
    }

    li_ival = li_ival->next;
  }

  return 0;
}

/* Restrict register constraints in order to avoid register corrupted by
 * function calls. */
void handleCallerSaveRegisters(t_regAllocator *ra, t_cfg *cfg)
{
  cfgIterateNodes(cfg, (void *)ra, handleCallerSaveRegistersNodeCallback);
}

bool compareFreeRegListNodes(void *freeReg, void *constraintReg)
{
  return INT_TO_LIST_DATA(constraintReg) == INT_TO_LIST_DATA(freeReg);
}

/* Get a new register from the free list */
t_regID assignRegister(t_regAllocator *RA, t_listNode *constraints)
{
  t_regID regID;
  t_listNode *i;

  if (constraints == NULL)
    return RA_SPILL_REQUIRED;

  for (i = constraints; i; i = i->next) {
    t_listNode *freeReg;

    regID = (t_regID)LIST_DATA_TO_INT(i->data);
    freeReg = listFindWithCallback(
        RA->freeRegisters, INT_TO_LIST_DATA(regID), compareFreeRegListNodes);
    if (freeReg) {
      RA->freeRegisters = listRemoveNode(RA->freeRegisters, freeReg);
      return regID;
    }
  }

  return RA_SPILL_REQUIRED;
}

/* Perform a spill that allows the allocation of the given
 * interval, given the list of active live intervals */
t_listNode *spillAtInterval(
    t_regAllocator *RA, t_listNode *active_intervals, t_liveInterval *interval)
{
  t_listNode *last_element;
  t_liveInterval *last_interval;

  /* get the last element of the list of active intervals */
  /* Precondition: if the list of active intervals is empty
   * we are working on a machine with 0 registers available
   * for the register allocation */
  if (active_intervals == NULL) {
    RA->bindings[interval->varID] = RA_SPILL_REQUIRED;
    return active_intervals;
  }

  last_element = listGetLastNode(active_intervals);
  last_interval = (t_liveInterval *)last_element->data;

  /* If the current interval ends before the last one, spill
   * the last one, otherwise spill the current interval. */
  if (last_interval->endPoint > interval->endPoint) {
    t_regID attempt = RA->bindings[last_interval->varID];
    if (listFind(interval->mcRegConstraints, INT_TO_LIST_DATA(attempt))) {
      RA->bindings[interval->varID] = RA->bindings[last_interval->varID];
      RA->bindings[last_interval->varID] = RA_SPILL_REQUIRED;

      active_intervals = listFindAndRemove(active_intervals, last_interval);

      active_intervals =
          listInsertSorted(active_intervals, interval, compareLiveIntEndPoints);
      return active_intervals;
    }
  }

  RA->bindings[interval->varID] = RA_SPILL_REQUIRED;
  return active_intervals;
}

/* Remove from active_intervals all the live intervals that end before the
 * beginning of the current live interval */
t_listNode *expireOldIntervals(
    t_regAllocator *RA, t_listNode *active_intervals, t_liveInterval *interval)
{
  /* No active intervals, bail out! */
  if (active_intervals == NULL)
    return NULL;

  /* Iterate over the set of active intervals */
  t_listNode *current_element = active_intervals;
  while (current_element != NULL) {
    /* Get the live interval */
    t_liveInterval *current_interval = (t_liveInterval *)current_element->data;

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
      t_regID curIntReg = RA->bindings[current_interval->varID];
      if (curIntReg >= 0) {
        t_listNode *allocated =
            listInsert(NULL, INT_TO_LIST_DATA(curIntReg), 0);
        interval->mcRegConstraints =
            optimizeRegisterSet(interval->mcRegConstraints, allocated);
        deleteList(allocated);
      }
    }

    /* Get the next live interval */
    t_listNode *next_element = current_element->next;

    /* Remove the current element from the list */
    active_intervals =
        listFindAndRemove(active_intervals, current_interval);

    /* Free all the registers associated with the removed interval */
    RA->freeRegisters = listInsert(RA->freeRegisters,
        INT_TO_LIST_DATA(RA->bindings[current_interval->varID]), 0);

    /* Step to the next interval */
    current_element = next_element;
  }

  /* Return the updated list of active intervals */
  return active_intervals;
}

/* Deallocate the register allocator data structures */
void deleteRegAllocator(t_regAllocator *RA)
{
  if (RA == NULL)
    return;

  /* Free the live intervals */
  deleteLiveIntervals(RA->live_intervals);

  /* Free memory used for the variable/register bindings */
  free(RA->bindings);
  deleteList(RA->freeRegisters);

  free(RA);
}

/* Allocate and initialize the register allocator */
t_regAllocator *newRegAllocator(t_cfg *graph)
{
  /* allocate memory for a new instance of `t_regAllocator' */
  t_regAllocator *result = (t_regAllocator *)calloc(1, sizeof(t_regAllocator));
  if (result == NULL)
    fatalError("out of memory");

  /* retrieve the max identifier from each live interval */
  t_regID max_var_ID = 0;
  t_listNode *current_cflow_var = graph->registers;
  while (current_cflow_var != NULL) {
    /* fetch the data informations about a variable */
    t_cfgReg *cflow_var = (t_cfgReg *)current_cflow_var->data;

    /* update the value of max_var_ID */
    max_var_ID = MAX(max_var_ID, cflow_var->ID);

    /* retrieve the next variable */
    current_cflow_var = current_cflow_var->next;
  }
  result->varNum = max_var_ID + 1; /* +1 to count R0 */

  /* Assuming there are some variables to associate to regs,
   * allocate space for the binding array, and initialize it */

  /*alloc memory for the array of bindings */
  result->bindings = malloc(sizeof(t_regID) * (size_t)result->varNum);
  if (result->bindings == NULL)
    fatalError("out of memory");

  /* initialize the array of bindings */
  for (int counter = 0; counter < result->varNum; counter++)
    result->bindings[counter] = RA_REGISTER_INVALID;

  /* Liveness analysis: compute the list of live intervals */
  t_listNode *intervals;
  getLiveIntervals(graph, &intervals);

  /* Copy the liveness info into the register allocator, sorting it in the
   * process. */
  if (intervals != NULL) {
    insertListOfIntervals(result, intervals);
    /* deallocate memory used to hold the results of the
     * liveness analysis */
    deleteList(intervals);
  } else {
    result->live_intervals = NULL;
  }

  /* create a list of freeRegisters */
  result->freeRegisters = getListOfRegisters();

  /* Initialize register constraints */
  initializeRegisterConstraints(result);
  handleCallerSaveRegisters(result, graph);

  /* return the new register allocator */
  return result;
}

/* Main register allocation function */
void executeLinearScan(t_regAllocator *RA)
{
  /* Liveness analysis ready? */
  if (RA->live_intervals == NULL)
    return;

  /* initialize the list of active intervals */
  t_listNode *active_intervals = NULL;

  /* Iterate over the list of live intervals */
  for (t_listNode *current_element = RA->live_intervals; current_element != NULL;
       current_element = current_element->next) {
    /* Get the live interval */
    t_liveInterval *current_interval = (t_liveInterval *)current_element->data;

    /* Check which intervals are ended and remove
     * them from the active set, thus freeing registers */
    active_intervals =
        expireOldIntervals(RA, active_intervals, current_interval);

    t_regID reg = assignRegister(RA, current_interval->mcRegConstraints);

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
      active_intervals = listInsertSorted(
          active_intervals, current_interval, compareLiveIntEndPoints);
    }
  }

  /* free the list of active intervals */
  deleteList(active_intervals);
}


/*
 * Materialization
 */

bool compareTempLabelWithRegId(void *a, void *b)
{
  t_tempLabel *lab = (t_tempLabel *)a;
  t_regID reg = *((t_regID *)b);

  if (lab == NULL)
    return 0;
  return lab->regID == reg;
}

t_tempLabel *newTempLabel(t_label *label, t_regID regID)
{
  t_tempLabel *result = malloc(sizeof(t_tempLabel));
  if (result == NULL)
    fatalError("out of memory");

  /* initialize the temp label */
  result->label = label;
  result->regID = regID;
  return result;
}

void deleteListOfTempLabels(t_listNode *tempLabels)
{
  t_listNode *current_element;
  t_tempLabel *tempLabel;

  /* test the preconditions */
  if (tempLabels == NULL)
    return;

  /* free all the list data elements */
  current_element = tempLabels;
  while (current_element != NULL) {
    tempLabel = (t_tempLabel *)current_element->data;
    free(tempLabel);

    current_element = current_element->next;
  }

  /* free the list links */
  deleteList(tempLabels);
}

/* For each spilled variable, this function statically allocates memory for
 * that variable, returns a list of t_templabel structures mapping the
 * spilled variables and the label that points to the allocated memory block.
 * Returns an error code. */
t_listNode *materializeSpillMemory(t_program *program, t_regAllocator *RA)
{
  /* initialize the local variable `result' */
  t_listNode *result = NULL;
  t_tempLabel *tlabel = NULL;

  /* allocate some memory for all spilled temporary variables */
  for (t_regID counter = 0; counter < RA->varNum; counter++) {
    if (RA->bindings[counter] != RA_SPILL_REQUIRED)
      continue;

    /* retrieve a new label */
    t_label *axe_label = createLabel(program);
    tlabel = newTempLabel(axe_label, counter);

    /* statically allocate some room for the spilled variable by
     * creating a new .WORD directive and making the label point to it. */
    genDataDeclaration(program, DATA_WORD, 0, axe_label);

    /* add the current tlabel to the list of labelbindings */
    result = listInsert(result, tlabel, -1);
  }

  return result;
}

int genStoreSpillVariable(t_regID temp_register, t_regID selected_register,
    t_cfg *graph, t_basicBlock *current_block, t_cfgNode *current_node,
    t_listNode *labelBindings, int before)
{
  t_listNode *elementFound =
      listFindWithCallback(labelBindings, &temp_register, compareTempLabelWithRegId);
  if (elementFound == NULL)
    return ERROR_INVALID_CFLOW_GRAPH;

  t_tempLabel *tlabel = (t_tempLabel *)elementFound->data;

  /* create a store instruction */
  t_instruction *storeInstr = genSWGlobal(NULL, selected_register, tlabel->label, REG_T6);
  t_cfgNode *storeNode = createCFGNode(graph, storeInstr);

  /* test if we have to insert the node `storeNode' before `current_node'
   * inside the basic block */
  if (before) {
    bbInsertNodeBefore(current_block, current_node, storeNode);
  } else {
    bbInsertNodeAfter(current_block, current_node, storeNode);
  }

  return NO_ERROR;
}

int genLoadSpillVariable(t_regID temp_register, t_regID selected_register,
    t_cfg *graph, t_basicBlock *block, t_cfgNode *current_node,
    t_listNode *labelBindings, int before)
{
  t_listNode *elementFound =
      listFindWithCallback(labelBindings, &temp_register, compareTempLabelWithRegId);
  if (elementFound == NULL)
    return ERROR_INVALID_CFLOW_GRAPH;

  t_tempLabel *tlabel = (t_tempLabel *)elementFound->data;

  /* create a load instruction */
  t_instruction *loadInstr = genLWGlobal(NULL, selected_register, tlabel->label);
  t_cfgNode *loadNode = createCFGNode(graph, loadInstr);

  if (before) {
    /* insert the node `loadNode' before `current_node' */
    bbInsertNodeBefore(block, current_node, loadNode);
    /* if the `current_node' instruction has a label, move it to the new
     * load instruction */
    if ((current_node->instr)->label != NULL) {
      loadInstr->label = (current_node->instr)->label;
      (current_node->instr)->label = NULL;
    }
  } else {
    bbInsertNodeAfter(block, current_node, loadNode);
  }

  return NO_ERROR;
}

int materializeRegAllocInBBForInstructionNode(t_cfg *graph,
    t_basicBlock *current_block, t_spillState *state, t_cfgNode *current_node,
    t_regAllocator *RA, t_listNode *label_bindings)
{
  int error;
  t_instruction *instr;
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
    argState[num_args].isDestination = 1;
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
      if (state->regs[current_row].assignedVar != argState[current_arg].reg->ID)
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
    for (other_arg = 0; other_arg < current_arg && !alreadyFound; other_arg++) {
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
      return ERROR_REG_ALLOC_ERROR;

    /* If needed, write back the old variable that was assigned to this
     * slot before reassigning it */
    if (state->regs[current_row].needsWB == 1) {
      error = genStoreSpillVariable(state->regs[current_row].assignedVar,
          getSpillRegister(current_row), graph, current_block, current_node,
          label_bindings, 1);
      if (error != NO_ERROR)
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
          getSpillRegister(current_row), graph, current_block, current_node,
          label_bindings, 1);
      if (error != NO_ERROR)
        return error;
    }
  }

  /* rewrite the register identifiers to use the appropriate
   * register number instead of the variable number. */
  for (current_arg = 0; current_arg < num_args; current_arg++) {
    t_instrArg *curReg = argState[current_arg].reg;
    if (argState[current_arg].spillSlot == -1) {
      /* normal case */
      curReg->ID = RA->bindings[curReg->ID];
    } else {
      /* spilled register case */
      curReg->ID = getSpillRegister(argState[current_arg].spillSlot);
    }
  }

  return NO_ERROR;
}

int materializeRegAllocInBB(t_cfg *graph, t_basicBlock *current_block,
    t_regAllocator *RA, t_listNode *label_bindings)
{
  t_spillState state;

  /* initialize the state for this block */
  for (int counter = 0; counter < NUM_SPILL_REGS; counter++) {
    state.regs[counter].assignedVar = REG_INVALID;
    state.regs[counter].needsWB = 0;
  }

  /* iterate through the instructions in the block */
  t_cfgNode *current_node;
  t_listNode *current_nd_element = current_block->nodes;
  while (current_nd_element != NULL) {
    current_node = (t_cfgNode *)current_nd_element->data;

    /* Change the register IDs of the argument of the instruction accoring
     * to the given register allocation. Generate load and stores for spilled
     * registers */
    int error = materializeRegAllocInBBForInstructionNode(
        graph, current_block, &state, current_node, RA, label_bindings);
    if (error)
      return error;

    current_nd_element = current_nd_element->next;
  }

  bool bbHasTermInstr = current_block->nodes &&
      (isJumpInstruction(current_node->instr) ||
          isHaltOrRetInstruction(current_node->instr));

  /* writeback everything at the end of the basic block */
  for (int counter = 0; counter < NUM_SPILL_REGS; counter++) {
    if (state.regs[counter].needsWB == 0)
      continue;
    int error = genStoreSpillVariable(state.regs[counter].assignedVar,
        getSpillRegister(counter), graph, current_block, current_node,
        label_bindings, bbHasTermInstr);
    if (error)
      return error;
  }

  return NO_ERROR;
}

/* update the control flow informations by unsing the result
 * of the register allocation process and a list of bindings
 * between new assembly labels and spilled variables */
int materializeRegAllocInCFG(
    t_cfg *graph, t_regAllocator *RA, t_listNode *label_bindings)
{
  t_listNode *current_bb_element = graph->blocks;
  while (current_bb_element != NULL) {
    t_basicBlock *current_block = (t_basicBlock *)current_bb_element->data;

    int error = materializeRegAllocInBB(graph, current_block, RA, label_bindings);
    if (error)
      return error;

    /* retrieve the next basic block element */
    current_bb_element = current_bb_element->next;
  }

  return NO_ERROR;
}

/* Replace the variable identifiers in the instructions of the CFG with the
 * register assignments in the register allocator. Materialize spilled
 * variables to the scratch registers. All new instructions are inserted
 * in the CFG. Synchronize the list of instructions with the newly
 * modified program. */
int materializeRegisterAllocation(
    t_program *program, t_cfg *graph, t_regAllocator *RA)
{
  /* retrieve a list of t_templabels for the given RA infos and
   * update the content of the data segment. */
  t_listNode *label_bindings = materializeSpillMemory(program, RA);

  /* update the control flow graph with the reg-alloc infos. */
  int error = materializeRegAllocInCFG(graph, RA, label_bindings);
  deleteListOfTempLabels(label_bindings);
  if (error != NO_ERROR)
    return error;

  /* update the code segment informations */
  cfgToProgram(program, graph);

  return NO_ERROR;
}


/*
 *  Debug print utilities
 */

void dumpVariableBindings(t_regID *bindings, int numVars, FILE *fout)
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
      fprintf(
          fout, "Variable T%-3d is assigned to register %s\n", counter, reg);
      free(reg);
    }
  }

  fflush(fout);
}

void dumpLiveIntervals(t_listNode *intervals, FILE *fout)
{
  t_listNode *current_element;
  t_liveInterval *interval;
  t_listNode *i;

  /* precondition */
  if (fout == NULL)
    return;

  /* retireve the first element of the list */
  current_element = intervals;
  while (current_element != NULL) {
    interval = (t_liveInterval *)current_element->data;

    fprintf(fout, "[T%-3d] Live interval: [%3d, %3d]\n", interval->varID,
        interval->startPoint, interval->endPoint);
    fprintf(fout, "       Constraint set: {");

    for (i = interval->mcRegConstraints; i != NULL; i = i->next) {
      char *reg;

      reg = registerIDToString((t_regID)LIST_DATA_TO_INT(i->data), 1);
      fprintf(fout, "%s", reg);
      free(reg);

      if (i->next != NULL)
        fprintf(fout, ", ");
    }
    fprintf(fout, "}\n");

    /* retrieve the next element in the list of intervals */
    current_element = current_element->next;
  }
  fflush(fout);
}

void dumpRegAllocInfos(t_regAllocator *RA, FILE *fout)
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
  dumpLiveIntervals(RA->live_intervals, fout);
  fprintf(fout, "\n");

  fprintf(fout, "----------------------------\n");
  fprintf(fout, " VARIABLE/REGISTER BINDINGS \n");
  fprintf(fout, "----------------------------\n");
  dumpVariableBindings(RA->bindings, RA->varNum, fout);

  fflush(fout);
}


/*
 * Wrapper function
 */

void doRegisterAllocation(t_program *program)
{
  char *logFileName;
  FILE *logFile;

  /* create the control flow graph */
  t_cfg *graph = programToCFG(program);

#ifndef NDEBUG
  logFileName = getLogFileName("controlFlow");
  debugPrintf(" -> Writing the control flow graph to \"%s\"\n", logFileName);
  logFile = fopen(logFileName, "w");
  cfgDump(graph, logFile, false);
  fclose(logFile);
  free(logFileName);
#endif

  cfgComputeLiveness(graph);

#ifndef NDEBUG
  logFileName = getLogFileName("dataFlow");
  debugPrintf(" -> Writing the liveness information to \"%s\"\n", logFileName);
  logFile = fopen(logFileName, "w");
  cfgDump(graph, logFile, true);
  fclose(logFile);
  free(logFileName);
#endif

  /* execute the linear scan algorithm */
  t_regAllocator *RA = newRegAllocator(graph);
  executeLinearScan(RA);

#ifndef NDEBUG
  logFileName = getLogFileName("regAlloc");
  debugPrintf(" -> Writing the register bindings to \"%s\"\n", logFileName);
  logFile = fopen(logFileName, "w");
  dumpRegAllocInfos(RA, logFile);
  fclose(logFile);
  free(logFileName);
#endif

  /* apply changes to the program informations by using the informations
   * of the register allocation process */
  int error = materializeRegisterAllocation(program, graph, RA);
  if (error != NO_ERROR)
    fatalError("register allocation failed with code %d", error);

  deleteRegAllocator(RA);
  deleteCFG(graph);
}
