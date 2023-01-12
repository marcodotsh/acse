/// @file cflow_graph.h
/// @brief Control-Flow-Graph generation and related analyses

#ifndef CFLOW_GRAPH_H
#define CFLOW_GRAPH_H

#include <stdio.h>
#include "program.h"
#include "list.h"

/* If this macro is defined, the control flow analysis will always consider
 * x0 as a LIVE IN temporary register (i.e. variable) */
#define CFG_T0_ALWAYS_LIVE

/* Max number of defs and uses for each cfg node */
#define CFG_MAX_DEFS 2
#define CFG_MAX_USES 3

/* Error codes */
enum {
  ERROR_CFG_INVALID_NODE = 1000,
  ERROR_CFG_INVALID_LABEL_FOUND,
  ERROR_CFG_NODE_ALREADY_INSERTED,
  ERROR_CFG_BBLOCK_ALREADY_INSERTED
};

/* Special variables */
#define VAR_PSW -2
#define VAR_UNDEFINED -1


/* A variable of the intermediate code */
typedef struct t_cfgVar {
  /* Variable identifier. Negative IDs are reserved for artificial
   * variables which are not part of the code */
  int ID;
  /* Physical register whitelist */
  t_listNode *mcRegWhitelist;
} t_cfgVar;

/* A Node exists only in a basic block. It defines a list of
 * def-uses and it is associated with a specific instruction
 * inside the code */
typedef struct t_cfgNode {
  /* set of variables defined by this node */
  t_cfgVar *defs[CFG_MAX_DEFS];
  /* set of variables that will be used by this node */
  t_cfgVar *uses[CFG_MAX_USES];
  /* a pointer to the instruction associated with this node */
  t_instruction *instr;
  /* variables that are live-in the current node */
  t_listNode *in;
  /* variables that are live-out the current node */
  t_listNode *out;
} t_cfgNode;

/* an ordered list of nodes with only one predecessor and one successor */
typedef struct t_basicBlock {
  t_listNode *pred;  /* predecessors : a list of basic blocks */
  t_listNode *succ;  /* successors : a list of basic blocks */
  t_listNode *nodes; /* an ordered list of instructions */
} t_basicBlock;

/* a control flow graph */
typedef struct t_cfg {
  t_basicBlock *startingBlock; /* the starting basic block of code */
  t_basicBlock *endingBlock;   /* the last block of the graph */
  t_listNode *blocks;          /* an ordered list of all the basic blocks */
  t_listNode *cflow_variables; /* a list of all the variable identifiers */
} t_cfg;


/* Nodes */

/** Allocate a new node for a given Control Flow Graph.
 *  @param graph The Control Flow Graph where the node will be put.
 *  @param instr The instruction which will be represented by the node.
 *  @returns The new node. */
extern t_cfgNode *cfgCreateNode(t_cfg *graph, t_instruction *instr);
/** Free a given Control Flow Graph node.
 *  @param node The node to be freed. */
extern void deleteCFGNode(t_cfgNode *node);


/* Basic Blocks */

/** Allocate a new empty basic block
 *  @returns The new block. */
extern t_basicBlock *newBasicBlock();
/** Frees the memory associated with a given basic block
 *  @param block The block to be freed. */
extern void deleteBasicBlock(t_basicBlock *block);

/** Adds a predecessor to a basic block.
 *  @param block The successor block.
 *  @param pred The predecessor block. */
extern void bbSetPred(t_basicBlock *block, t_basicBlock *pred);
/** Adds a successor to a basic block.
 *  @param block The predecessor block.
 *  @param pred The successor block. */
extern void bbSetSucc(t_basicBlock *block, t_basicBlock *succ);

/** Inserts a new node at the end of a block.
 *  @param block The block where to insert the node.
 *  @param node The node to insert.
 *  @returns NO_ERROR if the operation succeeded, otherwise an error code. */
extern int bbInsertNode(t_basicBlock *block, t_cfgNode *node);
/** Inserts a new node before another inside a basic block.
 *  @param block The block where to insert the node.
 *  @param before_node The node at the insertion point. Must not be NULL.
 *  @param new_node The node to insert.
 *  @returns NO_ERROR if the operation succeeded, otherwise an error code. */
extern int bbInsertNodeBefore(
    t_basicBlock *block, t_cfgNode *before_node, t_cfgNode *new_node);
/** Inserts a new node after another inside a basic block.
 *  @param block The block where to insert the node.
 *  @param before_node The node at the insertion point. Must not be NULL.
 *  @param new_node The node to insert.
 *  @returns NO_ERROR if the operation succeeded, otherwise an error code. */
extern int bbInsertNodeAfter(
    t_basicBlock *block, t_cfgNode *after_node, t_cfgNode *new_node);


/* Control Flow Graph */

/** Creates a new control flow graph (CFG) from a program.
 *  @param program The program to be analyzed and converted into a CFG.
 *  @param error Points to a variable that will be set to an error
 *               code if an error occurs.
 *  @returns The new control flow graph, or NULL in case of error. */
extern t_cfg *programToCFG(t_program *program, int *error);
/** Frees a control flow graph.
 *  @param graph The graph to be freed. */
extern void deleteCFG(t_cfg *graph);

/** Inserts a new block in a control flow graph. Before invoking this function,
 *  the block must be linked to the others in the graph with bbSetPred and
 *  bbSetSucc.
 *  @param graph The graph where the block must be added.
 *  @param block The block to add.
 *  @returns NO_ERROR if the operation succeeded, otherwise an error code. */
extern int cfgInsertBlock(t_cfg *graph, t_basicBlock *block);

/** Iterates through the nodes in a control flow graph.
 *  @param graph The graph that must be iterated over.
 *  @param context The context pointer that will be passed to the callback
 *         function.
 *  @param callback The callback function that will be called at each node
 *         found. The callback can return 1 to stop the iteration process.
 *  @returns The value returned by the last callback invocation. */
extern int cfgIterateNodes(t_cfg *graph, void *context,
    int (*callback)(
        t_basicBlock *block, t_cfgNode *node, int nodeIndex, void *context));

/** Rebuilds a program from the given CFG.
 *  @param program The program to be modified
 *  @param graph The control flow graph to be linearized and transformed into a
 *         new program. */
extern void cfgToProgram(t_program *program, t_cfg *graph);


/* Data Flow Analysis */

/** Computes graph-level liveness information of the variables.
 *  @param graph The control flow graph. */
extern void cfgComputeLiveness(t_cfg *graph);

/** Retrieve the list of live variables entering the given block. Only valid
 *  after calling cfgComputeLiveness() on the graph.
 *  @param bblock The basic block.
 *  @return The list of variables. The list is dynamically allocated and must be
 *          freed. */
extern t_listNode *bbGetLiveInVars(t_basicBlock *bblock);
/** Retrieve the list of live variables when exiting the given block. Only valid
 *  after calling cfgComputeLiveness() on the graph.
 *  @param bblock The basic block.
 *  @return The list of variables. The list is dynamically allocated and must be
 *          freed. */
extern t_listNode *bbGetLiveOutVars(t_basicBlock *bblock);


/* Utilities */

/** Print debug information about the control flow graph.
 * @param graph The graph to log information about.
 * @param fout The output file.
 * @param verbose Pass a non-zero value to also print additional information
 *        about the liveness of the variables. */
extern void cfgDump(t_cfg *graph, FILE *fout, int verbose);

#endif
