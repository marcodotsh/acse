/// @file cfg.h
/// @brief Control Flow Graph generation and related analyses

#ifndef CFG_H
#define CFG_H

#include <stdio.h>
#include <stdbool.h>
#include "program.h"
#include "list.h"

/**
 * @defgroup cfg Control Flow Graph
 * @brief Control Flow Graph generation and related analyses
 *
 * These functions and data structures allow to build, inspect and partially
 * modify a Control Flow Graph (CFG) representation of a program.
 * A program can be converted to and from a CFG, and a CFG can be analyzed to
 * compute the liveness of each register.
 * These facilities are used by the register allocation process.
 * @{
 */

/// Maximum number of temporary register definitions for each node
#define CFG_MAX_DEFS 1
/// Maximum number of temporary register uses for each node
#define CFG_MAX_USES 2


/** Data structure which uniquely identifies a register used or defined by a
 * node in a basic block */
typedef struct {
  /// Register identifier
  t_regID tempRegID;
  /// Physical register whitelist. Used by the register allocator.
  t_listNode *mcRegWhitelist;
} t_cfgReg;

typedef struct t_basicBlock t_basicBlock;
typedef struct t_cfg t_cfg;

/** Node in a basic block. Represents an instruction, the temporary registers
 * it uses and/or defines, and live temporary registers in/out of the node. */
typedef struct {
  /// Pointer to the containing basic block.
  t_basicBlock *parent;
  /// Pointer to the instruction associated with this node.
  t_instruction *instr;
  /// Set of registers defined by this node ('def' set). NULL slots are ignored.
  t_cfgReg *defs[CFG_MAX_DEFS];
  /// Set of registers used by this node ('use' set). NULL slots are ignored
  t_cfgReg *uses[CFG_MAX_USES];
  /// Set of registers live at the entry of the node ('in' set).
  t_listNode *in;
  /// Set of registers live at the exit of the node ('out' set).
  t_listNode *out;
} t_bbNode;

/** Structure representing a basic block, i.e. a segment of contiguous
 * instructions with no branches in the middle. The use of basic blocks allows
 * -- without loss of generality -- to minimize the number of edges in the
 * Control Flow Graph, increasing the performance of code analysis. */
struct t_basicBlock {
  t_cfg *parent;     ///< The containing basic block.
  t_listNode *pred;  ///< List of predecessors to this basic block.
  t_listNode *succ;  ///< List of successors to this basic block.
  t_listNode *nodes; ///< List of instructions in the block.
};

/** Data structure describing a control flow graph */
struct t_cfg {
  /// List of all the basic blocks, in program order.
  t_listNode *blocks;
  /// Unique final basic block. The control flow must eventually reach here.
  /// This block is always empty, and is not part of the 'blocks' list.
  t_basicBlock *endingBlock;
  /// List of all temporary registers used in the program
  t_listNode *registers;
};


/// @name Basic Blocks
/// @{

/** Inserts a new instruction at the end of a block.
 *  @param block The block where to insert the instruction.
 *  @param instr The instruction to insert
 *  @returns The newly created basic block node. */
t_bbNode *bbInsertInstruction(t_basicBlock *block, t_instruction *instr);
/** Inserts a new instruction before another inside a basic block.
 *  @param block The block where to insert the instruction.
 *  @param instr The instruction to insert
 *  @param ip    The basic block node at the insertion point. Must not be NULL.
 *  @returns The newly created basic block node. */
t_bbNode *bbInsertInstructionBefore(
    t_basicBlock *block, t_instruction *instr, t_bbNode *ip);
/** Inserts a new instruction after another inside a basic block.
 *  @param block The block where to insert the instruction.
 *  @param instr The instruction to insert.
 *  @param ip    The basic block node at the insertion point. Must not be NULL.
 *  @returns The newly created basic block node. */
t_bbNode *bbInsertInstructionAfter(
    t_basicBlock *block, t_instruction *instr, t_bbNode *ip);

/// @}


/// @name Control Flow Graph construction
/// @{

/** Creates a new control flow graph (CFG) from a program.
 *  @param program The program to be analyzed and converted into a CFG.
 *  @returns The new control flow graph, or NULL in case of error. */
t_cfg *programToCFG(t_program *program);

/** Iterates through the nodes in a control flow graph.
 *  @param graph    The graph that must be iterated over.
 *  @param context  The context pointer that will be passed to the callback
 *                  function.
 *  @param callback The callback function that will be called at each node
 *                  found. The arguments of the callback are as follows:
 *                   - `block`: the current block
 *                   - `node`: the current node
 *                   - `nodeIndex`: an index, increasing at each new node
 *                   - `context`: the same context passed to cfgIterateNodes
 *                  The callback can return a non-zero value to stop the
 *                  iteration process.
 *  @returns The value returned by the last callback invocation. */
int cfgIterateNodes(t_cfg *graph, void *context,
    int (*callback)(t_bbNode *node, int nodeIndex, void *context));

/** Rebuilds a program from the given CFG.
 *  @param program The program to be modified
 *  @param graph The control flow graph to be linearized and transformed into a
 *         new program. */
void cfgToProgram(t_program *program, t_cfg *graph);

/** Frees a control flow graph.
 *  @param graph The graph to be freed. */
void deleteCFG(t_cfg *graph);

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
