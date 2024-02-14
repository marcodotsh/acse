/// @file cflow_graph.c

#include <assert.h>
#include "cflow_graph.h"
#include "list.h"
#include "utils.h"
#include "utils.h"
#include "target_info.h"
#include "target_asm_print.h"


static bool compareCFGVariables(void *a, void *b)
{
  t_cfgReg *varA = (t_cfgReg *)a;
  t_cfgReg *varB = (t_cfgReg *)b;

  if (a == NULL && b == NULL)
    return true;
  if (a == NULL || b == NULL)
    return false;

  return varA->ID == varB->ID;
}

/* Alloc a new control flow graph variable object. If a variable object
 * referencing the same identifier already exists, returns the pre-existing
 * object. */
t_cfgReg *cfgCreateVariable(t_cfg *graph, t_regID identifier, t_listNode *mcRegs)
{
  assert(graph != NULL);

  // alloc memory for a variable information
  t_cfgReg *result = malloc(sizeof(t_cfgReg));
  if (result == NULL)
    fatalError("out of memory");

  // update the value of result
  result->ID = identifier;
  result->mcRegWhitelist = NULL;

  // test if a variable with the same identifier was already present
  t_listNode *elementFound = findElementWithCallback(
      graph->cflow_variables, result, compareCFGVariables);

  if (elementFound == NULL) {
    // update the set of variables
    graph->cflow_variables = addElement(graph->cflow_variables, result, -1);
  } else {
    free(result);
    result = (t_cfgReg *)elementFound->data;
    assert(result != NULL);
    assert(result->ID == identifier);
  }

  // copy the machine register allocation constraint, or compute the
  // intersection between the register allocation constraint sets
  if (mcRegs) {
    if (result->mcRegWhitelist == NULL) {
      result->mcRegWhitelist = cloneList(mcRegs);
    } else {
      t_listNode *thisReg = result->mcRegWhitelist;
      while (thisReg) {
        t_listNode *nextReg = thisReg->next;
        if (!findElement(mcRegs, thisReg->data)) {
          result->mcRegWhitelist =
              removeElement(result->mcRegWhitelist, thisReg);
        }
        thisReg = nextReg;
      }
      assert(result->mcRegWhitelist);
    }
  }

  return result;
}

void cfgComputeDefUses(t_cfg *graph, t_cfgNode *node)
{
  // preconditions
  assert(graph != NULL);
  assert(node != NULL);
  assert(node->instr != NULL);

  // update the value of `instr' 
  t_instruction *instr = node->instr;

  // initialize the values of varDest, varSource1 and varSource2 
  t_cfgReg *varDest = NULL;
  t_cfgReg *varSource1 = NULL;
  t_cfgReg *varSource2 = NULL;

  // update the values of the variables 
  if (instr->reg_dest != NULL) {
    varDest = cfgCreateVariable(
        graph, (instr->reg_dest)->ID, instr->reg_dest->mcRegWhitelist);
  }
  if (instr->reg_src1 != NULL) {
    varSource1 = cfgCreateVariable(
        graph, (instr->reg_src1)->ID, instr->reg_src1->mcRegWhitelist);
  }
  if (instr->reg_src2 != NULL) {
    varSource2 = cfgCreateVariable(
        graph, (instr->reg_src2)->ID, instr->reg_src2->mcRegWhitelist);
  }

  int def_i = 0;
  int use_i = 0;

  if (varDest)
    node->defs[def_i++] = varDest;

  if (varSource1)
    node->uses[use_i++] = varSource1;
  if (varSource1)
    node->uses[use_i++] = varSource2;
}

t_cfgNode *cfgCreateNode(t_cfg *graph, t_instruction *instr)
{
  assert(graph != NULL);
  assert(instr != NULL);

  // create a new instance of type `t_cflow_node' 
  t_cfgNode *result = malloc(sizeof(t_cfgNode));
  if (result == NULL)
    fatalError("out of memory");

  // initialize result 
  for (int i = 0; i < CFG_MAX_DEFS; i++)
    result->defs[i] = NULL;
  for (int i = 0; i < CFG_MAX_USES; i++)
    result->uses[i] = NULL;
  result->instr = instr;

  // set the def-uses for the current node 
  cfgComputeDefUses(graph, result);

  // initialize the list of variables that are live in
  // and live out from the current node
  result->in = NULL;
  result->out = NULL;

  // return the node
  return result;
}

void deleteCFGNode(t_cfgNode *node)
{
  if (node == NULL)
    return;

  // free the two lists `in' and `out' 
  if (node->in != NULL)
    freeList(node->in);
  if (node->out != NULL)
    freeList(node->out);

  // free the current node 
  free(node);
}

t_basicBlock *newBasicBlock(void)
{
  t_basicBlock *result = malloc(sizeof(t_basicBlock));
  if (result == NULL)
    fatalError("out of memory");
  result->pred = NULL;
  result->succ = NULL;
  result->nodes = NULL;
  return result;
}

void deleteBasicBlock(t_basicBlock *block)
{
  if (block == NULL)
    return;

  if (block->pred != NULL)
    freeList(block->pred);
  if (block->succ != NULL)
    freeList(block->succ);

  t_listNode *current_element = block->nodes;
  while (current_element != NULL) {
    t_cfgNode *current_node = (t_cfgNode *)current_element->data;
    deleteCFGNode(current_node);
    current_element = current_element->next;
  }

  freeList(block->nodes);
  free(block);
}

void bbAddPred(t_basicBlock *block, t_basicBlock *pred)
{
  assert(block != NULL);
  assert(pred != NULL);

  // do not insert if the block is already inserted in the list of predecessors 
  if (findElement(block->pred, pred) == NULL) {
    block->pred = addElement(block->pred, pred, -1);
    pred->succ = addElement(pred->succ, block, -1);
  }
}

void bbAddSucc(t_basicBlock *block, t_basicBlock *succ)
{
  assert(block != NULL);
  assert(succ != NULL);

  // do not insert if the node is already inserted in the list of successors 
  if (findElement(block->succ, succ) == NULL) {
    block->succ = addElement(block->succ, succ, -1);
    succ->pred = addElement(succ->pred, block, -1);
  }
}

t_cfgError bbInsertNode(t_basicBlock *block, t_cfgNode *node)
{
  assert(block != NULL);
  assert(node != NULL && node->instr != NULL);

  if (findElement(block->nodes, node) != NULL)
    return CFG_ERROR_NODE_ALREADY_INSERTED;
  
  block->nodes = addElement(block->nodes, node, -1);
  return CFG_NO_ERROR;
}

t_cfgError bbInsertNodeBefore(
    t_basicBlock *block, t_cfgNode *before_node, t_cfgNode *new_node)
{
  assert(block != NULL);
  assert(new_node != NULL && new_node->instr != NULL && before_node != NULL);

  t_listNode *before_node_elem = findElement(block->nodes, before_node);
  if (before_node_elem == NULL)
    return CFG_ERROR_INVALID_NODE;

  if (findElement(block->nodes, new_node) != NULL)
    return CFG_ERROR_NODE_ALREADY_INSERTED;

  block->nodes = addBefore(block->nodes, before_node_elem, new_node);
  return CFG_NO_ERROR;
}

/* insert a new node without updating the dataflow informations */
t_cfgError bbInsertNodeAfter(
    t_basicBlock *block, t_cfgNode *after_node, t_cfgNode *new_node)
{
  assert(block != NULL);
  assert(new_node != NULL && new_node->instr != NULL && after_node != NULL);

  t_listNode *after_node_elem = findElement(block->nodes, after_node);
  if (after_node_elem == NULL)
    return CFG_ERROR_INVALID_NODE;

  if (findElement(block->nodes, new_node) != NULL)
    return CFG_ERROR_NODE_ALREADY_INSERTED;

  block->nodes = addAfter(block->nodes, after_node_elem, new_node);
  return CFG_NO_ERROR;
}

/* allocate memory for a control flow graph */
static t_cfg *newCFG(void)
{
  t_cfg *result = malloc(sizeof(t_cfg));
  if (result == NULL)
    fatalError("out of memory");
  result->blocks = NULL;
  result->cflow_variables = NULL;
  // Create the dummy ending block.
  result->endingBlock = newBasicBlock();
  return result;
}

/* finalize the memory associated with the given control flow graph */
void deleteCFG(t_cfg *graph)
{
  if (graph == NULL)
    return;

  t_listNode *current_element = graph->blocks;
  while (current_element != NULL) {
    t_basicBlock *current_block = (t_basicBlock *)current_element->data;
    assert(current_block != NULL);

    deleteBasicBlock(current_block);

    current_element = current_element->next;
  }

  if (graph->blocks != NULL)
    freeList(graph->blocks);
  if (graph->endingBlock != NULL)
    deleteBasicBlock(graph->endingBlock);
  if (graph->cflow_variables != NULL) {
    t_listNode *current_element = graph->cflow_variables;
    while (current_element != NULL) {
      t_cfgReg *current_variable = (t_cfgReg *)current_element->data;

      if (current_variable != NULL) {
        freeList(current_variable->mcRegWhitelist);
        free(current_variable);
      }

      current_element = current_element->next;
    }

    freeList(graph->cflow_variables);
  }

  free(graph);
}

/* look up for a label inside the graph */
static t_basicBlock *cfgSearchLabel(t_cfg *graph, t_label *label)
{
  assert(graph != NULL);
  if (label == NULL)
    return NULL;

  t_basicBlock *bblock = NULL;
  t_listNode *current_element = graph->blocks;
  while (current_element != NULL) {
    bblock = (t_basicBlock *)current_element->data;
    assert(bblock != NULL);
    assert(bblock->nodes != NULL);

    // Check the first node of the basic block. If its instruction has a label,
    // check if it's the label we are searching fore.
    t_cfgNode *current_node = (t_cfgNode *)bblock->nodes->data;
    assert(current_node != NULL);
    if ((current_node->instr)->label != NULL) {
      if (compareLabels((current_node->instr)->label, label))
        // Found!
        break;
    }

    current_element = current_element->next;
  }

  return bblock;
}

/* test if the current instruction `instr' is a labelled instruction */
bool instrIsStartingNode(t_instruction *instr)
{
  assert(instr != NULL);
  return instr->label != NULL;
}

/* test if the current instruction will end a basic block */
bool instrIsEndingNode(t_instruction *instr)
{
  assert(instr != NULL);
  return isHaltOrRetInstruction(instr) || isJumpInstruction(instr);
}

t_cfgError cfgInsertBlock(t_cfg *graph, t_basicBlock *block)
{
  assert(graph != NULL);
  assert(block != NULL);

  if (findElement(graph->blocks, block) != NULL)
    return CFG_ERROR_BBLOCK_ALREADY_INSERTED;

  graph->blocks = addElement(graph->blocks, block, -1);
  return CFG_NO_ERROR;
}

static void cfgComputeTransitions(t_cfg *graph)
{
  // This function is the continuation of programToCFG(), where after creating
  // the blocks in the CFG we need to construct the transitions between them.
  //   After the basic block construction, all branch instructions are now
  // found at the end of (some of the) basic blocks. The algorithm for adding
  // the transitions simply consists of searching for every branch, and adding
  // the correct outgoing edges to its basic block.
  t_listNode *current_element = graph->blocks;
  while (current_element != NULL) {
    t_basicBlock *current_block = (t_basicBlock *)current_element->data;
    assert(current_block != NULL);
    assert(current_block->nodes != NULL);

    // Get the last instruction in the basic block
    t_listNode *last_element = getLastElement(current_block->nodes);
    assert(last_element != NULL);
    t_cfgNode *last_node = (t_cfgNode *)last_element->data;
    assert(last_node != NULL);
    t_instruction *last_instruction = last_node->instr;
    assert(last_instruction != NULL);

    // If the instruction is return-like of exit-like, by definition the next
    // block is the ending block because it stops the program/subroutine.
    if (isHaltOrRetInstruction(last_instruction)) {
      bbAddSucc(current_block, graph->endingBlock);
      bbAddPred(graph->endingBlock, current_block);
      continue;
    }

    // All branch/jump instructions may transfer their control to the code
    // indicated by their label argument, so add edges appropriately.
    if (isJumpInstruction(last_instruction)) {
      if (last_instruction->addressParam == NULL)
        fatalError("malformed jump instruction with no label in CFG");
      t_basicBlock *jumpBlock = cfgSearchLabel(graph, last_instruction->addressParam);
      if (jumpBlock == NULL)
        fatalError("malformed jump instruction with invalid label in CFG");
      bbAddPred(jumpBlock, current_block);
      bbAddSucc(current_block, jumpBlock);
    }

    // Additionally, conditional jumps may also not be taken, and in that
    // case the execution continues to the next instruction.
    //   As the order of the blocks in the block list reflects the order of
    // the instructions in the program, we can rely on this property to fetch
    // the correct block for this fallthrough case.
    if (!isUnconditionalJump(last_instruction)) {
      t_listNode *next_element = current_element->next;
      if (next_element != NULL) {
        // The current basic block has a successor in the list, all is fine
        t_basicBlock *nextBlock = next_element->data;
        assert(nextBlock != NULL);
        bbAddSucc(current_block, nextBlock);
        bbAddPred(nextBlock, current_block);
      } else {
        // If this is the last basic block in the list, the next block is
        // the ending block (which exists outside the list)
        bbAddSucc(current_block, graph->endingBlock);
        bbAddPred(graph->endingBlock, current_block);
      }
    }

    current_element = current_element->next;
  }
}

t_cfg *programToCFG(t_program *program)
{
  assert(program != NULL);
  t_listNode *instructions = program->instructions;

  // alloc memory for a new control flow graph
  t_cfg *result = newCFG();

  // Generate each basic block, by splitting the list of instruction at each
  // terminator instruction or labeled instruction. Labeled instructions are
  // instructions with a label assigned to them. Terminator instructions are
  // branch-like instructions: branches themselves, but also any instruction
  // used for subroutine return.
  //   The `bblock' variable contains a basic block we are adding instructions
  // to, or NULL if we have just found the end of the last basic block and we
  // are not sure whether to insert a new one. When `bblock' is NULL, a new
  // block is created lazily at the next instruction found. This ensures no
  // empty blocks are created.
  t_basicBlock *bblock = NULL;
  t_listNode *current_element = instructions;
  while (current_element != NULL) {
    t_instruction *current_instr = (t_instruction *)current_element->data;
    assert(current_instr != NULL);

    // create the next node to insert in the block
    t_cfgNode *current_node = cfgCreateNode(result, current_instr);

    // If the instruction node needs to be at the beginning of a basic block
    // (= is labeled) or if `bblock' is NULL (because the last instruction was
    // a terminator) then create a new basic block.
    if (instrIsStartingNode(current_instr) || bblock == NULL) {
      bblock = newBasicBlock();
      // Add the block to the graph.
      t_cfgError error = cfgInsertBlock(result, bblock);
      assert(error == CFG_NO_ERROR && "cfgInsertNode failed, corrupt CFG?");
    }

    // Add the instruction to the current basic block
    t_cfgError error = bbInsertNode(bblock, current_node);
    assert(error == CFG_NO_ERROR && "bbInsertNode failed, corrupt CFG?");

    // If the instruction is a basic block terminator, set `bblock' to NULL
    // to stop inserting nodes into it.
    if (instrIsEndingNode(current_instr))
      bblock = NULL;

    current_element = current_element->next;
  }

  // Now all the blocks have been created, we need to add the edges between
  // blocks, which is done in the cfgComputeTransitions() function.
  cfgComputeTransitions(result);
  return result;
}

int cfgIterateNodes(t_cfg *graph, void *context,
    int (*callback)(
        t_basicBlock *block, t_cfgNode *node, int nodeIndex, void *context))
{
  int counter = 0;
  int exitcode = 0;

  t_listNode *current_bb_element = graph->blocks;
  while (current_bb_element != NULL) {
    t_basicBlock *current_block = (t_basicBlock *)current_bb_element->data;

    t_listNode *current_nd_element = current_block->nodes;
    while (current_nd_element != NULL) {
      t_cfgNode *current_node = (t_cfgNode *)current_nd_element->data;

      exitcode = callback(current_block, current_node, counter, context);
      if (exitcode != 0)
        return exitcode;

      counter++;
      current_nd_element = current_nd_element->next;
    }

    current_bb_element = current_bb_element->next;
  }
  return exitcode;
}

void cfgToProgram(t_program *program, t_cfg *graph)
{
  assert(program != NULL);
  assert(graph != NULL);

  // Erase the old code segment
  program->instructions = freeList(program->instructions);

  // Iterate through all the instructions in all the basic blocks (in order)
  // and re-add them to the program.
  t_listNode *current_bb_element = graph->blocks;
  while (current_bb_element != NULL) {
    t_basicBlock *bblock = (t_basicBlock *)current_bb_element->data;
    t_listNode *current_nd_element = bblock->nodes;
    while (current_nd_element != NULL) {
      t_cfgNode *node = (t_cfgNode *)current_nd_element->data;

      program->instructions =
          addElement(program->instructions, node->instr, -1);

      current_nd_element = current_nd_element->next;
    }
    current_bb_element = current_bb_element->next;
  }
}

t_listNode *bbGetLiveOutVars(t_basicBlock *bblock)
{
  if (bblock == NULL)
    return NULL;
  if (bblock->nodes == NULL)
    return NULL;

  t_listNode *last = getLastElement(bblock->nodes);
  t_cfgNode *lastNode = (t_cfgNode *)last->data;
  assert(lastNode != NULL);

  return cloneList(lastNode->out);
}

t_listNode *bbGetLiveInVars(t_basicBlock *bblock)
{
  if (bblock == NULL)
    return NULL;
  if (bblock->nodes == NULL)
    return NULL;

  t_cfgNode *firstNode = (t_cfgNode *)bblock->nodes->data;
  assert(firstNode != NULL);

  return cloneList(firstNode->in);
}

t_listNode *addElementToSet(t_listNode *set, void *element,
    bool (*compareFunc)(void *a, void *b), int *modified)
{
  if (element == NULL)
    return set;

  // Add the element if it's not already in the `set' list.
  if (findElementWithCallback(set, element, compareFunc) == NULL) {
    set = addElement(set, element, -1);
    if (modified != NULL)
      (*modified) = 1;
  }

  return set;
}

t_listNode *addElementsToSet(t_listNode *set, t_listNode *elements,
    bool (*compareFunc)(void *a, void *b), int *modified)
{
  // Add all the elements to the set one by one
  t_listNode *current_element = elements;
  while (current_element != NULL) {
    void *current_data = current_element->data;
    set = addElementToSet(set, current_data, compareFunc, modified);
    current_element = current_element->next;
  }

  // return the new list
  return set;
}

t_listNode *computeLiveInSetEquation(t_cfgReg *defs[CFG_MAX_DEFS],
    t_cfgReg *uses[CFG_MAX_USES], t_listNode *liveOut)
{
  // Initialize live in set as equal to live out set
  t_listNode *liveIn = cloneList(liveOut);

  // Add all items from set of uses
  for (int i = 0; i < CFG_MAX_USES; i++) {
    if (uses[i] == NULL)
      continue;
    if (TARGET_REG_ZERO_IS_CONST && uses[i]->ID == REG_0)
      continue;
    liveIn = addElementToSet(liveIn, uses[i], NULL, NULL);
  }

  // Remove items from set of definitions as long as they are not present in
  // the set of uses
  for (int def_i = 0; def_i < CFG_MAX_DEFS; def_i++) {
    int found = 0;

    if (defs[def_i] == NULL)
      continue;
    if (TARGET_REG_ZERO_IS_CONST && defs[def_i]->ID == REG_0)
      continue;

    for (int use_i = 0; use_i < CFG_MAX_USES && !found; use_i++) {
      if (uses[use_i] && uses[use_i]->ID == defs[def_i]->ID)
        found = 1;
    }

    if (!found)
      liveIn = removeElementWithData(liveIn, defs[def_i]);
  }

  return liveIn;
}

/* Re-computes the variables live out of a block by applying the standard
 * flow equation:
 *   out(block) = union in(block') for all successor block' */
t_listNode *cfgComputeLiveOutOfBlock(t_cfg *graph, t_basicBlock *block)
{
  assert(block != NULL);
  assert(graph != NULL);

  // Iterate through all the successor blocks
  t_listNode *result = NULL;
  t_listNode *current_elem = block->succ;
  while (current_elem != NULL) {
    t_basicBlock *current_succ = (t_basicBlock *)current_elem->data;
    assert(current_succ != NULL);

    if (current_succ != graph->endingBlock) {
      // Update our block's 'out' set by adding all variables 'in' to the
      // current successor
      t_listNode *liveINVars = bbGetLiveInVars(current_succ);
      result = addElementsToSet(result, liveINVars, NULL, NULL);
      freeList(liveINVars);
    }

    current_elem = current_elem->next;
  }

  return result;
}

static bool cfgUpdateLivenessOfNodesInBlock(t_cfg *graph, t_basicBlock *bblock)
{
  assert(bblock != NULL && bblock->nodes != NULL);

  // Keep track of whether we modified something or not
  int modified = 0;

  // Start with the last node in the basic block, we will proceed upwards
  // from there.
  t_listNode *curLI = getLastElement(bblock->nodes);
  // The live in set of the successors of the last node in the block is the
  // live out set of the block itself.
  t_listNode *successorsLiveIn = cfgComputeLiveOutOfBlock(graph, bblock);

  while (curLI != NULL) {
    // Get the current CFG node
    t_cfgNode *curNode = (t_cfgNode *)curLI->data;
    assert(curNode != NULL);

    // Live out of a block is equal to the union of the live in sets of the
    // successors
    curNode->out =
        addElementsToSet(curNode->out, successorsLiveIn, NULL, &modified);
    freeList(successorsLiveIn);

    // Compute the live in set of the block using the set of definition,
    // uses and live out variables of the block
    t_listNode *liveIn =
        computeLiveInSetEquation(curNode->defs, curNode->uses, curNode->out);
    curNode->in = addElementsToSet(curNode->in, liveIn, NULL, &modified);

    // Since there are no branches in a basic block, the successors of
    // the predecessor is just the current block.
    successorsLiveIn = liveIn;

    // Continue backwards to the previous node
    curLI = curLI->prev;
  }

  freeList(successorsLiveIn);

  // Return a non-zero value if anything was modified
  return !!modified;
}

static bool cfgPerformLivenessIteration(t_cfg *graph)
{
  assert(graph != NULL);
  assert(graph->endingBlock != NULL);

  bool modified = false;
  t_listNode *current_element = getLastElement(graph->blocks);
  while (current_element != NULL) {
    t_basicBlock *current_bblock = (t_basicBlock *)current_element->data;
    assert(current_bblock != NULL);

    // update the liveness informations for the current bblock
    if (cfgUpdateLivenessOfNodesInBlock(graph, current_bblock))
      modified = true;

    current_element = current_element->prev;
  }
  return modified;
}

void cfgComputeLiveness(t_cfg *graph)
{
  bool modified;
  do {
    modified = cfgPerformLivenessIteration(graph);
  } while (modified);
}

void dumpCFlowGraphVariable(t_cfgReg *var, FILE *fout)
{
  if (var->ID == REG_INVALID) {
    fprintf(fout, "<!UNDEF!>");
  } else {
    char *reg = registerIDToString(var->ID, false);
    fprintf(fout, "%s", reg);
    free(reg);
  }
}

static void dumpArrayOfVariables(t_cfgReg **array, int size, FILE *fout)
{
  int foundVariables = 0;
  int i;

  for (i = 0; i < size; i++) {
    if (!(array[i]))
      continue;

    if (foundVariables > 0)
      fprintf(fout, ", ");

    dumpCFlowGraphVariable(array[i], fout);
    foundVariables++;
  }

  fflush(fout);
}

static void dumpListOfVariables(t_listNode *variables, FILE *fout)
{
  t_listNode *current_element;
  t_cfgReg *current_variable;

  if (variables == NULL)
    return;
  if (fout == NULL)
    return;

  current_element = variables;
  while (current_element != NULL) {
    current_variable = (t_cfgReg *)current_element->data;
    dumpCFlowGraphVariable(current_variable, fout);
    if (current_element->next != NULL)
      fprintf(fout, ", ");

    current_element = current_element->next;
  }
  fflush(fout);
}

static int cfgComputeBBIndex(t_cfg *cfg, t_basicBlock *bb)
{
  if (bb == cfg->endingBlock)
    return getLength(cfg->blocks);

  int res = 1;
  t_listNode *cur = cfg->blocks;
  while (cur) {
    t_basicBlock *bb2 = (t_basicBlock *)cur->data;
    if (bb2 == bb)
      return res;
    res++;
    cur = cur->next;
  }

  fatalError("malformed CFG, found basic block not in list");
}

static void dumpBBList(t_cfg *cfg, t_listNode *list, FILE *fout)
{
  t_listNode *cur = list;
  while (cur) {
    t_basicBlock *bb = (t_basicBlock *)cur->data;
    fprintf(fout, "%d", cfgComputeBBIndex(cfg, bb));
    cur = cur->next;
    if (cur)
      fprintf(fout, ", ");
  }
}

void cfgDumpBB(t_cfg *cfg, t_basicBlock *block, FILE *fout, bool verbose)
{
  if (block == NULL)
    return;
  if (fout == NULL)
    return;

  fprintf(fout, "Predecessors: {");
  dumpBBList(cfg, block->pred, fout);
  fprintf(fout, "}\n");
  fprintf(fout, "Successors:   {");
  dumpBBList(cfg, block->succ, fout);
  fprintf(fout, "}\n");

  int count = 1;
  t_listNode *elem = block->nodes;
  while (elem != NULL) {
    t_cfgNode *current_node = (t_cfgNode *)elem->data;

    fprintf(fout, "%4d. ", count);
    if (current_node->instr == NULL)
      fprintf(fout, "(null)");
    else
      printInstruction(current_node->instr, fout, false);
    fprintf(fout, "\n");

    if (verbose) {
      fprintf(fout, "      DEFS     = {");
      dumpArrayOfVariables(current_node->defs, CFG_MAX_DEFS, fout);
      fprintf(fout, "}\n");
      fprintf(fout, "      USES     = {");
      dumpArrayOfVariables(current_node->uses, CFG_MAX_USES, fout);
      fprintf(fout, "}\n");

      fprintf(fout, "      LIVE IN  = {");
      dumpListOfVariables(current_node->in, fout);
      fprintf(fout, "}\n");
      fprintf(fout, "      LIVE OUT = {");
      dumpListOfVariables(current_node->out, fout);
      fprintf(fout, "}\n");
    }

    count++;
    elem = elem->next;
  }
  fflush(fout);
}

void cfgDump(t_cfg *graph, FILE *fout, bool verbose)
{
  if (graph == NULL)
    return;
  if (fout == NULL)
    return;

  fprintf(fout, "*************************\n");
  fprintf(fout, "    CONTROL FLOW GRAPH   \n");
  fprintf(fout, "*************************\n\n");

  fprintf(fout, "%s",
      "NOTE: Temporary registers are considered as variables of the\n"
      "intermediate language.\n");
  if (TARGET_REG_ZERO_IS_CONST) {
    fprintf(fout, "%s",
        "  Variable \'x0\' (which refers to the physical register \'zero\') "
        "is\n"
        "always considered LIVE-IN for each node of a basic block.\n"
        "Thus, in the following control flow graph, \'x0\' will never appear\n"
        "as LIVE-IN or LIVE-OUT variable for a statement.\n"
        "  If you want to consider \'x0\' as a normal variable, you have to\n"
        "un-define the macro TARGET_REG_ZERO_IS_CONST in \"cflow_graph.h\"."
        "\n\n");
  }

  fprintf(fout, "--------------\n");
  fprintf(fout, "  STATISTICS\n");
  fprintf(fout, "--------------\n\n");

  fprintf(fout, "Number of basic blocks:   %d\n", getLength(graph->blocks));
  fprintf(fout, "Number of used variables: %d\n\n",
      getLength(graph->cflow_variables));

  fprintf(fout, "----------------\n");
  fprintf(fout, "  BASIC BLOCKS\n");
  fprintf(fout, "----------------\n\n");

  int counter = 1;
  t_listNode *current_element = graph->blocks;
  while (current_element != NULL) {
    t_basicBlock *current_bblock = (t_basicBlock *)current_element->data;
    fprintf(fout, "[BLOCK %d]\n", counter);
    cfgDumpBB(graph, current_bblock, fout, verbose);
    fprintf(fout, "\n");

    counter++;
    current_element = current_element->next;
  }
  fflush(fout);
}
