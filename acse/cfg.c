/// @file cfg.c

#include <assert.h>
#include "cfg.h"
#include "target_info.h"
#include "target_asm_print.h"
#include "errors.h"


static bool compareCFGRegAndRegID(void *a, void *b)
{
  t_cfgReg *cfgReg = (t_cfgReg *)a;
  t_regID id = *((t_regID *)b);

  if (cfgReg == NULL)
    return false;
  return cfgReg->tempRegID == id;
}

/* Alloc a new control flow graph register object. If a register object
 * referencing the same identifier already exists, returns the pre-existing
 * object. */
static t_cfgReg *createCFGRegister(
    t_cfg *graph, t_regID identifier, t_listNode *mcRegs)
{
  // Test if a register with the same identifier is already present.
  t_listNode *elementFound = listFindWithCallback(
      graph->registers, &identifier, compareCFGRegAndRegID);

  t_cfgReg *result;
  if (elementFound) {
    // It's there: just use it
    result = (t_cfgReg *)elementFound->data;
  } else {
    // If it's not there it needs to be created
    result = malloc(sizeof(t_cfgReg));
    if (result == NULL)
      fatalError("out of memory");
    result->tempRegID = identifier;
    result->mcRegWhitelist = NULL;
    // Insert it in the list of registers
    graph->registers = listInsert(graph->registers, result, -1);
  }

  // copy the machine register allocation constraint, or compute the
  // intersection between the register allocation constraint sets
  if (mcRegs) {
    if (result->mcRegWhitelist == NULL) {
      result->mcRegWhitelist = listClone(mcRegs);
    } else {
      t_listNode *thisReg = result->mcRegWhitelist;
      while (thisReg) {
        t_listNode *nextReg = thisReg->next;
        if (!listFind(mcRegs, thisReg->data)) {
          result->mcRegWhitelist =
              listRemoveNode(result->mcRegWhitelist, thisReg);
        }
        thisReg = nextReg;
      }
      if (result->mcRegWhitelist == NULL)
        fatalError("bug: unsatisfiable register constraints on t%d", identifier);
    }
  }

  return result;
}

static void cfgComputeDefUses(t_cfg *graph, t_cfgNode *node)
{
  t_instruction *instr = node->instr;

  // Create or lookup CFG register objects for all arguments
  t_cfgReg *regDest = NULL;
  t_cfgReg *regSource1 = NULL;
  t_cfgReg *regSource2 = NULL;
  if (instr->rDest != NULL) {
    regDest = createCFGRegister(
        graph, (instr->rDest)->ID, instr->rDest->mcRegWhitelist);
  }
  if (instr->rSrc1 != NULL) {
    regSource1 = createCFGRegister(
        graph, (instr->rSrc1)->ID, instr->rSrc1->mcRegWhitelist);
  }
  if (instr->rSrc2 != NULL) {
    regSource2 = createCFGRegister(
        graph, (instr->rSrc2)->ID, instr->rSrc2->mcRegWhitelist);
  }

  // Fill the def/use sets for this node
  int defIdx = 0;
  if (regDest)
    node->defs[defIdx++] = regDest;
  int useIdx = 0;
  if (regSource1)
    node->uses[useIdx++] = regSource1;
  if (regSource2)
    node->uses[useIdx++] = regSource2;
}

t_cfgNode *createCFGNode(t_cfg *graph, t_instruction *instr)
{
  t_cfgNode *result = malloc(sizeof(t_cfgNode));
  if (result == NULL)
    fatalError("out of memory");

  for (int i = 0; i < CFG_MAX_DEFS; i++)
    result->defs[i] = NULL;
  for (int i = 0; i < CFG_MAX_USES; i++)
    result->uses[i] = NULL;
  result->instr = instr;
  result->in = NULL;
  result->out = NULL;
  cfgComputeDefUses(graph, result);

  return result;
}

void deleteCFGNode(t_cfgNode *node)
{
  if (node == NULL)
    return;

  if (node->in != NULL)
    deleteList(node->in);
  if (node->out != NULL)
    deleteList(node->out);
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
    deleteList(block->pred);
  if (block->succ != NULL)
    deleteList(block->succ);

  t_listNode *curNode = block->nodes;
  while (curNode != NULL) {
    t_cfgNode *curCFGNode = (t_cfgNode *)curNode->data;
    deleteCFGNode(curCFGNode);
    curNode = curNode->next;
  }

  deleteList(block->nodes);
  free(block);
}

void bbAddPred(t_basicBlock *block, t_basicBlock *pred)
{
  // do not insert if the block is already inserted in the list of predecessors
  if (listFind(block->pred, pred) == NULL) {
    block->pred = listInsert(block->pred, pred, -1);
    pred->succ = listInsert(pred->succ, block, -1);
  }
}

void bbAddSucc(t_basicBlock *block, t_basicBlock *succ)
{
  // do not insert if the node is already inserted in the list of successors
  if (listFind(block->succ, succ) == NULL) {
    block->succ = listInsert(block->succ, succ, -1);
    succ->pred = listInsert(succ->pred, block, -1);
  }
}

t_cfgError bbInsertNode(t_basicBlock *block, t_cfgNode *newNode)
{
  if (listFind(block->nodes, newNode) != NULL)
    return CFG_ERROR_NODE_ALREADY_INSERTED;

  block->nodes = listInsert(block->nodes, newNode, -1);
  return CFG_NO_ERROR;
}

t_cfgError bbInsertNodeBefore(
    t_basicBlock *block, t_cfgNode *ip, t_cfgNode *newNode)
{
  t_listNode *listIP = listFind(block->nodes, ip);
  if (listIP == NULL)
    return CFG_ERROR_INVALID_NODE;

  if (listFind(block->nodes, newNode) != NULL)
    return CFG_ERROR_NODE_ALREADY_INSERTED;

  block->nodes = listInsertBefore(block->nodes, listIP, newNode);
  return CFG_NO_ERROR;
}

/* insert a new node without updating the dataflow informations */
t_cfgError bbInsertNodeAfter(
    t_basicBlock *block, t_cfgNode *nextCFGNode, t_cfgNode *newNode)
{
  t_listNode *listIP = listFind(block->nodes, nextCFGNode);
  if (listIP == NULL)
    return CFG_ERROR_INVALID_NODE;

  if (listFind(block->nodes, newNode) != NULL)
    return CFG_ERROR_NODE_ALREADY_INSERTED;

  block->nodes = listInsertAfter(block->nodes, listIP, newNode);
  return CFG_NO_ERROR;
}

/* allocate memory for a control flow graph */
static t_cfg *newCFG(void)
{
  t_cfg *result = malloc(sizeof(t_cfg));
  if (result == NULL)
    fatalError("out of memory");
  result->blocks = NULL;
  result->registers = NULL;
  // Create the dummy ending block.
  result->endingBlock = newBasicBlock();
  return result;
}

/* finalize the memory associated with the given control flow graph */
void deleteCFG(t_cfg *graph)
{
  if (graph == NULL)
    return;

  t_listNode *curNode = graph->blocks;
  while (curNode != NULL) {
    t_basicBlock *curBlock = (t_basicBlock *)curNode->data;
    deleteBasicBlock(curBlock);
    curNode = curNode->next;
  }

  if (graph->blocks != NULL)
    deleteList(graph->blocks);
  if (graph->endingBlock != NULL)
    deleteBasicBlock(graph->endingBlock);
  if (graph->registers != NULL) {
    t_listNode *curNode = graph->registers;
    while (curNode != NULL) {
      t_cfgReg *curReg = (t_cfgReg *)curNode->data;

      if (curReg != NULL) {
        deleteList(curReg->mcRegWhitelist);
        free(curReg);
      }

      curNode = curNode->next;
    }

    deleteList(graph->registers);
  }

  free(graph);
}

/* look up for a label inside the graph */
static t_basicBlock *cfgSearchLabel(t_cfg *graph, t_label *label)
{
  if (label == NULL)
    return NULL;

  t_basicBlock *bblock = NULL;
  t_listNode *curNode = graph->blocks;
  while (curNode != NULL) {
    bblock = (t_basicBlock *)curNode->data;

    // Check the first node of the basic block. If its instruction has a label,
    // check if it's the label we are searching fore.
    t_cfgNode *curCFGNode = (t_cfgNode *)bblock->nodes->data;
    if ((curCFGNode->instr)->label != NULL) {
      if ((curCFGNode->instr)->label->labelID == label->labelID)
        // Found!
        break;
    }

    curNode = curNode->next;
  }

  return bblock;
}

/* test if 'instr' is a labelled instruction */
static bool instrIsStartingNode(t_instruction *instr)
{
  return instr->label != NULL;
}

/* test if 'instr' must appear at the end of a basic block */
static bool instrIsEndingNode(t_instruction *instr)
{
  return isHaltOrRetInstruction(instr) || isJumpInstruction(instr);
}

t_cfgError cfgInsertBlock(t_cfg *graph, t_basicBlock *block)
{
  if (listFind(graph->blocks, block) != NULL)
    return CFG_ERROR_BBLOCK_ALREADY_INSERTED;

  graph->blocks = listInsert(graph->blocks, block, -1);
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
  t_listNode *curNode = graph->blocks;
  while (curNode != NULL) {
    t_basicBlock *curBlock = (t_basicBlock *)curNode->data;

    // Get the last instruction in the basic block
    t_listNode *lastNode = listGetLastNode(curBlock->nodes);
    t_cfgNode *lastCFGNode = (t_cfgNode *)lastNode->data;
    t_instruction *lastInstr = lastCFGNode->instr;

    // If the instruction is return-like or exit-like, by definition the next
    // block is the ending block because it stops the program/subroutine.
    if (isHaltOrRetInstruction(lastInstr)) {
      bbAddSucc(curBlock, graph->endingBlock);
      bbAddPred(graph->endingBlock, curBlock);
      continue;
    }

    // All branch/jump instructions may transfer control to the code
    // indicated by their label argument, so add edges appropriately.
    if (isJumpInstruction(lastInstr)) {
      if (lastInstr->addressParam == NULL)
        fatalError("bug: malformed jump instruction with no label in CFG");
      t_basicBlock *jumpBlock =
          cfgSearchLabel(graph, lastInstr->addressParam);
      if (jumpBlock == NULL)
        fatalError("bug: malformed jump instruction with invalid label in CFG");
      bbAddPred(jumpBlock, curBlock);
      bbAddSucc(curBlock, jumpBlock);
    }

    // Additionally, conditional jumps may also not be taken, and in that
    // case the execution continues to the next instruction.
    //   As the order of the blocks in the block list reflects the order of
    // the instructions in the program, we can rely on this property to fetch
    // the correct block for this fallthrough case.
    if (!isUnconditionalJump(lastInstr)) {
      t_listNode *nextNode = curNode->next;
      if (nextNode != NULL) {
        // The current basic block has a successor in the list, all is fine
        t_basicBlock *nextBlock = nextNode->data;
        bbAddSucc(curBlock, nextBlock);
        bbAddPred(nextBlock, curBlock);
      } else {
        // If this is the last basic block in the list, the next block is
        // the ending block (which exists outside the list)
        bbAddSucc(curBlock, graph->endingBlock);
        bbAddPred(graph->endingBlock, curBlock);
      }
    }

    curNode = curNode->next;
  }
}

t_cfg *programToCFG(t_program *program)
{
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
  t_listNode *curNode = instructions;
  while (curNode != NULL) {
    t_instruction *curInstr = (t_instruction *)curNode->data;

    // create the next node to insert in the block
    t_cfgNode *curCFGNode = createCFGNode(result, curInstr);

    // If the instruction node needs to be at the beginning of a basic block
    // (= is labeled) or if `bblock' is NULL (because the last instruction was
    // a terminator) then create a new basic block.
    if (instrIsStartingNode(curInstr) || bblock == NULL) {
      bblock = newBasicBlock();
      // Add the block to the graph.
      t_cfgError error = cfgInsertBlock(result, bblock);
      assert(error == CFG_NO_ERROR && "cfgInsertNode failed, corrupt CFG?");
    }

    // Add the instruction to the current basic block
    t_cfgError error = bbInsertNode(bblock, curCFGNode);
    assert(error == CFG_NO_ERROR && "bbInsertNode failed, corrupt CFG?");

    // If the instruction is a basic block terminator, set `bblock' to NULL
    // to stop inserting nodes into it.
    if (instrIsEndingNode(curInstr))
      bblock = NULL;

    curNode = curNode->next;
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

  t_listNode *curBlockNode = graph->blocks;
  while (curBlockNode != NULL) {
    t_basicBlock *curBlock = (t_basicBlock *)curBlockNode->data;

    t_listNode *curInnerNode = curBlock->nodes;
    while (curInnerNode != NULL) {
      t_cfgNode *curCFGNode = (t_cfgNode *)curInnerNode->data;

      exitcode = callback(curBlock, curCFGNode, counter, context);
      if (exitcode != 0)
        return exitcode;

      counter++;
      curInnerNode = curInnerNode->next;
    }

    curBlockNode = curBlockNode->next;
  }
  return exitcode;
}

void cfgToProgram(t_program *program, t_cfg *graph)
{
  // Erase the old code segment
  program->instructions = deleteList(program->instructions);

  // Iterate through all the instructions in all the basic blocks (in order)
  // and re-add them to the program.
  t_listNode *curBlockNode = graph->blocks;
  while (curBlockNode != NULL) {
    t_basicBlock *bblock = (t_basicBlock *)curBlockNode->data;
    t_listNode *curInnerNode = bblock->nodes;
    while (curInnerNode != NULL) {
      t_cfgNode *node = (t_cfgNode *)curInnerNode->data;

      program->instructions =
          listInsert(program->instructions, node->instr, -1);

      curInnerNode = curInnerNode->next;
    }
    curBlockNode = curBlockNode->next;
  }
}

t_listNode *bbGetLiveOut(t_basicBlock *bblock)
{
  if (bblock == NULL)
    return NULL;
  if (bblock->nodes == NULL)
    return NULL;

  t_listNode *last = listGetLastNode(bblock->nodes);
  t_cfgNode *lastNode = (t_cfgNode *)last->data;
  return listClone(lastNode->out);
}

t_listNode *bbGetLiveIn(t_basicBlock *bblock)
{
  if (bblock == NULL)
    return NULL;
  if (bblock->nodes == NULL)
    return NULL;

  t_cfgNode *firstNode = (t_cfgNode *)bblock->nodes->data;
  return listClone(firstNode->in);
}

static t_listNode *addElementToSet(t_listNode *set, void *element,
    bool (*compareFunc)(void *a, void *b), bool *modified)
{
  if (element == NULL)
    return set;

  // Add the element if it's not already in the `set' list.
  if (listFindWithCallback(set, element, compareFunc) == NULL) {
    set = listInsert(set, element, -1);
    if (modified != NULL)
      (*modified) = true;
  }

  return set;
}

static t_listNode *addElementsToSet(t_listNode *set, t_listNode *elements,
    bool (*compareFunc)(void *a, void *b), bool *modified)
{
  // Add all the elements to the set one by one
  t_listNode *curNode = elements;
  while (curNode != NULL) {
    void *curData = curNode->data;
    set = addElementToSet(set, curData, compareFunc, modified);
    curNode = curNode->next;
  }

  // return the new list
  return set;
}

static t_listNode *computeLiveInSetEquation(t_cfgReg *defs[CFG_MAX_DEFS],
    t_cfgReg *uses[CFG_MAX_USES], t_listNode *liveOut)
{
  // Initialize live in set as equal to live out set
  t_listNode *liveIn = listClone(liveOut);

  // Add all items from set of uses
  for (int i = 0; i < CFG_MAX_USES; i++) {
    if (uses[i] == NULL)
      continue;
    if (TARGET_REG_ZERO_IS_CONST && uses[i]->tempRegID == REG_0)
      continue;
    liveIn = addElementToSet(liveIn, uses[i], NULL, NULL);
  }

  // Remove items from set of definitions as long as they are not present in
  // the set of uses
  for (int defIdx = 0; defIdx < CFG_MAX_DEFS; defIdx++) {
    int found = 0;

    if (defs[defIdx] == NULL)
      continue;
    if (TARGET_REG_ZERO_IS_CONST && defs[defIdx]->tempRegID == REG_0)
      continue;

    for (int useIdx = 0; useIdx < CFG_MAX_USES && !found; useIdx++) {
      if (uses[useIdx] && uses[useIdx]->tempRegID == defs[defIdx]->tempRegID)
        found = 1;
    }

    if (!found)
      liveIn = listFindAndRemove(liveIn, defs[defIdx]);
  }

  return liveIn;
}

/* Re-computes the live registers out of a block by applying the standard
 * flow equation:
 *   out(block) = union in(block') for all successor block' */
static t_listNode *cfgComputeLiveOutOfBlock(t_cfg *graph, t_basicBlock *block)
{
  // Iterate through all the successor blocks
  t_listNode *result = NULL;
  t_listNode *curSuccNode = block->succ;
  while (curSuccNode != NULL) {
    t_basicBlock *curSuccessor = (t_basicBlock *)curSuccNode->data;

    if (curSuccessor != graph->endingBlock) {
      // Update our block's 'out' set by adding all registers 'in' to the
      // current successor
      t_listNode *liveINRegs = bbGetLiveIn(curSuccessor);
      result = addElementsToSet(result, liveINRegs, NULL, NULL);
      deleteList(liveINRegs);
    }

    curSuccNode = curSuccNode->next;
  }

  return result;
}

static bool cfgUpdateLivenessOfNodesInBlock(t_cfg *graph, t_basicBlock *bblock)
{
  // Keep track of whether we modified something or not
  bool modified = false;

  // Start with the last node in the basic block, we will proceed upwards
  // from there.
  t_listNode *curLI = listGetLastNode(bblock->nodes);
  // The live in set of the successors of the last node in the block is the
  // live out set of the block itself.
  t_listNode *successorsLiveIn = cfgComputeLiveOutOfBlock(graph, bblock);

  while (curLI != NULL) {
    // Get the current CFG node
    t_cfgNode *curNode = (t_cfgNode *)curLI->data;

    // Live out of a block is equal to the union of the live in sets of the
    // successors
    curNode->out =
        addElementsToSet(curNode->out, successorsLiveIn, NULL, &modified);
    deleteList(successorsLiveIn);

    // Compute the live in set of the block using the set of definition,
    // uses and live out registers of the block
    t_listNode *liveIn =
        computeLiveInSetEquation(curNode->defs, curNode->uses, curNode->out);
    curNode->in = addElementsToSet(curNode->in, liveIn, NULL, &modified);

    // Since there are no branches in a basic block, the successors of
    // the predecessor is just the current block.
    successorsLiveIn = liveIn;

    // Continue backwards to the previous node
    curLI = curLI->prev;
  }

  deleteList(successorsLiveIn);

  // Return a non-zero value if anything was modified
  return modified;
}

static bool cfgPerformLivenessIteration(t_cfg *graph)
{
  bool modified = false;
  t_listNode *curNode = listGetLastNode(graph->blocks);
  while (curNode != NULL) {
    t_basicBlock *curBlock = (t_basicBlock *)curNode->data;

    // update the liveness informations for the current bblock
    if (cfgUpdateLivenessOfNodesInBlock(graph, curBlock))
      modified = true;

    curNode = curNode->prev;
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

static void dumpCFGRegister(t_cfgReg *reg, FILE *fout)
{
  if (reg->tempRegID == REG_INVALID) {
    fprintf(fout, "<!UNDEF!>");
  } else {
    char *regName = registerIDToString(reg->tempRegID, false);
    fprintf(fout, "%s", regName);
    free(regName);
  }
}

static void dumpArrayOfCFGRegisters(t_cfgReg **array, int size, FILE *fout)
{
  int foundRegs = 0;

  for (int i = 0; i < size; i++) {
    if (!(array[i]))
      continue;

    if (foundRegs > 0)
      fprintf(fout, ", ");

    dumpCFGRegister(array[i], fout);
    foundRegs++;
  }

  fflush(fout);
}

static void dumpListOfCFGRegisters(t_listNode *regs, FILE *fout)
{
  t_listNode *currentListNode;
  t_cfgReg *curReg;

  if (regs == NULL)
    return;
  if (fout == NULL)
    return;

  currentListNode = regs;
  while (currentListNode != NULL) {
    curReg = (t_cfgReg *)currentListNode->data;
    dumpCFGRegister(curReg, fout);
    if (currentListNode->next != NULL)
      fprintf(fout, ", ");

    currentListNode = currentListNode->next;
  }
  fflush(fout);
}

static int cfgComputeBBIndex(t_cfg *cfg, t_basicBlock *bb)
{
  if (bb == cfg->endingBlock)
    return listLength(cfg->blocks);

  int res = 1;
  t_listNode *cur = cfg->blocks;
  while (cur) {
    t_basicBlock *bb2 = (t_basicBlock *)cur->data;
    if (bb2 == bb)
      return res;
    res++;
    cur = cur->next;
  }

  fatalError("bug: malformed CFG, found basic block not in list");
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

static void cfgDumpBB(t_cfg *cfg, t_basicBlock *block, FILE *fout, bool verbose)
{
  if (block == NULL)
    return;
  if (fout == NULL)
    return;

  fprintf(fout, "  Predecessor blocks: {");
  dumpBBList(cfg, block->pred, fout);
  fprintf(fout, "}\n");
  fprintf(fout, "  Successor blocks:   {");
  dumpBBList(cfg, block->succ, fout);
  fprintf(fout, "}\n");

  int count = 1;
  t_listNode *elem = block->nodes;
  while (elem != NULL) {
    t_cfgNode *curCFGNode = (t_cfgNode *)elem->data;

    fprintf(fout, "  Node %4d: ", count);
    if (curCFGNode->instr == NULL)
      fprintf(fout, "(null)");
    else
      printInstruction(curCFGNode->instr, fout, false);
    fprintf(fout, "\n");

    if (verbose) {
      fprintf(fout, "    def = {");
      dumpArrayOfCFGRegisters(curCFGNode->defs, CFG_MAX_DEFS, fout);
      fprintf(fout, "}\n");
      fprintf(fout, "    use = {");
      dumpArrayOfCFGRegisters(curCFGNode->uses, CFG_MAX_USES, fout);
      fprintf(fout, "}\n");

      fprintf(fout, "    in  = {");
      dumpListOfCFGRegisters(curCFGNode->in, fout);
      fprintf(fout, "}\n");
      fprintf(fout, "    out = {");
      dumpListOfCFGRegisters(curCFGNode->out, fout);
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

  fprintf(fout, "# Control Flow Graph dump\n\n");

  if (TARGET_REG_ZERO_IS_CONST) {
    fprintf(fout, "%s",
        "Note: Temporary register \'temp0\' refers to the physical register "
        "\'zero\', whose\nvalue is immutable. As a result, it does not appear "
        "in the liveness sets.\n\n");
  }

  fprintf(fout, "Number of basic blocks:   %d\n", listLength(graph->blocks));
  fprintf(
      fout, "Number of used registers: %d\n\n", listLength(graph->registers));

  fprintf(fout, "## Basic Blocks\n\n");

  int counter = 1;
  t_listNode *curNode = graph->blocks;
  while (curNode != NULL) {
    t_basicBlock *curBlock = (t_basicBlock *)curNode->data;
    fprintf(fout, "Block %d:\n", counter);
    cfgDumpBB(graph, curBlock, fout, verbose);
    fprintf(fout, "\n");

    counter++;
    curNode = curNode->next;
  }
  fflush(fout);
}
