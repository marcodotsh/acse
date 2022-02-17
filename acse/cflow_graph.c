/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * cflow_graph.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#include <assert.h>
#include "labels.h"
#include "cflow_graph.h"
#include "list.h"
#include "utils.h"
#include "utils.h"
#include "target_info.h"
#include "target_asm_print.h"


int compareCFGVariables(void *a, void *b)
{
   t_cflow_var *varA = (t_cflow_var *)a;
   t_cflow_var *varB = (t_cflow_var *)b;

   if (a == NULL && b == NULL)
      return 1;
   if (a == NULL || b == NULL)
      return 0;

   return varA->ID == varB->ID;
}

/* Alloc a new control flow graph variable object. If a variable object
 * referencing the same identifier already exists, returns the pre-existing
 * object. */
t_cflow_var *allocVariable(
      t_cflow_Graph *graph, int identifier, t_list *mcRegs, int *error)
{
   t_cflow_var *result;
   t_list *elementFound;

   assert(graph != NULL);

   /* alloc memory for a variable information */
   result = malloc(sizeof(t_cflow_var));
   if (result == NULL) {
      if (error)
         *error = AXE_OUT_OF_MEMORY;
      return NULL;
   }

   /* update the value of result */
   result->ID = identifier;
   result->mcRegWhitelist = NULL;

   /* test if a variable with the same identifier was already present */
   elementFound = findElementWithCallback(
         graph->cflow_variables, result, compareCFGVariables);

   if (elementFound == NULL) {
      /* update the set of variables */
      graph->cflow_variables = addElement(graph->cflow_variables, result, -1);
   } else {
      free(result);
      result = (t_cflow_var *)elementFound->data;
      assert(result != NULL);
      assert(result->ID == identifier);
   }

   /* copy the machine register allocation constraint, or compute the
    * intersection between the register allocation constraint sets */
   if (mcRegs) {
      if (result->mcRegWhitelist == NULL) {
         result->mcRegWhitelist = cloneList(mcRegs);
      } else {
         t_list *thisReg = result->mcRegWhitelist;
         while (thisReg) {
            t_list *nextReg = thisReg->next;
            if (!findElement(mcRegs, thisReg->data)) {
               result->mcRegWhitelist =
                     removeElementLink(result->mcRegWhitelist, thisReg);
            }
            thisReg = nextReg;
         }
         assert(result->mcRegWhitelist);
      }
   }

   /* return a new var identifier */
   return result;
}

/* set the def-use values for the current node */
int setDefUses(t_cflow_Graph *graph, t_cflow_Node *node)
{
   t_axe_instruction *instr;
   t_cflow_var *varDest, *varSource1, *varSource2, *varPSW;
   int def_i, use_i, error;

   /* preconditions */
   assert(graph != NULL);
   assert(node != NULL);
   assert(node->instr != NULL);

   /* update the value of `instr' */
   instr = node->instr;

   /* initialize the values of varDest, varSource1 and varSource2 */
   varDest = NULL;
   varSource1 = NULL;
   varSource2 = NULL;
   varPSW = allocVariable(graph, VAR_PSW, NULL, &error);
   if (varPSW == NULL)
      return error;

   /* update the values of the variables */
   if (instr->reg_dest != NULL) {
      varDest = allocVariable(graph, (instr->reg_dest)->ID,
            instr->reg_dest->mcRegWhitelist, &error);
      if (varDest == NULL)
         return error;
   }
   if (instr->reg_src1 != NULL) {
      varSource1 = allocVariable(graph, (instr->reg_src1)->ID,
            instr->reg_src1->mcRegWhitelist, &error);
      if (varSource1 == NULL)
         return error;
   }
   if (instr->reg_src2 != NULL) {
      varSource2 = allocVariable(graph, (instr->reg_src2)->ID,
            instr->reg_src2->mcRegWhitelist, &error);
      if (varSource2 == NULL)
         return error;
   }

   def_i = 0;
   use_i = 0;

   if (varDest)
      node->defs[def_i++] = varDest;
   if (instructionDefinesPSW(instr))
      node->defs[def_i++] = varPSW;

   if (varSource1)
      node->uses[use_i++] = varSource1;
   if (varSource1)
      node->uses[use_i++] = varSource2;
   if (instructionUsesPSW(instr))
      node->uses[use_i++] = varPSW;

   return AXE_OK;
}

t_cflow_Node *allocNode(
      t_cflow_Graph *graph, t_axe_instruction *instr, int *error)
{
   t_cflow_Node *result;
   int i, error2;

   /* test the preconditions */
   assert(graph != NULL);
   assert(instr != NULL);

   /* create a new instance of type `t_cflow_node' */
   result = malloc(sizeof(t_cflow_Node));
   if (result == NULL) {
      if (error)
         *error = AXE_OUT_OF_MEMORY;
      return NULL;
   }

   /* initialize result */
   for (i = 0; i < CFLOW_MAX_DEFS; i++)
      result->defs[i] = NULL;
   for (i = 0; i < CFLOW_MAX_USES; i++)
      result->uses[i] = NULL;
   result->instr = instr;

   /* set the def-uses for the current node */
   error2 = setDefUses(graph, result);
   if (error2 != AXE_OK) {
      if (error)
         *error = error2;
      free(result);
      return NULL;
   }

   /* set the list of variables that are live in
    * and live out from the current node */
   result->in = NULL;
   result->out = NULL;

   /* return the node */
   return result;
}

void finalizeNode(t_cflow_Node *node)
{
   if (node == NULL)
      return;

   /* free the two lists `in' and `out' */
   if (node->in != NULL)
      freeList(node->in);
   if (node->out != NULL)
      freeList(node->out);

   /* free the current node */
   free(node);
}

t_basic_block *allocBasicBlock(int *error)
{
   t_basic_block *result;

   result = malloc(sizeof(t_basic_block));
   if (result == NULL) {
      if (error)
         *error = AXE_OUT_OF_MEMORY;
      return NULL;
   }

   /* initialize result */
   result->pred = NULL;
   result->succ = NULL;
   result->nodes = NULL;

   return result;
}

void finalizeBasicBlock(t_basic_block *block)
{
   t_list *current_element;
   t_cflow_Node *current_node;

   if (block == NULL)
      return;

   if (block->pred != NULL)
      freeList(block->pred);
   if (block->succ != NULL)
      freeList(block->succ);

   /* initialize current_element */
   current_element = block->nodes;

   while (current_element != NULL) {
      /* retrieve the current node */
      current_node = (t_cflow_Node *)current_element->data;

      /* free the memory associated with the current node */
      finalizeNode(current_node);

      /* retrieve the next node in the list */
      current_element = current_element->next;
   }

   freeList(block->nodes);

   /* free the memory associated with this basic block */
   free(block);
}

void setPred(t_basic_block *block, t_basic_block *pred)
{
   assert(block != NULL);
   assert(pred != NULL);

   /* test if the block is already inserted in the list of predecessors */
   if (findElement(block->pred, pred) == NULL) {
      block->pred = addElement(block->pred, pred, -1);
      pred->succ = addElement(pred->succ, block, -1);
   }
}

void setSucc(t_basic_block *block, t_basic_block *succ)
{
   /* preconditions */
   assert(block != NULL);
   assert(succ != NULL);

   /* test if the node is already inserted in the list of successors */
   if (findElement(block->succ, succ) == NULL) {
      block->succ = addElement(block->succ, succ, -1);
      succ->pred = addElement(succ->pred, block, -1);
   }
}

int insertNode(t_basic_block *block, t_cflow_Node *node)
{
   /* preconditions */
   assert(block != NULL);
   assert(node != NULL && node->instr != NULL);

   if (findElement(block->nodes, node) != NULL)
      return CFLOW_NODE_ALREADY_INSERTED;

   /* add the current node to the basic block */
   block->nodes = addElement(block->nodes, node, -1);
   return AXE_OK;
}

int insertNodeBefore(
      t_basic_block *block, t_cflow_Node *before_node, t_cflow_Node *new_node)
{
   int before_node_posn;
   t_list *before_node_elem;

   /* preconditions */
   assert(block != NULL);
   assert(new_node != NULL && new_node->instr != NULL && before_node != NULL);

   before_node_elem = findElement(block->nodes, before_node);
   if (before_node_elem == NULL)
      return CFLOW_INVALID_NODE;

   if (findElement(block->nodes, new_node) != NULL)
      return CFLOW_NODE_ALREADY_INSERTED;

   /* add the current node to the basic block */
   block->nodes = addBefore(block->nodes, before_node_elem, new_node);
   return AXE_OK;
}

/* insert a new node without updating the dataflow informations */
int insertNodeAfter(
      t_basic_block *block, t_cflow_Node *after_node, t_cflow_Node *new_node)
{
   int after_node_posn;
   t_list *after_node_elem;

   /* preconditions */
   assert(block != NULL);
   assert(new_node != NULL && new_node->instr != NULL && after_node != NULL);

   after_node_elem = findElement(block->nodes, after_node);
   if (after_node_elem == NULL)
      return CFLOW_INVALID_NODE;

   if (findElement(block->nodes, new_node) != NULL)
      return CFLOW_NODE_ALREADY_INSERTED;

   /* add the current node to the basic block */
   block->nodes = addAfter(block->nodes, after_node_elem, new_node);
   return AXE_OK;
}

/* allocate memory for a control flow graph */
t_cflow_Graph *allocGraph(int *error)
{
   t_cflow_Graph *result;

   result = malloc(sizeof(t_cflow_Graph));
   if (result == NULL) {
      if (error)
         *error = AXE_OUT_OF_MEMORY;
      return NULL;
   }

   /* initialize `result' */
   result->startingBlock = NULL;
   result->blocks = NULL;
   result->cflow_variables = NULL;
   result->endingBlock = allocBasicBlock(error);
   if (!result->endingBlock) {
      free(result);
      return NULL;
   }

   /* return the just created cflow graph */
   return result;
}

/* finalize the memory associated with the given control flow graph */
void finalizeGraph(t_cflow_Graph *graph)
{
   t_list *current_element;
   t_basic_block *current_block;

   if (graph == NULL)
      return;

   current_element = graph->blocks;
   while (current_element != NULL) {
      /* retrieve the current node */
      current_block = (t_basic_block *)current_element->data;
      assert(current_block != NULL);

      finalizeBasicBlock(current_block);

      current_element = current_element->next;
   }

   if (graph->blocks != NULL)
      freeList(graph->blocks);
   if (graph->endingBlock != NULL)
      finalizeBasicBlock(graph->endingBlock);
   if (graph->cflow_variables != NULL) {
      t_list *current_element;
      t_cflow_var *current_variable;

      current_element = graph->cflow_variables;
      while (current_element != NULL) {
         current_variable = (t_cflow_var *)current_element->data;

         if (current_variable != NULL) {
            freeList(current_variable->mcRegWhitelist);
            free(current_variable);
         }

         /* retrieve the next variable in the list */
         current_element = current_element->next;
      }

      freeList(graph->cflow_variables);
   }

   free(graph);
}

/* look up for a label inside the graph */
t_basic_block *searchLabel(t_cflow_Graph *graph, t_axe_label *label)
{
   t_list *current_element;
   t_basic_block *bblock;
   t_cflow_Node *current_node;

   /* preconditions: graph should not be a NULL pointer */
   assert(graph != NULL);

   /* test if we haven't to search for a label */
   if (label == NULL)
      return NULL;

   /* initialize `bblock' */
   bblock = NULL;

   current_element = graph->blocks;
   while (current_element != NULL) {
      bblock = (t_basic_block *)current_element->data;
      assert(bblock != NULL);
      assert(bblock->nodes != NULL);

      /* retrieve the first node of the basic block */
      current_node = (t_cflow_Node *)bblock->nodes->data;
      assert(current_node != NULL);

      /* if the first node holds a label information, we
       * have to verify if we have found the right label */
      if ((current_node->instr)->label != NULL) {
         if (compareLabels((current_node->instr)->label, label))
            /* we found the correct basic block */
            break;
      }

      /* retrieve the next element */
      current_element = current_element->next;
   }

   return bblock;
}

/* test if the current instruction `instr' is a labelled instruction */
int isStartingNode(t_axe_instruction *instr)
{
   /* preconditions */
   assert(instr != NULL);

   /* test if the instruction holds a label identifier */
   return instr->label != NULL;
}

/* test if the current instruction will end a basic block */
int isEndingNode(t_axe_instruction *instr)
{
   /* preconditions */
   assert(instr != NULL);
   return isHaltOrRetInstruction(instr) || isJumpInstruction(instr);
}

int insertBlock(t_cflow_Graph *graph, t_basic_block *block)
{
   /* preconditions */
   assert(graph != NULL);
   assert(block != NULL);

   if (findElement(graph->blocks, block) != NULL)
      return CFLOW_BBLOCK_ALREADY_INSERTED;

   /* add the current node to the basic block */
   graph->blocks = addElement(graph->blocks, block, -1);

   /* test if this is the first basic block for the program */
   if (graph->startingBlock == NULL)
      graph->startingBlock = block;
   return AXE_OK;
}

int updateFlowGraph(t_cflow_Graph *graph)
{
   t_list *current_element;
   t_basic_block *current_block;

   /* preconditions: graph should not be a NULL pointer */
   assert(graph != NULL);

   current_element = graph->blocks;
   while (current_element != NULL) {
      t_list *last_element;
      t_cflow_Node *last_node;
      t_axe_instruction *last_instruction;
      t_basic_block *jumpBlock;

      /* retrieve the current block */
      current_block = (t_basic_block *)current_element->data;
      assert(current_block != NULL);
      assert(current_block->nodes != NULL);

      /* get the last node of the basic block */
      last_element = getLastElement(current_block->nodes);
      assert(last_element != NULL);

      last_node = (t_cflow_Node *)last_element->data;
      assert(last_node != NULL);

      last_instruction = last_node->instr;
      assert(last_instruction != NULL);

      if (isHaltOrRetInstruction(last_instruction)) {
         setSucc(current_block, graph->endingBlock);
         setPred(graph->endingBlock, current_block);
      } else {
         if (isJumpInstruction(last_instruction)) {
            if ((last_instruction->address == NULL) ||
                  ((last_instruction->address)->labelID == NULL))
               return CFLOW_INVALID_LABEL_FOUND;

            jumpBlock =
                  searchLabel(graph, (last_instruction->address)->labelID);
            if (jumpBlock == NULL)
               return CFLOW_INVALID_LABEL_FOUND;

            /* add the jumpBlock to the list of successors of current_block */
            /* add also current_block to the list of predecessors of jumpBlock
             */
            setPred(jumpBlock, current_block);
            setSucc(current_block, jumpBlock);
         }

         if (!isUnconditionalJump(last_instruction)) {
            t_basic_block *nextBlock;
            t_list *next_element;

            next_element = current_element->next;
            if (next_element != NULL) {
               nextBlock = next_element->data;
               assert(nextBlock != NULL);

               setSucc(current_block, nextBlock);
               setPred(nextBlock, current_block);
            } else {
               setSucc(current_block, graph->endingBlock);
               setPred(graph->endingBlock, current_block);
            }
         }
      }

      /* update the value of `current_element' */
      current_element = current_element->next;
   }

   return AXE_OK;
}

t_cflow_Graph *createFlowGraph(t_program_infos *program, int *error)
{
   t_list *instructions;
   t_cflow_Graph *result;
   t_basic_block *bblock;
   t_list *current_element;
   t_cflow_Node *current_node;
   t_axe_instruction *current_instr;
   int startingNode;
   int endingNode;
   int error2;

   /* preconditions */
   assert(program != NULL);
   instructions = program->instructions;
   assert(instructions != NULL);

   /* alloc memory for a new control flow graph */
   result = allocGraph(error);
   if (result == NULL)
      return NULL;

   /* set the starting basic block */
   bblock = NULL;

   /* initialize the current element */
   current_element = instructions;
   while (current_element != NULL) {
      /* retrieve the current instruction */
      current_instr = (t_axe_instruction *)current_element->data;
      assert(current_instr != NULL);

      /* create a new node for the current basic block */
      current_node = allocNode(result, current_instr, error);
      if (current_node == NULL) {
         finalizeGraph(result);
         return NULL;
      }

      /* test if the current instruction will start or end a block */
      startingNode = isStartingNode(current_instr);
      endingNode = isEndingNode(current_instr);

      if (startingNode || bblock == NULL) {
         /* alloc a new basic block */
         bblock = allocBasicBlock(error);
         if (bblock == NULL) {
            finalizeGraph(result);
            finalizeNode(current_node);
            return NULL;
         }

         /* add the current instruction to the newly created
          * basic block */
         error2 = insertNode(bblock, current_node);
         if (error2 != AXE_OK) {
            if (error)
               *error = error2;
            finalizeGraph(result);
            finalizeNode(current_node);
            finalizeBasicBlock(bblock);
            return NULL;
         }

         /* add the new basic block to the control flow graph */
         error2 = insertBlock(result, bblock);
         if (error2 != AXE_OK) {
            if (error)
               *error = error2;
            finalizeGraph(result);
            finalizeNode(current_node);
            finalizeBasicBlock(bblock);
            return NULL;
         }
      } else {
         /* add the current instruction to the current
          * basic block */
         error2 = insertNode(bblock, current_node);
         if (error2 != AXE_OK) {
            if (error)
               *error = error2;
            finalizeGraph(result);
            finalizeNode(current_node);
            return NULL;
         }
      }

      if (endingNode)
         bblock = NULL;

      /* retrieve the next element */
      current_element = current_element->next;
   }

   /* update the basic blocks chain */
   error2 = updateFlowGraph(result);
   if (error2 != AXE_OK) {
      if (error)
         *error = error2;
      finalizeGraph(result);
      return NULL;
   }

   /* return the graph */
   return result;
}

int iterateCFGNodes(t_cflow_Graph *graph, void *context,
      int (*callback)(t_basic_block *block, t_cflow_Node *node, int nodeIndex,
            void *context))
{
   t_list *current_bb_element;
   t_list *current_nd_element;
   t_basic_block *current_block;
   t_cflow_Node *current_node;
   int counter, exitcode;

   /* intialize the instruction counter */
   counter = 0;

   /* fetch the first basic block */
   current_bb_element = graph->blocks;
   while (current_bb_element != NULL) {
      current_block = (t_basic_block *)current_bb_element->data;

      /* fetch the first node of the basic block */
      current_nd_element = current_block->nodes;
      while (current_nd_element != NULL) {
         current_node = (t_cflow_Node *)current_nd_element->data;

         /* invoke the callback */
         exitcode = callback(current_block, current_node, counter, context);
         if (exitcode != 0)
            return exitcode;

         /* fetch the next node in the basic block */
         counter++;
         current_nd_element = current_nd_element->next;
      }

      /* fetch the next element in the list of basic blocks */
      current_bb_element = current_bb_element->next;
   }

   return exitcode;
}

void updateProgramFromCFG(t_program_infos *program, t_cflow_Graph *graph)
{
   t_list *current_bb_element;
   t_list *current_nd_element;
   t_basic_block *bblock;
   t_cflow_Node *node;

   /* preconditions */
   assert(program != NULL);
   assert(graph != NULL);

   /* erase the old code segment */
   freeList(program->instructions);
   program->instructions = NULL;

   current_bb_element = graph->blocks;
   while (current_bb_element != NULL) {
      bblock = (t_basic_block *)current_bb_element->data;

      current_nd_element = bblock->nodes;
      while (current_nd_element != NULL) {
         node = (t_cflow_Node *)current_nd_element->data;

         program->instructions =
               addElement(program->instructions, node->instr, -1);

         current_nd_element = current_nd_element->next;
      }

      current_bb_element = current_bb_element->next;
   }
}

t_list *getLiveOUTVars(t_basic_block *bblock)
{
   t_list *last_Element;
   t_cflow_Node *lastNode;

   if (bblock == NULL)
      return NULL;
   if (bblock->nodes == NULL)
      return NULL;

   last_Element = getLastElement(bblock->nodes);
   lastNode = (t_cflow_Node *)last_Element->data;
   assert(lastNode != NULL);

   /* return a copy of the list of variables live in
    * input to the current basic block */
   return cloneList(lastNode->out);
}

t_list *getLiveINVars(t_basic_block *bblock)
{
   t_cflow_Node *firstNode;

   if (bblock == NULL)
      return NULL;
   if (bblock->nodes == NULL)
      return NULL;

   firstNode = (t_cflow_Node *)bblock->nodes->data;
   assert(firstNode != NULL);

   /* return a copy of the list of variables live in
    * input to the current basic block */
   return cloneList(firstNode->in);
}

t_list *addVariableToSet(t_list *set, t_cflow_var *element, int *modified)
{
   /* test the preconditions */
   if (element == NULL)
      return set;

   if (findElementWithCallback(set, element, compareCFGVariables) == NULL) {
      set = addElement(set, element, -1);
      if (modified != NULL)
         (*modified) = 1;
   }

   /* postconditions */
   return set;
}

t_list *addVariables(t_list *set, t_list *elements, int *modified)
{
   /* test the preconditions */
   if (set == NULL || elements == NULL)
      return set;

   /* update the set of variables */
   set = addListToSet(set, elements, compareCFGVariables, modified);

   /* postconditions: return the new list of variables */
   return set;
}

int performLivenessOnBlock(t_basic_block *bblock, t_list *out)
{
   t_list *current_element;
   t_list *cloned_list;
   t_cflow_Node *next_node;
   t_cflow_Node *current_node;
   int modified;
   int i, def_i, use_i;

   /* initialize the local variables */
   modified = 0;

   assert(bblock != NULL && bblock->nodes != NULL);

   current_element = getLastElement(bblock->nodes);
   current_node = (t_cflow_Node *)current_element->data;
   assert(current_node != NULL);

   /* update the out set */
   current_node->out = addListToSet(current_node->out, out, NULL, &modified);

   /* update the in list */
   cloned_list = cloneList(current_node->out);

   for (i = 0; i < CFLOW_MAX_USES; i++) {
#ifdef CFLOW_ALWAYS_LIVEIN_R0
      if ((current_node->uses)[i] != NULL &&
            (current_node->uses)[i]->ID != REG_0)
         cloned_list =
               addVariableToSet(cloned_list, (current_node->uses)[i], NULL);
#else
      if ((current_node->uses)[i] != NULL)
         cloned_list =
               addVariableToSet(cloned_list, (current_node->uses)[i], NULL);
#endif
   }

   for (def_i = 0; def_i < CFLOW_MAX_DEFS; def_i++) {
      int found = 0;
#ifdef CFLOW_ALWAYS_LIVEIN_R0
      if (!(current_node->defs)[def_i] ||
            (current_node->defs)[def_i]->ID == REG_0)
         continue;
#else
      if (!(current_node->defs)[def_i])
         continue;
#endif

      for (use_i = 0; use_i < CFLOW_MAX_USES && !found; use_i++) {
         if ((current_node->uses)[use_i]) {
            if ((current_node->uses)[use_i]->ID ==
                  (current_node->defs)[def_i]->ID)
               found = 1;
         }
      }

      if (!found)
         cloned_list = removeElement(cloned_list, current_node->defs[def_i]);
   }

   current_node->in =
         addListToSet(current_node->in, cloned_list, NULL, &modified);

   /* remove the cloned list */
   freeList(cloned_list);

   /* set the new value of next_node */
   next_node = current_node;
   current_element = current_element->prev;
   while (current_element != NULL) {
      /* take a new node */
      current_node = (t_cflow_Node *)current_element->data;
      assert(current_node != NULL);

      /* clone the `in' list of the next_node */
      cloned_list = cloneList(next_node->in);

      /* update the out list */
      current_node->out =
            addListToSet(current_node->out, cloned_list, NULL, &modified);

      /* remove the cloned list */
      freeList(cloned_list);

      /* clone the `in' list of the next_node */
      cloned_list = cloneList(current_node->out);

      /* update the in list */
      for (i = 0; i < CFLOW_MAX_USES; i++) {
#ifdef CFLOW_ALWAYS_LIVEIN_R0
         if ((current_node->uses)[i] != NULL &&
               (current_node->uses)[i]->ID != REG_0)
            cloned_list =
                  addVariableToSet(cloned_list, (current_node->uses)[i], NULL);
#else
         if ((current_node->uses)[i] != NULL)
            cloned_list =
                  addVariableToSet(cloned_list, (current_node->uses)[i], NULL);
#endif
      }

      for (def_i = 0; def_i < CFLOW_MAX_DEFS; def_i++) {
         int found = 0;
#ifdef CFLOW_ALWAYS_LIVEIN_R0
         if (!(current_node->defs)[def_i] ||
               (current_node->defs)[def_i]->ID == REG_0)
            continue;
#else
         if (!(current_node->defs)[def_i])
            continue;
#endif

         for (use_i = 0; use_i < CFLOW_MAX_USES && !found; use_i++) {
            if ((current_node->uses)[use_i]) {
               if ((current_node->uses)[use_i]->ID ==
                     (current_node->defs)[def_i]->ID)
                  found = 1;
            }
         }

         if (!found)
            cloned_list = removeElement(cloned_list, current_node->defs[def_i]);
      }

      current_node->in =
            addListToSet(current_node->in, cloned_list, NULL, &modified);

      /* remove the cloned list */
      freeList(cloned_list);

      /* update the loop control informations */
      current_element = current_element->prev;
      next_node = current_node;
   }

   /* return the `modified' value */
   return modified;
}

t_list *computeLiveOutVars(t_cflow_Graph *graph, t_basic_block *block)
{
   t_list *current_elem;
   t_basic_block *current_succ;
   t_list *result;
   t_list *liveINVars;

   /* preconditions */
   assert(block != NULL);
   assert(graph != NULL);

   /* initialize `current_elem' */
   current_elem = block->succ;

   /* initialize `result' */
   result = NULL;
   while (current_elem != NULL) {
      current_succ = (t_basic_block *)current_elem->data;
      assert(current_succ != NULL);

      if (current_succ != graph->endingBlock) {
         liveINVars = getLiveINVars(current_succ);

         /* update the value of `result' */
         result = addListToSet(result, liveINVars, NULL, NULL);

         /* free the temporary list of live intervals */
         freeList(liveINVars);
      }

      current_elem = current_elem->next;
   }

   /* postconditions */
   return result;
}

int performLivenessIteration(t_cflow_Graph *graph)
{
   int modified;
   t_list *current_element;
   t_basic_block *current_bblock;

   /* initialize the value of the local variable `modified' */
   modified = 0;

   /* test the preconditions */
   assert(graph != NULL);

   /* test if `graph->endingBlock' is valid */
   assert(graph->endingBlock != NULL);

   /* retrieve the last basic block in the list */
   current_element = getLastElement(graph->blocks);

   while (current_element != NULL) {
      t_list *live_out_vars;

      current_bblock = (t_basic_block *)current_element->data;
      assert(current_bblock != NULL);

      /* retrieve the variables that will be live out from this block */
      live_out_vars = computeLiveOutVars(graph, current_bblock);

      /* retrieve the liveness informations for the current bblock */
      if (performLivenessOnBlock(current_bblock, live_out_vars))
         modified = 1;

      /* remove the list `out' */
      freeList(live_out_vars);

      /* retrieve the previous element in the list */
      current_element = current_element->prev;
   }

   /* return 1 if another liveness iteration is required */
   return modified;
}

void performLivenessAnalysis(t_cflow_Graph *graph)
{
   int modified;

   do {
      modified = performLivenessIteration(graph);
   } while (modified);
}

void printCFlowGraphVariable(t_cflow_var *var, FILE *fout)
{
   if (var->ID == VAR_PSW)
      fprintf(fout, "PSW");
   else if (var->ID == VAR_UNDEFINED)
      fprintf(fout, "<!UNDEF!>");
   else
      fprintf(fout, "x%d", var->ID);
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
      current_variable = (t_cflow_var *) current_element->data;
      printCFlowGraphVariable(current_variable, fout);
      if (current_element->next != NULL)
         fprintf(fout, ", ");
      
      current_element = current_element->next;
   }
   fflush(fout);
}

void printBBlockInfos(t_basic_block *block, FILE *fout, int verbose)
{
   t_list *elem;
   t_cflow_Node *current_node;
   int count;

   /* preconditions */
   if (block == NULL)
      return;
   if (fout == NULL)
      return;

   fprintf(fout, "Number of predecessors: %d\n", getLength(block->pred));
   fprintf(fout, "Number of successors:   %d\n", getLength(block->succ));
   fprintf(fout, "Number of instructions: %d\n", getLength(block->nodes));

   count = 1;
   elem = block->nodes;
   while (elem != NULL) {
      current_node = (t_cflow_Node *)elem->data;

      fprintf(fout, "%3d. ", count);
      if (current_node->instr == NULL)
         fprintf(fout, "(null)");
      else
         printInstruction(current_node->instr, fout, 0);
      fprintf(fout, "\n");

      if (verbose != 0) {
         fprintf(fout, "     DEFS = [");
         printArrayOfVariables(current_node->defs, CFLOW_MAX_DEFS, fout);
         fprintf(fout, "]\n");

         fprintf(fout, "     USES = [");
         printArrayOfVariables(current_node->uses, CFLOW_MAX_USES, fout);
         fprintf(fout, "]\n");

         fprintf(fout, "     LIVE IN = [");
         printListOfVariables(current_node->in, fout);
         fprintf(fout, "]\n");
         fprintf(fout, "     LIVE OUT = [");
         printListOfVariables(current_node->out, fout);
         fprintf(fout, "]\n");
      }

      count++;
      elem = elem->next;
   }
   fflush(fout);
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

   fprintf(fout, "*************************\n");
   fprintf(fout, "    CONTROL FLOW GRAPH   \n");
   fprintf(fout, "*************************\n\n");

   fprintf(fout, "%s",
         "NOTE: Temporary registers are considered as variables of the\n"
         "intermediate language.\n");
#ifdef CFLOW_ALWAYS_LIVEIN_R0
   fprintf(fout, "%s",
         "  Variable \'x0\' (which refers to the physical register \'zero\') is\n"
         "always considered LIVE-IN for each node of a basic block.\n"
         "Thus, in the following control flow graph, \'x0\' will never appear\n"
         "as LIVE-IN or LIVE-OUT variable for a statement.\n"
         "  If you want to consider \'x0\' as a normal variable, you have to\n"
         "un-define the macro CFLOW_ALWAYS_LIVEIN_R0 in \"cflow_graph.h\"."
         "\n\n");
#endif

   fprintf(fout, "--------------\n");
   fprintf(fout, "  STATISTICS\n");
   fprintf(fout, "--------------\n\n");

   fprintf(fout, "Number of basic blocks:   %d\n", getLength(graph->blocks));
   fprintf(fout, "Number of used variables: %d\n\n",
         getLength(graph->cflow_variables));

   fprintf(fout, "----------------\n");
   fprintf(fout, "  BASIC BLOCKS\n");
   fprintf(fout, "----------------\n\n");

   counter = 1;
   current_element = graph->blocks;
   while (current_element != NULL) {
      current_bblock = (t_basic_block *)current_element->data;
      fprintf(fout, "[BLOCK %d]\n", counter);
      printBBlockInfos(current_bblock, fout, verbose);
      fprintf(fout, "\n");

      counter++;
      current_element = current_element->next;
   }
   fflush(fout);
}
