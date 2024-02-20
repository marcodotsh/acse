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

/// Maximum amount of arguments to an instruction
#define MAX_INSTR_ARGS (CFG_MAX_DEFS + CFG_MAX_USES)

/// Fictitious register ID associated to registers to be spilled.
#define RA_SPILL_REQUIRED    ((t_regID)(-2))
/// Fictitious register ID marking currently unallocated temporaries.
#define RA_REGISTER_INVALID  ((t_regID)(-1))


/// Structure describing a live interval of a register in a program.
typedef struct {
  /// Identifier of the register
  t_regID tempRegID;
  /// List of all physical registers where this temporary register can be
  /// allocated.
  t_listNode *mcRegConstraints;
  /// Index of the first instruction that uses/defines this register
  int startPoint;
  /// Index of the last instruction that uses/defines this register
  int endPoint;
} t_liveInterval;

/// Structure encapsulating the state of the register allocator
typedef struct {
  /// List of live intervals, ordered depending on their start index
  t_listNode *liveIntervals;
  /// Number of temporary registers in the program
  int tempRegNum;
  /// Pointer to a dynamically allocated array which maps every temporary
  /// register to the corresponding physical register.
  /// Temporary registers allocated to a spill location are marked by the
  /// RA_SPILL_REQUIRED virtual register ID.
  t_regID *bindings;
  /// List of currently active intervals during the allocation process.
  t_listNode *activeIntervals;
  /// List of currently free physical registers during the allocation process.
  t_listNode *freeRegisters;
} t_regAllocator;

/// Structure used for mapping a spilled temporary register to the label
/// pointing to its physical storage location in memory.
typedef struct {
  /// The spilled temporary register ID
  t_regID tempRegID;
  /// The label pointing to the spill area
  t_label *label;
} t_spillLocation;

/// Structure representing the current state of an instruction argument during
/// the spill load/store materialization process
typedef struct {
  /// The instruction argument structure
  t_instrArg *reg;
  /// If the register is a destination register
  bool isDestination;
  /// The physical spill register index where the argument will be materialized,
  /// or -1 otherwise.
  int spillSlot;
} t_spillInstrArgState;

/// Structure representing the current state of a spill-reserved register
typedef struct {
  /// Temporary register ID associated to this spill register
  t_regID assignedTempReg;
  /// Non-zero if at least one of the instructions wrote something new into
  /// the spill register, and the value has not been written to the spill
  /// memory location yet.
  bool needsWB;
} t_spillRegState;

/// Spill register slots state
typedef struct {
  /// each array element corresponds to one of the registers reserved for
  /// spilled variables, ordered by ascending register number.
  t_spillRegState regs[NUM_SPILL_REGS];
} t_spillState;


/* Allocate and initialize a live interval data structure with a given
 * temporary register ID, starting and ending points */
t_liveInterval *newLiveInterval(
    t_regID tempRegID, t_listNode *mcRegs, int startPoint, int endPoint)
{
  t_liveInterval *result = malloc(sizeof(t_liveInterval));
  if (result == NULL)
    fatalError("out of memory");

  result->tempRegID = tempRegID;
  result->mcRegConstraints = listClone(mcRegs);
  result->startPoint = startPoint;
  result->endPoint = endPoint;
  return result;
}

/* Deallocate a live interval */
void deleteLiveInterval(t_liveInterval *interval)
{
  if (interval == NULL)
    return;
  
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
bool compareLiveIntWithRegID(void *a, void *b)
{
  t_liveInterval *interval = (t_liveInterval *)a;
  t_regID tempRegID = *((t_regID *)b);
  return interval->tempRegID == tempRegID;
}

/* Update the liveness interval list to account for the fact that variable 'var'
 * is live at index 'counter' in the current program.
 * If the variable already appears in the list, its live interval its prolonged
 * to include the given counter location.
 * Otherwise, a new liveness interval is generated for it.*/
t_listNode *updateIntervalsWithLiveVarAtLocation(t_listNode *intervals, t_cfgReg *var, int counter)
{
  // Search if there's already a liveness interval for the variable
  t_listNode *element_found = listFindWithCallback(intervals, &(var->tempRegID), compareLiveIntWithRegID);

  if (!element_found) {
    // It's not there: add a new interval at the end of the list
    t_liveInterval *interval =
        newLiveInterval(var->tempRegID, var->mcRegWhitelist, counter, counter);
    intervals = listInsert(intervals, interval, -1);
  } else {
    // It's there: update the interval range
    t_liveInterval *interval_found = (t_liveInterval *)element_found->data;
    // Counter should always be increasing!
    assert(interval_found->startPoint <= counter);
    assert(interval_found->endPoint <= counter);
    interval_found->endPoint = counter;
  }
  
  return intervals;
}

/* Add/augment the live interval list with the variables live at a given
 * instruction location in the program */
t_listNode *updateIntervalsWithInstrAtLocation(
    t_listNode *result, t_cfgNode *node, int counter)
{
  t_listNode *elem;

  elem = node->in;
  while (elem != NULL) {
    t_cfgReg *current_var = (t_cfgReg *)elem->data;
    result = updateIntervalsWithLiveVarAtLocation(result, current_var, counter);
    elem = elem->next;
  }

  elem = node->out;
  while (elem != NULL) {
    t_cfgReg *current_var = (t_cfgReg *)elem->data;
    result = updateIntervalsWithLiveVarAtLocation(result, current_var, counter);
    elem = elem->next;
  }

  for (int i = 0; i < CFG_MAX_DEFS; i++) {
    if (node->defs[i])
      result = updateIntervalsWithLiveVarAtLocation(result, node->defs[i], counter);
  }

  return result;
}

int getLiveIntervalsNodeCallback(
    t_basicBlock *block, t_cfgNode *node, int nodeIndex, void *context)
{
  t_listNode **list = (t_listNode **)context;
  *list = updateIntervalsWithInstrAtLocation(*list, node, nodeIndex);
  return 0;
}

/* Collect a list of live intervals from the in/out sets in the CFG.
 * Since cfgIterateNodes passes incrementing counter values to the
 * callback, the list returned from here is already ordered. */
t_listNode *getLiveIntervals(t_cfg *graph)
{
  t_listNode *result = NULL;
  cfgIterateNodes(graph, (void *)&result, getLiveIntervalsNodeCallback);
  return result;
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

t_listNode *subtractRegisterSets(t_listNode *a, t_listNode *b)
{
  for (; b; b = b->next) {
    a = listFindAndRemove(a, b->data);
  }
  return a;
}

/* Create register constraint sets for all temporaries that don't have one.
 * This is the main function that makes register allocation with constraints
 * work.
 *   The idea is that we rely on the fact that all temporaries without
 * constraints are distinguishable from temporaries with constraints.
 * When a temporary *without* constraints A is alive at the same time as a
 * temporary *with* constraints B, we prohibit allocation of A to all the
 * viable registers for B. This guarantees A won't steal a register needed by B.
 *   Of course this will stop working as nicely with multiple overlapping
 * constraints, but in ACSE this doesn't happen.
 *   The effect of this function is */
void initializeRegisterConstraints(t_regAllocator *ra)
{
  t_listNode *i = ra->liveIntervals;
  for (; i; i = i->next) {
    t_liveInterval *interval = i->data;
    // Skip instructions that already have constraints
    if (interval->mcRegConstraints)
      continue;
    // Initial set consists of all registers.
    interval->mcRegConstraints = getListOfGenPurposeMachineRegisters();

    // Scan the temporary registers that are alive together with this one and
    // already have constraints.
    t_listNode *j = i->next;
    for (; j; j = j->next) {
      t_liveInterval *overlappingIval = j->data;
      if (overlappingIval->startPoint > interval->endPoint)
        break;
      if (!overlappingIval->mcRegConstraints)
        continue;
      if (overlappingIval->startPoint == interval->endPoint) {
        // Some instruction is using our temporary register as a source and the
        // other temporary register as a destination. Optimize the constraint
        // order to allow allocating source and destination to the same register
        // if possible.
        interval->mcRegConstraints = optimizeRegisterSet(
            interval->mcRegConstraints, overlappingIval->mcRegConstraints);
      } else {
        // Another variable (defined after this one) wants to be allocated
        // to a restricted set of registers. Punch a hole in the current
        // variable's set of allowed registers to ensure that this is
        // possible.
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

  if (!isCallInstruction(node->instr))
    return 0;

  t_listNode *clobbered_regs = getListOfCallerSaveMachineRegisters();
  for (int i = 0; i < CFG_MAX_DEFS; i++) {
    if (node->defs[i] != NULL)
      clobbered_regs =
          subtractRegisterSets(clobbered_regs, node->defs[i]->mcRegWhitelist);
  }
  for (int i = 0; i < CFG_MAX_USES; i++) {
    if (node->uses[i] != NULL)
      clobbered_regs =
          subtractRegisterSets(clobbered_regs, node->uses[i]->mcRegWhitelist);
  }

  t_listNode *li_ival = ra->liveIntervals;
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

/* Allocate and initialize the register allocator */
t_regAllocator *newRegAllocator(t_cfg *graph)
{
  t_regAllocator *result = (t_regAllocator *)calloc(1, sizeof(t_regAllocator));
  if (result == NULL)
    fatalError("out of memory");

  // Find the maximum temporary register ID in the program, then allocate the
  // array of register bindings with that size. If there are unused register
  // IDs, the array will have holes, but that's not a problem.
  t_regID maxTempRegID = 0;
  t_listNode *current_cflow_var = graph->registers;
  while (current_cflow_var != NULL) {
    t_cfgReg *cflow_var = (t_cfgReg *)current_cflow_var->data;
    maxTempRegID = MAX(maxTempRegID, cflow_var->tempRegID);
    current_cflow_var = current_cflow_var->next;
  }
  result->tempRegNum = maxTempRegID + 1;

  // allocate space for the binding array, and initialize it */
  result->bindings = malloc(sizeof(t_regID) * (size_t)result->tempRegNum);
  if (result->bindings == NULL)
    fatalError("out of memory");
  for (int counter = 0; counter < result->tempRegNum; counter++)
    result->bindings[counter] = RA_REGISTER_INVALID;
  
  // If the target has a special meaning for register zero, allocate it to
  // itself immediately
  if (TARGET_REG_ZERO_IS_CONST)
    result->bindings[REG_0] = REG_0;

  // Compute the ordered list of live intervals
  result->liveIntervals = getLiveIntervals(graph);

  // Create the list of free physical (machine) registers
  result->freeRegisters = getListOfMachineRegisters();

  /* Initialize register constraints */
  initializeRegisterConstraints(result);
  handleCallerSaveRegisters(result, graph);

  /* return the new register allocator */
  return result;
}

bool compareFreeRegListNodes(void *freeReg, void *constraintReg)
{
  return INT_TO_LIST_DATA(constraintReg) == INT_TO_LIST_DATA(freeReg);
}

/* Remove from RA->activeIntervals all the live intervals that end before the
 * beginning of the current live interval */
void expireOldIntervals(t_regAllocator *RA, t_liveInterval *interval)
{
  /* No active intervals, bail out! */
  if (RA->activeIntervals == NULL)
    return;

  /* Iterate over the set of active intervals */
  t_listNode *current_element = RA->activeIntervals;
  while (current_element != NULL) {
    /* Get the live interval */
    t_liveInterval *current_interval = (t_liveInterval *)current_element->data;

    /* If the considered interval ends before the beginning of
     * the current live interval, we don't need to keep track of
     * it anymore; otherwise, this is the first interval we must
     * still take into account when assigning registers. */
    if (current_interval->endPoint > interval->startPoint)
      return;

    /* when current_interval->endPoint == interval->startPoint,
     * the variable associated to current_interval is being used by the
     * instruction that defines interval. As a result, we can allocate
     * interval to the same reg as current_interval. */
    if (current_interval->endPoint == interval->startPoint) {
      t_regID curIntReg = RA->bindings[current_interval->tempRegID];
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
    RA->activeIntervals =
        listFindAndRemove(RA->activeIntervals, current_interval);

    /* Free all the registers associated with the removed interval */
    RA->freeRegisters = listInsert(RA->freeRegisters,
        INT_TO_LIST_DATA(RA->bindings[current_interval->tempRegID]), 0);

    /* Step to the next interval */
    current_element = next_element;
  }
}

/* Get a new register from the free list */
t_regID assignRegister(t_regAllocator *RA, t_listNode *constraints)
{
  t_regID tempRegID;
  t_listNode *i;

  if (constraints == NULL)
    return RA_SPILL_REQUIRED;

  for (i = constraints; i; i = i->next) {
    t_listNode *freeReg;

    tempRegID = (t_regID)LIST_DATA_TO_INT(i->data);
    freeReg = listFindWithCallback(
        RA->freeRegisters, INT_TO_LIST_DATA(tempRegID), compareFreeRegListNodes);
    if (freeReg) {
      RA->freeRegisters = listRemoveNode(RA->freeRegisters, freeReg);
      return tempRegID;
    }
  }

  return RA_SPILL_REQUIRED;
}

/* Perform a spill that allows the allocation of the given
 * interval, given the list of active live intervals */
void spillAtInterval(t_regAllocator *RA, t_liveInterval *interval)
{
  t_listNode *last_element;
  t_liveInterval *last_interval;

  /* get the last element of the list of active intervals */
  /* Precondition: if the list of active intervals is empty
   * we are working on a machine with 0 registers available
   * for the register allocation */
  if (RA->activeIntervals == NULL) {
    RA->bindings[interval->tempRegID] = RA_SPILL_REQUIRED;
    return;
  }

  last_element = listGetLastNode(RA->activeIntervals);
  last_interval = (t_liveInterval *)last_element->data;

  /* If the current interval ends before the last one, spill
   * the last one, otherwise spill the current interval. */
  if (last_interval->endPoint > interval->endPoint) {
    t_regID attempt = RA->bindings[last_interval->tempRegID];
    if (listFind(interval->mcRegConstraints, INT_TO_LIST_DATA(attempt))) {
      RA->bindings[interval->tempRegID] = RA->bindings[last_interval->tempRegID];
      RA->bindings[last_interval->tempRegID] = RA_SPILL_REQUIRED;

      RA->activeIntervals = listFindAndRemove(RA->activeIntervals, last_interval);

      RA->activeIntervals =
          listInsertSorted(RA->activeIntervals, interval, compareLiveIntEndPoints);
      return;
    }
  }

  RA->bindings[interval->tempRegID] = RA_SPILL_REQUIRED;
}

/* Main register allocation function */
void executeLinearScan(t_regAllocator *RA)
{
  /* Iterate over the list of live intervals */
  for (t_listNode *current_element = RA->liveIntervals; current_element != NULL;
       current_element = current_element->next) {
    /* Get the live interval */
    t_liveInterval *current_interval = (t_liveInterval *)current_element->data;

    /* Check which intervals are ended and remove
     * them from the active set, thus freeing registers */
    expireOldIntervals(RA, current_interval);

    t_regID reg = assignRegister(RA, current_interval->mcRegConstraints);

    /* If all registers are busy, perform a spill */
    if (reg == RA_SPILL_REQUIRED) {
      /* perform a spill */
      spillAtInterval(RA, current_interval);
    } else {
      /* Otherwise, assign a new register to the current live interval */
      RA->bindings[current_interval->tempRegID] = reg;

      /* Add the current interval to the list of active intervals, in
       * order of ending points (to allow easier expire management) */
      RA->activeIntervals = listInsertSorted(
          RA->activeIntervals, current_interval, compareLiveIntEndPoints);
    }
  }

  /* free the list of active intervals */
  RA->activeIntervals = deleteList(RA->activeIntervals);
}

/* Deallocate the register allocator data structures */
void deleteRegAllocator(t_regAllocator *RA)
{
  if (RA == NULL)
    return;

  /* finalize the memory blocks associated with all
   * the live intervals */
  for (t_listNode *current_element = RA->liveIntervals; current_element != NULL;
       current_element = current_element->next) {
    /* fetch the current interval */
    t_liveInterval *current_interval = (t_liveInterval *)current_element->data;
    if (current_interval != NULL) {
      /* finalize the memory block associated with
       * the current interval */
      deleteLiveInterval(current_interval);
    }
  }

  /* deallocate the list of intervals */
  deleteList(RA->liveIntervals);

  /* Free memory used for the variable/register bindings */
  free(RA->bindings);
  deleteList(RA->activeIntervals);
  deleteList(RA->freeRegisters);

  free(RA);
}


/*
 * Materialization
 */

bool compareTempLabelWithRegId(void *a, void *b)
{
  t_spillLocation *lab = (t_spillLocation *)a;
  t_regID reg = *((t_regID *)b);

  if (lab == NULL)
    return 0;
  return lab->tempRegID == reg;
}

t_spillLocation *newTempLabel(t_label *label, t_regID tempRegID)
{
  t_spillLocation *result = malloc(sizeof(t_spillLocation));
  if (result == NULL)
    fatalError("out of memory");

  /* initialize the temp label */
  result->label = label;
  result->tempRegID = tempRegID;
  return result;
}

void deleteListOfTempLabels(t_listNode *tempLabels)
{
  t_listNode *current_element;
  t_spillLocation *tempLabel;

  /* test the preconditions */
  if (tempLabels == NULL)
    return;

  /* free all the list data elements */
  current_element = tempLabels;
  while (current_element != NULL) {
    tempLabel = (t_spillLocation *)current_element->data;
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
  t_spillLocation *tlabel = NULL;

  /* allocate some memory for all spilled temporary variables */
  for (t_regID counter = 0; counter < RA->tempRegNum; counter++) {
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

void genStoreSpillVariable(t_regID temp_register, t_regID selected_register,
    t_cfg *graph, t_basicBlock *current_block, t_cfgNode *current_node,
    t_listNode *labelBindings, bool before)
{
  t_listNode *elementFound =
      listFindWithCallback(labelBindings, &temp_register, compareTempLabelWithRegId);
  if (elementFound == NULL)
    fatalError("bug: t%d missing from the spill label list", temp_register);

  t_spillLocation *tlabel = (t_spillLocation *)elementFound->data;

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
}

void genLoadSpillVariable(t_regID temp_register, t_regID selected_register,
    t_cfg *graph, t_basicBlock *block, t_cfgNode *current_node,
    t_listNode *labelBindings, bool before)
{
  t_listNode *elementFound =
      listFindWithCallback(labelBindings, &temp_register, compareTempLabelWithRegId);
  if (elementFound == NULL)
    fatalError("bug: t%d missing from the spill label list", temp_register);

  t_spillLocation *tlabel = (t_spillLocation *)elementFound->data;

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
}

void materializeRegAllocInBBForInstructionNode(t_cfg *graph,
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
  t_spillInstrArgState argState[MAX_INSTR_ARGS];

  /* fetch the current instruction */
  instr = current_node->instr;

  /* initialize the array of arguments to the instruction */
  num_args = 0;
  if (instr->reg_dest) {
    argState[num_args].reg = instr->reg_dest;
    argState[num_args].isDestination = true;
    argState[num_args].spillSlot = -1;
    num_args++;
  }
  if (instr->reg_src1) {
    argState[num_args].reg = instr->reg_src1;
    argState[num_args].isDestination = false;
    argState[num_args].spillSlot = -1;
    num_args++;
  }
  if (instr->reg_src2) {
    argState[num_args].reg = instr->reg_src2;
    argState[num_args].isDestination = false;
    argState[num_args].spillSlot = -1;
    num_args++;
  }

  /* Test if a requested variable is already loaded into a register
   * from a previous instruction. */
  for (current_arg = 0; current_arg < num_args; current_arg++) {
    if (RA->bindings[argState[current_arg].reg->ID] != RA_SPILL_REQUIRED)
      continue;

    for (current_row = 0; current_row < NUM_SPILL_REGS; current_row++) {
      if (state->regs[current_row].assignedTempReg != argState[current_arg].reg->ID)
        continue;

      /* update the value of used_Register */
      argState[current_arg].spillSlot = current_row;

      /* update the value of `assignedRegisters` */
      /* set currently used flag */
      spillSlotInUse[current_row] = 1;

      /* test if a write back is needed. Writebacks are needed
       * when an instruction modifies a spilled register. */
      if (argState[current_arg].isDestination)
        state->regs[current_row].needsWB = true;

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
      fatalError("bug: spill slots exhausted");

    /* If needed, write back the old variable that was assigned to this
     * slot before reassigning it */
    if (state->regs[current_row].needsWB) {
      genStoreSpillVariable(state->regs[current_row].assignedTempReg,
          getSpillMachineRegister(current_row), graph, current_block, current_node,
          label_bindings, true);
    }

    /* Update the state of this spill slot */
    spillSlotInUse[current_row] = 1;
    argState[current_arg].spillSlot = current_row;
    state->regs[current_row].assignedTempReg = argState[current_arg].reg->ID;
    state->regs[current_row].needsWB = argState[current_arg].isDestination;

    /* Load the value of the variable in the spill register if not a
     * destination of the instruction */
    if (!argState[current_arg].isDestination) {
      genLoadSpillVariable(argState[current_arg].reg->ID,
          getSpillMachineRegister(current_row), graph, current_block, current_node,
          label_bindings, true);
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
      curReg->ID = getSpillMachineRegister(argState[current_arg].spillSlot);
    }
  }
}

void materializeRegAllocInBB(t_cfg *graph, t_basicBlock *current_block,
    t_regAllocator *RA, t_listNode *label_bindings)
{
  t_spillState state;

  /* initialize the state for this block */
  for (int counter = 0; counter < NUM_SPILL_REGS; counter++) {
    state.regs[counter].assignedTempReg = REG_INVALID;
    state.regs[counter].needsWB = false;
  }

  /* iterate through the instructions in the block */
  t_cfgNode *current_node;
  t_listNode *current_nd_element = current_block->nodes;
  while (current_nd_element != NULL) {
    current_node = (t_cfgNode *)current_nd_element->data;

    /* Change the register IDs of the argument of the instruction accoring
     * to the given register allocation. Generate load and stores for spilled
     * registers */
    materializeRegAllocInBBForInstructionNode(
        graph, current_block, &state, current_node, RA, label_bindings);

    current_nd_element = current_nd_element->next;
  }

  bool bbHasTermInstr = current_block->nodes &&
      (isJumpInstruction(current_node->instr) ||
          isHaltOrRetInstruction(current_node->instr));

  /* writeback everything at the end of the basic block */
  for (int counter = 0; counter < NUM_SPILL_REGS; counter++) {
    if (state.regs[counter].needsWB == false)
      continue;
    genStoreSpillVariable(state.regs[counter].assignedTempReg,
        getSpillMachineRegister(counter), graph, current_block, current_node,
        label_bindings, bbHasTermInstr);
  }
}

/* update the control flow informations by unsing the result
 * of the register allocation process and a list of bindings
 * between new assembly labels and spilled variables */
void materializeRegAllocInCFG(
    t_cfg *graph, t_regAllocator *RA, t_listNode *label_bindings)
{
  t_listNode *current_bb_element = graph->blocks;
  while (current_bb_element != NULL) {
    t_basicBlock *current_block = (t_basicBlock *)current_bb_element->data;

    materializeRegAllocInBB(graph, current_block, RA, label_bindings);

    /* retrieve the next basic block element */
    current_bb_element = current_bb_element->next;
  }
}

/* Replace the variable identifiers in the instructions of the CFG with the
 * register assignments in the register allocator. Materialize spilled
 * variables to the scratch registers. All new instructions are inserted
 * in the CFG. Synchronize the list of instructions with the newly
 * modified program. */
void materializeRegisterAllocation(
    t_program *program, t_cfg *graph, t_regAllocator *RA)
{
  /* retrieve a list of t_templabels for the given RA infos and
   * update the content of the data segment. */
  t_listNode *label_bindings = materializeSpillMemory(program, RA);

  /* update the control flow graph with the reg-alloc infos. */
  materializeRegAllocInCFG(graph, RA, label_bindings);
  deleteListOfTempLabels(label_bindings);

  /* update the code segment informations */
  cfgToProgram(program, graph);
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

  for (counter = 0; counter < numVars; counter++) {
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

    fprintf(fout, "[T%-3d] Live interval: [%3d, %3d]\n", interval->tempRegID,
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
  fprintf(fout, "Number of used variables: %d\n\n", RA->tempRegNum);

  fprintf(fout, "----------------\n");
  fprintf(fout, " LIVE INTERVALS \n");
  fprintf(fout, "----------------\n");
  dumpLiveIntervals(RA->liveIntervals, fout);
  fprintf(fout, "\n");

  fprintf(fout, "----------------------------\n");
  fprintf(fout, " VARIABLE/REGISTER BINDINGS \n");
  fprintf(fout, "----------------------------\n");
  dumpVariableBindings(RA->bindings, RA->tempRegNum, fout);

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
  materializeRegisterAllocation(program, graph, RA);

  deleteRegAllocator(RA);
  deleteCFG(graph);
}
