/// @file symbols.h
/// @brief Symbol handling (both scalar variables and arrays)

#ifndef VARIABLES_H
#define VARIABLES_H

#include "program.h"
#include "utils.h"
#include <stdbool.h>

/** Supported data types */
typedef enum {
  TYPE_INT,      ///< `int' scalar type
  TYPE_INT_ARRAY ///< `int' array type
} t_symbolType;

/** A structure that represents the properties of a given symbol in the source
 *  code */
typedef struct t_symbol {
  t_symbolType type; ///< A valid data type
  char *ID;          ///< Symbol name (should never be a NULL pointer or an
                     ///  empty string "")
  int reg_location;  ///< For scalar variables only, the register ID associated
                     ///  to the variable.
  int arraySize;     ///< For arrays only, the size of the array.
  t_label *label;    ///< For arrays only, a label that refers to the location
                     ///  of the variable inside the data segment
} t_symbol;


/** Add a symbol to the program.
 * @param program    The program where to add the symbol.
 * @param ID         The name (or identifier) of the new symbol.
 * @param type       The data type of the variable associated to the symbol.
 * @param arraySize  For arrays, the size of the array.
 * @returns A pointer to the newly created symbol object. */
t_symbol *createSymbol(
    t_program *program, char *ID, t_symbolType type, int arraySize);

/** Deletes all symbols in the given list.
 * @param symbols The list of `t_symbol's to be deleted. */
void deleteSymbols(t_listNode *symbols);

/** Get information about a previously added symbol.
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


/** Given a scalar variable symbol object, return the ID of the register where
 * the value of the variable is stored in the compiled program.
 * @param program The program where the variable belongs.
 * @param var     The symbol object of the variable.
 * @returns The register identifier of the variable or, if an error occurs,
 *          REG_INVALID. */
int getRegLocationOfVariable(t_program *program, t_symbol *var);

/** Generate instructions that load the address of an element of an array in a
 * register.
 * @param program The program where the array belongs.
 * @param array   The symbol object that refers to an array.
 * @param index   An expression that refers to a specific element of the array.
 * @returns The identifier of the register that (at runtime) will contain the
 *          address of the array element at position `index'. */
int genLoadArrayAddress(
    t_program *program, t_symbol *array, t_expressionValue index);

/** Generate instructions that load the content of an element of an array in a
 * register.
 * @param program The program where the array belongs.
 * @param array   The symbol object that refers to an array.
 * @param index   An expression that refers to a specific element of the array.
 * @returns The identifier of the register that (at runtime) will contain the
 *          value of the array element at position `index'. */
int genLoadArrayElement(
    t_program *program, t_symbol *array, t_expressionValue index);

/** Generate instructions that store a value into a given array element.
 * @param program The program where the array belongs.
 * @param array   The symbol object that refers to an array
 * @param index   An expression that refers to a specific element of the array.
 * @param data    An expression that refers to the value to be stored in the
 *                array element specified by `index'. */
void genStoreArrayElement(t_program *program, t_symbol *array,
    t_expressionValue index, t_expressionValue data);


#endif
