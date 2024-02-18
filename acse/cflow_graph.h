/// @file cflow_graph.h
/// @brief Control Flow Graph generation and related analyses

#ifndef CFLOW_GRAPH_H
#define CFLOW_GRAPH_H

#include <stdio.h>
#include <stdbool.h>
#include "program.h"
#include "list.h"

/**
 * @defgroup cflow_graph Control Flow Graph
 * @brief Control Flow Graph generation and related analyses
 *
 * Once the program has been translated to an initial assembly-like intermediate
 * language, the compiler needs to allocate each temporary register to a
 * physical register. However, the register allocation process requires
 * computing the liveness intervals of all temporary registers, which in
 * turn requires building the Control Flow Graph (CFG). These functions
 * and data structures are used to perform the construction of the graph and
 * the liveness analysis.
 * @{
 */

/// Maximum number of temporary register definitions for each node
#define CFG_MAX_DEFS 1
/// Maximum number of temporary register uses for each node
#define CFG_MAX_USES 2

/// Type for error codes returned by the CFG construction subsystem
typedef int t_cfgError;
enum {
  CFG_NO_ERROR = 0,
  CFG_ERROR_INVALID_NODE = 1000,
  CFG_ERROR_NODE_ALREADY_INSERTED,
  CFG_ERROR_BBLOCK_ALREADY_INSERTED
};


/** Data structure which uniquely identifies a register used or defined by a
 * node in a basic block */
typedef struct {
  /// Register identifier
  t_regID tempRegID;
  /// Physical register whitelist. Used by the register allocator.
  t_listNode *mcRegWhitelist;
} t_cfgReg;

/** Node in a basic block. Represents an instruction, the temporary registers
 * it uses and/or defines, and live temporary registers in/out of the node. */
typedef struct {
  /// Pointer to the instruction associated with this node
  t_instruction *instr;
  /// Set of registers defined by this node ('def' set). NULL slots are ignored.
  t_cfgReg *defs[CFG_MAX_DEFS];
  /// Set of registers used by this node ('use' set). NULL slots are ignored
  t_cfgReg *uses[CFG_MAX_USES];
  /// Set of registers live at the entry of the node ('in' set).
  t_listNode *in;
  /// Set of registers live at the exit of the node ('out' set).
  t_listNode *out;
} t_cfgNode;

/** Structure representing a basic block, i.e. a segment of contiguous
 * instructions with no branches in the middle. The use of basic blocks allows
 * -- without loss of generality -- to minimize the number of edges in the
 * Control Flow Graph, increasing the performance of code analysis. */
typedef struct {
  t_listNode *pred;  /// List of predecessors to this basic block.
  t_listNode *succ;  /// List of successors to this basic block.
  t_listNode *nodes; /// List of instructions in the block.
} t_basicBlock;

/** Data structure describing a control flow graph */
typedef struct {
  /// List of all the basic blocks, in program order.
  t_listNode *blocks;
  /// Unique final basic block. The control flow must eventually reach here.
  /// This block is always empty, and is not part of the 'blocks' list.
  t_basicBlock *endingBlock;
  /// List of all temporary registers used in the program
  t_listNode *registers;
} t_cfg;


/// @name Instruction Nodes
/// @{

/** Allocate a new node for a given Control Flow Graph.
 *  @param graph The Control Flow Graph where the node will be put.
 *  @param instr The instruction which will be represented by the node.
 *  @returns The new node. */
t_cfgNode *createCFGNode(t_cfg *graph, t_instruction *instr);
/** Free a given Control Flow Graph node.
 *  @param node The node to be freed. */
void deleteCFGNode(t_cfgNode *node);

/// @}


/// @name Basic Blocks
/// @{

/** Allocate a new empty basic block
 *  @returns The new block. */
t_basicBlock *newBasicBlock(void);
/** Frees the memory associated with a given basic block
 *  @param block The block to be freed. */
void deleteBasicBlock(t_basicBlock *block);

/** Adds a predecessor to a basic block.
 *  @param block The successor block.
 *  @param pred The predecessor block. */
void bbAddPred(t_basicBlock *block, t_basicBlock *pred);
/** Adds a successor to a basic block.
 *  @param block The predecessor block.
 *  @param pred The successor block. */
void bbAddSucc(t_basicBlock *block, t_basicBlock *succ);

/** Inserts a new node at the end of a block.
 *  @param block The block where to insert the node.
 *  @param node The node to insert.
 *  @returns CFG_NO_ERROR if the operation succeeded, otherwise
 *           CFG_ERROR_NODE_ALREADY_INSERTED if the node to insert is already
 *           present in the block. */
t_cfgError bbInsertNode(t_basicBlock *block, t_cfgNode *node);
/** Inserts a new node before another inside a basic block.
 *  @param block    The block where to insert the node.
 *  @param ip       The node at the insertion point. Must not be NULL.
 *  @param newNode  The node to insert.
 *  @returns CFG_NO_ERROR if the operation succeeded, otherwise
 *           CFG_ERROR_INVALID_NODE if ip does not belong to the
 *           given basic block, or CFG_ERROR_NODE_ALREADY_INSERTED if the node
 *           to insert is already present in the block. */
t_cfgError bbInsertNodeBefore(
    t_basicBlock *block, t_cfgNode *ip, t_cfgNode *newNode);
/** Inserts a new node after another inside a basic block.
 *  @param block    The block where to insert the node.
 *  @param ip       The node at the insertion point. Must not be NULL.
 *  @param newNode  The node to insert.
 *  @returns CFG_NO_ERROR if the operation succeeded, otherwise
 *           CFG_ERROR_INVALID_NODE if ip does not belong to the
 *           given basic block, or CFG_ERROR_NODE_ALREADY_INSERTED if the node
 *           to insert is already present in the block. */
t_cfgError bbInsertNodeAfter(
    t_basicBlock *block, t_cfgNode *ip, t_cfgNode *newNode);

/// @}


/// @name Control Flow Graph construction
/// @{

/** Creates a new control flow graph (CFG) from a program.
 *  @param program The program to be analyzed and converted into a CFG.
 *  @returns The new control flow graph, or NULL in case of error. */
t_cfg *programToCFG(t_program *program);
/** Frees a control flow graph.
 *  @param graph The graph to be freed. */
void deleteCFG(t_cfg *graph);

/** Inserts a new block in a control flow graph. Before invoking this function,
 *  the block must be linked to the others in the graph with bbAddPred and
 *  bbAddSucc.
 *  @param graph The graph where the block must be added.
 *  @param block The block to add.
 *  @returns CFG_NO_ERROR if the operation succeeded, otherwise
 *           CFG_ERROR_BBLOCK_ALREADY_INSERTED if the block was already inserted
 *           in the graph. */
t_cfgError cfgInsertBlock(t_cfg *graph, t_basicBlock *block);

/** Iterates through the nodes in a control flow graph.
 *  @param graph The graph that must be iterated over.
 *  @param context The context pointer that will be passed to the callback
 *         function.
 *  @param callback The callback function that will be called at each node
 *         found. The callback can return a non-zero value to stop the iteration
 *         process.
 *  @returns The value returned by the last callback invocation. */
int cfgIterateNodes(t_cfg *graph, void *context,
    int (*callback)(
        t_basicBlock *block, t_cfgNode *node, int nodeIndex, void *context));

/** Rebuilds a program from the given CFG.
 *  @param program The program to be modified
 *  @param graph The control flow graph to be linearized and transformed into a
 *         new program. */
void cfgToProgram(t_program *program, t_cfg *graph);

/// @}


/// @name Data Flow Analysis
/// @{

/** Computes graph-level liveness information of temporary registers.
 *  @param graph The control flow graph. */
void cfgComputeLiveness(t_cfg *graph);

/** Retrieve the list of live temporary registers entering the given block.
 * Only valid after calling cfgComputeLiveness() on the graph.
 * @param bblock The basic block.
 * @return The list of registers. The list is dynamically allocated and the
 *         caller is responsible for freeing it. */
t_listNode *bbGetLiveIn(t_basicBlock *bblock);
/** Retrieve the list of live temporary registers exiting the given block. Only
 * valid after calling cfgComputeLiveness() on the graph.
 * @param bblock The basic block.
 * @return The list of registers. The list is dynamically allocated and the
 *         caller is responsible for freeing it. */
t_listNode *bbGetLiveOut(t_basicBlock *bblock);

/// @}


/// @name Utilities
/// @{

/** Print debug information about the control flow graph.
 * @param graph The graph to log information about.
 * @param fout The output file.
 * @param verbose Pass a non-zero value to also print additional information
 *        about the liveness of the registers. */
void cfgDump(t_cfg *graph, FILE *fout, bool verbose);

/// @}


/**
 * @}
 */

#endif
