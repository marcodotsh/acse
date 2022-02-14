/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 *
 * variables.h
 * Formal Languages & Compilers Machine, 2007/2008
 *
 * Variable handling (both scalars and arrays)
 */

#ifndef VARIABLES_H
#define VARIABLES_H

#include "program.h"
#include "expressions.h"


/* Data types */
#define INTEGER_TYPE 0    /* int */
#define UNKNOWN_TYPE -1

/** A structure that defines the internal data of an `Acse variable' */
typedef struct t_axe_variable {
   int type;           /* A valid data type */
   char *ID;           /* Variable identifier (should never be a NULL
                        * pointer or an empty string "") */
   int isArray;        /* Must be TRUE if the current variable is an array */
   int init_val;       /* If `isArray' is FALSE, initial value of the
                        * variable. */
   int reg_location;   /* If `isArray' is FALSE, the register ID associated to
                        * the variable. */
   int arraySize;      /* If `isArray' is TRUE, the size of the array. */
   t_axe_label *label; /* If `isArray' is TRUE, a label that refers to the
                        * location of the variable inside the data segment */
} t_axe_variable;


/** Add a variable to the program.
 * @param program    The program where to add the variable.
 * @param ID         The name (or identifier) of the new variable.
 * @param type       The data type of the variable. Must be INTEGER_TYPE.
 * @param isArray    `0' if the variable is scalar, `1' otherwise.
 * @param arraySize  If isArray!=0, the size of the array.
 * @param init_val   If isArray==0, initial value of the variable. */
extern void createVariable(t_program_infos *program, char *ID, int type,
      int isArray, int arraySize, int init_val);

/** Deletes all variables in the given list.
 * @param variables The list of `t_axe_variable's to be deleted. */
extern void finalizeVariables(t_list *variables);


/** Get information about a previously added variable.
 * @param program The program where the variable belongs.
 * @param ID      The identifier of the variable. 
 * @returns A pointer to the t_axe_variable structure describing the
 *          variable, or NULL if the variable has not been declared yet. */
extern t_axe_variable *getVariable(t_program_infos *program, char *ID);

/** Given a scalar variable identifier, return the ID of the register where
 * the value of the variable is stored in the compiled program.
 * @param program The program where the variable belongs.
 * @param ID      The identifier of the variable. 
 * @returns The register identifier of the variable or, if an error occurs,
 *          REG_INVALID. */
extern int getRegLocationOfScalar(t_program_infos *program, char *ID);

/** Given an array variable identifier, get the label that marks the starting
 * address in memory of the contents of the array.
 * @param program The program where the array belongs.
 * @param ID      The identifier of the array. 
 * @returns The label associated with the array or, if an error occurs, NULL. */
extern t_axe_label *getMemLocationOfArray(t_program_infos *program, char *ID);


/** Generate instructions that load the address of an element of an array in a
 * register. 
 * @param program The program where the array belongs.
 * @param ID      A variable identifier (ID) that refers to an array
 * @param index   An expression that refers to a specific element of the array.
 * @returns The identifier of the register that (at runtime) will contain the
 *          address of the array element at position `index'. */
extern int genLoadArrayAddress(
      t_program_infos *program, char *ID, t_axe_expression index);

/** Generate instructions that load the content of an element of an array in a
 * register.
 * @param program The program where the array belongs.
 * @param ID      A variable identifier (ID) that refers to an array
 * @param index   An expression that refers to a specific element of the array.
 * @returns The identifier of the register that (at runtime) will contain the
 *          value of the array element at position `index'. */
extern int genLoadArrayElement(
      t_program_infos *program, char *ID, t_axe_expression index);

/** Generate instructions that store a value into a given array element.
 * @param program The program where the array belongs.
 * @param ID      A variable identifier (ID) that refers to an array
 * @param index   An expression that refers to a specific element of the array.
 * @param data    An expression that refers to the value to be stored in the
 *                array element specified by `index'. */
extern void genStoreArrayElement(t_program_infos *program, char *ID,
      t_axe_expression index, t_axe_expression data);


#endif
