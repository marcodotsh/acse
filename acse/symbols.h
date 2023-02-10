/// @file symbols.h
/// @brief Symbol handling (both scalar variables and arrays)

#ifndef VARIABLES_H
#define VARIABLES_H

#include "program.h"
#include "expressions.h"

/* Data types */
#define INTEGER_TYPE 0 /* int */

/** A structure that represents the properties of a given symbol in the source
 *  code */
typedef struct t_symbol {
  int type;         ///< A valid data type
  char *ID;         ///< Symbol name (should never be a NULL pointer or an empty
                    ///  string ""
  int isArray;      ///< Must be true if the current symbol refers to an array
  int reg_location; ///< If `isArray' is false, the register ID associated to
                    ///  the variable.
  int arraySize;    ///< If `isArray' is true, the size of the array.
  t_label *label;   ///< If `isArray' is true, a label that refers to the
                    ///  location of the variable inside the data segment
} t_symbol;


/** Add a symbol to the program.
 * @param program    The program where to add the symbol.
 * @param ID         The name (or identifier) of the new symbol.
 * @param type       The data type of the variable associated to the symbol.
 *                   Must be INTEGER_TYPE.
 * @param isArray    `0' if the symbol refers to a scalar variable, `1' if it
 *                   refers to an array.
 * @param arraySize  If isArray!=0, the size of the array.
 * @param init_val   If isArray==0, initial value of the variable.
 * @returns A pointer to the newly created symbol object. */
extern t_symbol *createSymbol(
    t_program *program, char *ID, int type, int isArray, int arraySize);

/** Deletes all symbols in the given list.
 * @param symbols The list of `t_symbol's to be deleted. */
extern void deleteSymbols(t_listNode *symbols);

/** Get information about a previously added symbol.
 * @param program The program where the symbol belongs.
 * @param ID      The identifier of the symbol.
 * @returns A pointer to the corresponding symbol object, or NULL if the symbol
 *          has not been declared yet. */
extern t_symbol *getSymbol(t_program *program, char *ID);


/** Given a scalar variable symbol object, return the ID of the register where
 * the value of the variable is stored in the compiled program.
 * @param program The program where the variable belongs.
 * @param var     The symbol object of the variable.
 * @returns The register identifier of the variable or, if an error occurs,
 *          REG_INVALID. */
extern int getRegLocationOfVariable(t_program *program, t_symbol *var);

/** Generate instructions that load the address of an element of an array in a
 * register.
 * @param program The program where the array belongs.
 * @param array   The symbol object that refers to an array.
 * @param index   An expression that refers to a specific element of the array.
 * @returns The identifier of the register that (at runtime) will contain the
 *          address of the array element at position `index'. */
extern int genLoadArrayAddress(
    t_program *program, t_symbol *array, t_expressionValue index);

/** Generate instructions that load the content of an element of an array in a
 * register.
 * @param program The program where the array belongs.
 * @param array   The symbol object that refers to an array.
 * @param index   An expression that refers to a specific element of the array.
 * @returns The identifier of the register that (at runtime) will contain the
 *          value of the array element at position `index'. */
extern int genLoadArrayElement(
    t_program *program, t_symbol *array, t_expressionValue index);

/** Generate instructions that store a value into a given array element.
 * @param program The program where the array belongs.
 * @param array   The symbol object that refers to an array
 * @param index   An expression that refers to a specific element of the array.
 * @param data    An expression that refers to the value to be stored in the
 *                array element specified by `index'. */
extern void genStoreArrayElement(t_program *program, t_symbol *array,
    t_expressionValue index, t_expressionValue data);


#endif
