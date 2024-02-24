/// @file program.h
/// @brief Contains t_program and some functions for label management
///        (reserve, fix, assign)

#ifndef PROGRAM_H
#define PROGRAM_H

#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include "list.h"

/**
 * @defgroup program Program Intermediate Representation
 * @brief Definitions and functions for handling the compiler IR.
 * 
 * During the compilation process, the program is built instruction by
 * instruction by the code in the syntactic-directed translator in parser.y.
 * The contents of the program are stored in an intermediate data structure of
 * type t_program.
 * 
 * In successive compilation steps, the instructions and data declarations
 * in the program intermediate representation are modified to make them conform
 * to the requirements of the target machine, and at the end they are
 * written to the output assembly file (see target_asm_print.h).
 * @{
 */


/// Type for register identifiers.
typedef int t_regID;


/// Constant used for invalid register identifiers.
#define REG_INVALID ((t_regID)(-1))
/// Constant identifying a register whose value is always zero.
#define REG_0       ((t_regID)(0))

/** Supported data types */
typedef enum {
  TYPE_INT,      ///< `int' scalar type
  TYPE_INT_ARRAY ///< `int' array type
} t_symbolType;


/** Object representing a label in the output assembly file.
 * @note A label object does not uniquely identify a label, its labelID field
 * does. This is used for aliasing multiple label objects to the same
 * physical label if more than one label is assigned to an instruction. */
typedef struct {
  unsigned int labelID; ///< Unique numeric identifier for the label.
  char *name;           ///< Name of the label. If NULL, the name will be
                        ///  automatically generated in the form L<ID>.
  bool global;          ///< True if the label will be defined as 'global'.
  bool isAlias;         ///< True if this label object is an alias to another
                        ///  one with the same labelID.
} t_label;

/** Object representing a register argument to an instruction. */
typedef struct {
  t_regID ID;                 ///< The register identifier
  t_listNode *mcRegWhitelist; ///< the list of machine registers where this
                              ///  argument may be allocated. NULL if any
                              ///  machine register is allowed.
} t_instrArg;

/** Object representing a symbolic assembly instruction. */
typedef struct {
  t_label *label;        ///< Label associated with the instruction, or NULL
  int opcode;            ///< Instruction opcode
  t_instrArg *rDest;     ///< Destination argument (or NULL if none)
  t_instrArg *rSrc1;     ///< First source argument (or NULL if none)
  t_instrArg *rSrc2;     ///< Second source argument (or NULL if none)
  int immediate;         ///< Immediate argument
  t_label *addressParam; ///< Address argument
  char *comment;         ///< A comment string associated with the instruction,
                         ///  or NULL if none.
} t_instruction;

/** A structure that represents the properties of a given symbol in the source
 * code */
typedef struct t_symbol {
  t_symbolType type;  ///< A valid data type
  char *ID;           ///< Symbol name (should never be a NULL pointer or an
                      ///  empty string "")
  t_label *label;     ///< A label that refers to the location of the variable
                      ///  inside the data segment
  int arraySize;      ///< For arrays only, the size of the array.
} t_symbol;

/** Object containing the program's intermediate representation during the
 * compilation process. */
typedef struct {
  t_listNode *labels;            ///< List of all labels
  t_listNode *instructions;      ///< List of instructions
  t_listNode *symbols;           ///< Symbol table
  t_regID firstUnusedReg;        ///< Next unused register ID
  unsigned int firstUnusedLblID; ///< Next unused label ID
  t_label *pendingLabel;         ///< Next pending label to assign
} t_program;


/// @name Construction/destruction of a program
/// @{

/** Create a new empty program object. */
t_program *newProgram(void);

/** Delete a program object. 
 * @param The program object to delete. */
void deleteProgram(t_program *program);

/// @}


/// @name Handling of labels
/// @{

/** Reserve a new label object, unassigned to any instruction.
 * @param program The program where the label belongs
 * @returns The new label object. */
t_label *createLabel(t_program *program);

/** Assign the given label object to the next instruction to be generated.
 * @param program The program where the label belongs
 * @param label   The label to be assigned */
void assignLabel(t_program *program, t_label *label);

/** Sets the name of a label to the specified string.
 * @param program The program where the label belongs
 * @param label   The label whose name to set
 * @param name    The string which will be used as label name
 * @note If another label with the same name already exists, the name assigned
 * to this label will be modified to remove any ambiguity. */
void setLabelName(t_program *program, t_label *label, const char *name);

/** Obtain the name of a given label
 * @param label The label
 * @returns A dynamically allocated string. The caller owns the string and is
 *          responsible for freeing it. */
char *getLabelName(t_label *label);

/// @}


/// @name Generation of instructions
/// @{

/** Obtain a currently unused temporary register identifier.
 * @param program The program where the register will be used.
 * @returns The identifier of the register. */
t_regID getNewRegister(t_program *program);

/** Add a new instruction at the end the current program's list of instructions.
 * @param program   The program where to add the instruction
 * @param opcode    Identifier for the operation performed by the instruction
 * @param r_dest    Identifier of the destination register argument,
 *                  or REG_INVALID if none
 * @param r_src1    Identifier of the first source register argument,
 *                  or REG_INVALID if none
 * @param r_src2    Identifier of the second source register argument,
 *                  or REG_INVALID if none
 * @param label     Label object representing the label argument,
 *                  or NULL if none
 * @param immediate Immediate argument to the instruction, if needed
 * @warning This is a low-level primitive for generating instructions. This
 *          function is not aware of the semantic meaning of each operation
 *          code, and performs no parameter checking. As a result using this
 *          function directly may result in the generation of invalid
 *          instructions.
 *          Use the helper functions in gencode.h instead for generating
 *          instructions. */
t_instruction *genInstruction(t_program *program, int opcode, t_regID r_dest,
    t_regID r_src1, t_regID r_src2, t_label *label, int immediate);

/** Remove an instruction from the program, given its node in the instruction
 * list.
 * @param program The program where to remove the instruction
 * @param instrLi Node in the instruction list to remove */
void removeInstructionAt(t_program *program, t_listNode *instrLi);

/// @}


/// @name Handling of symbols
/// @{

/** Add a symbol to the program.
 * @param program    The program where to add the symbol.
 * @param ID         The identifier (name) of the new symbol.
 * @param type       The data type of the variable associated to the symbol.
 * @param arraySize  For arrays, the size of the array.
 * @returns A pointer to the newly created symbol object. */
t_symbol *createSymbol(
    t_program *program, char *ID, t_symbolType type, int arraySize);

/** Lookup a previously added symbol.
 * @param program The program where the symbol belongs.
 * @param ID      The identifier of the symbol.
 * @returns A pointer to the corresponding symbol object, or NULL if the symbol
 *          has not been declared yet. */
t_symbol *getSymbol(t_program *program, char *ID);

/** Checks if the type of the given symbol is an array type.
 * @param symbol The symbol object.
 * @returns `true' if the type of the symbol is a kind of array, otherwise
 *          returns `false'. */
bool isArray(t_symbol *symbol);

/// @}


/// @name Utility functions
/// @{

/** Generates the final instruction sequence required at the end of a program.
 * @param program The program to be modified. */
void genProgramEpilog(t_program *program);

/** Dumps the current state of a program object in the specified file.
 * @param program The program which will be dumped
 * @param fout    The file where to print the dump */
void dumpProgram(t_program *program, FILE *fout);

/// @}

/**
 * @}
 */

#endif
