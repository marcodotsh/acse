/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * cflow_graph.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 * Control-Flow-Graph generation and related analyses
 */

#ifndef CFLOW_GRAPH_H
#define CFLOW_GRAPH_H

#include <stdio.h>
#include "program.h"
#include "list.h"

/* If this macro is defined, the control flow analysis will always consider
 * x0 as a LIVE IN temporary register (i.e. variable) */
#define CFLOW_ALWAYS_LIVEIN_R0

/* Max number of defs and uses for each cfg node */
#define CFLOW_MAX_DEFS 2
#define CFLOW_MAX_USES 3

/* Error codes */
enum {
   CFLOW_INVALID_NODE = 1000,
   CFLOW_INVALID_LABEL_FOUND,
   CFLOW_NODE_ALREADY_INSERTED,
   CFLOW_BBLOCK_ALREADY_INSERTED
};

/* Special variables */
#define VAR_PSW       -2
#define VAR_UNDEFINED -1


/* A variable of the intermediate code */
typedef struct t_cflow_var {
   /* Variable identifier. Negative IDs are reserved for artificial
    * variables which are not part of the code */
   int ID;
   /* Physical register whitelist */
   t_list *mcRegWhitelist;
} t_cflow_var;

/* A Node exists only in a basic block. It defines a list of
 * def-uses and it is associated with a specific instruction
 * inside the code */
typedef struct t_cflow_Node {
   /* set of variables defined by this node */
   t_cflow_var *defs[CFLOW_MAX_DEFS];
   /* set of variables that will be used by this node */
   t_cflow_var *uses[CFLOW_MAX_USES];
   /* a pointer to the instruction associated with this node */
   t_axe_instruction *instr;
   /* variables that are live-in the current node */
   t_list *in;
   /* variables that are live-out the current node */
   t_list *out;
} t_cflow_Node;

/* an ordered list of nodes with only one predecessor and one successor */
typedef struct t_basic_block {
   t_list *pred;  /* predecessors : a list of basic blocks */
   t_list *succ;  /* successors : a list of basic blocks */
   t_list *nodes; /* an ordered list of instructions */
} t_basic_block;

/* a control flow graph */
typedef struct t_cflow_Graph {
   t_basic_block *startingBlock; /* the starting basic block of code */
   t_basic_block *endingBlock;   /* the last block of the graph */
   t_list *blocks;               /* an ordered list of all the basic blocks */
   t_list *cflow_variables;      /* a list of all the variable identifiers */
} t_cflow_Graph;


/* Nodes */

/** Allocate a new node for a given Control Flow Graph.
 *  @param graph The Control Flow Graph where the node will be put.
 *  @param instr The instruction which will be represented by the node.
 *  @param error Points to a variable that will be set to an error
 *               code if an error occurs.
 *  @returns The new node, or NULL if an error occurred. */
extern t_cflow_Node *allocNode(
      t_cflow_Graph *graph, t_axe_instruction *instr, int *error);
/** Free a given Control Flow Graph node.
 *  @param node The node to be freed. */
extern void finalizeNode(t_cflow_Node *node);


/* Basic Blocks */

/** Allocate a new empty basic block
 *  @param error Points to a variable that will be set to an error
 *               code if an error occurs.
 *  @returns The new block, or NULL if an error occurred. */
extern t_basic_block *allocBasicBlock(int *error);
/** Frees the memory associated with a given basic block
 *  @param block The block to be freed. */
extern void finalizeBasicBlock(t_basic_block *block);

/** Adds a predecessor to a basic block.
 *  @param block The successor block.
 *  @param pred The predecessor block. */
extern void setPred(t_basic_block *block, t_basic_block *pred);
/** Adds a successor to a basic block.
 *  @param block The predecessor block.
 *  @param pred The successor block. */
extern void setSucc(t_basic_block *block, t_basic_block *succ);

/** Inserts a new node at the end of a block.
 *  @param block The block where to insert the node.
 *  @param node The node to insert.
 *  @returns AXE_OK if the operation succeeded, otherwise an error code. */
extern int insertNode(t_basic_block *block, t_cflow_Node *node);
/** Inserts a new node before another inside a basic block.
 *  @param block The block where to insert the node.
 *  @param before_node The node at the insertion point. Must not be NULL.
 *  @param new_node The node to insert.
 *  @returns AXE_OK if the operation succeeded, otherwise an error code. */
extern int insertNodeBefore(
      t_basic_block *block, t_cflow_Node *before_node, t_cflow_Node *new_node);
/** Inserts a new node after another inside a basic block.
 *  @param block The block where to insert the node.
 *  @param before_node The node at the insertion point. Must not be NULL.
 *  @param new_node The node to insert.
 *  @returns AXE_OK if the operation succeeded, otherwise an error code. */
extern int insertNodeAfter(
      t_basic_block *block, t_cflow_Node *after_node, t_cflow_Node *new_node);


/* Control Flow Graph */

/** Creates a new control flow graph (CFG) from a program.
 *  @param program The program to be analyzed and converted into a CFG.
 *  @param error Points to a variable that will be set to an error
 *               code if an error occurs.
 *  @returns The new control flow graph, or NULL in case of error. */
extern t_cflow_Graph *createFlowGraph(t_program_infos *program, int *error);
/** Frees a control flow graph.
 *  @param graph The graph to be freed. */
extern void finalizeGraph(t_cflow_Graph *graph);

/** Inserts a new block in a control flow graph. Before invoking this function,
 *  the block must be linked to the others in the graph with setPred and
 *  setSucc.
 *  @param graph The graph where the block must be added.
 *  @param block The block to add.
 *  @returns AXE_OK if the operation succeeded, otherwise an error code. */
extern int insertBlock(t_cflow_Graph *graph, t_basic_block *block);

/** Iterates through the nodes in a control flow graph.
 *  @param graph The graph that must be iterated over.
 *  @param context The context pointer that will be passed to the callback
 *         function.
 *  @param callback The callback function that will be called at each node
 *         found. The callback can return 1 to stop the iteration process.
 *  @returns The value returned by the last callback invocation. */
extern int iterateCFGNodes(t_cflow_Graph *graph, void *context,
      int (*callback)(t_basic_block *block, t_cflow_Node *node, int nodeIndex,
            void *context));

/** Rebuilds a program from the given CFG.
 *  @param program The program to be modified
 *  @param graph The control flow graph to be linearized and transformed into a
 *         new program. */
extern void updateProgramFromCFG(
      t_program_infos *program, t_cflow_Graph *graph);


/* Data Flow Analysis */

/** Computes graph-level liveness information of the variables.
 *  @param graph The control flow graph. */
extern void performLivenessAnalysis(t_cflow_Graph *graph);

/** Retrieve the list of live variables entering the given block. Only valid
 *  after calling performLivenessAnalysis() on the graph.
 *  @param bblock The basic block.
 *  @return The list of variables. The list is dynamically allocated and must be
 *          freed. */
extern t_list *getLiveINVars(t_basic_block *bblock);
/** Retrieve the list of live variables when exiting the given block. Only valid
 *  after calling performLivenessAnalysis() on the graph.
 *  @param bblock The basic block.
 *  @return The list of variables. The list is dynamically allocated and must be
 *          freed. */
extern t_list *getLiveOUTVars(t_basic_block *bblock);


/* Utilities */

/** Print debug information about the control flow graph.
 * @param graph The graph to log information about.
 * @param fout The output file.
 * @param verbose Pass a non-zero value to also print additional information
 *        about the liveness of the variables. */
extern void printGraphInfos(t_cflow_Graph *graph, FILE *fout, int verbose);

#endif
