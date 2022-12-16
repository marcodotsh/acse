/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 *
 * utils.h
 * Formal Languages & Compilers Machine, 2007/2008
 *
 * Contains important functions to access the list of symbols and other
 * utility functions and macros.
 */

#ifndef UTILS_H
#define UTILS_H

#include "program.h"
#include "list.h"

/** Maximum between two values. */
#define MAX(x, y) ((x) > (y) ? (x) : (y))

/** Minimum between two values. */
#define MIN(x, y) ((x) > (y) ? (y) : (x))

/** Utility structure used to collect information about variable
 * declarations. */
typedef struct t_declaration {
   char *ID;      /* variable identifier (should never be a NULL pointer
                   * or an empty string "") */
   int isArray;   /* must be TRUE if the current variable is an array */
   int arraySize; /* the size of the array. This information is useful
                   * only if the field `isArray' is TRUE */
} t_declaration;

/** Utility structure used to store information about a while statement. */
typedef struct t_whileStatement {
   t_label *label_condition; /* this label points to the expression
                              * that is used as loop condition */
   t_label *label_end;       /* this label points to the instruction
                              * that follows the while construct */
} t_whileStatement;


/** In debug builds (NDEBUG not defined), prints a message on the standard
 * output like `printf'. Otherwise, does nothing and returns zero. */
extern int debugPrintf(const char *fmt, ...);

/** Allocate and intialize a new variable declaration structure. */
extern t_declaration *newDeclaration(
      char *ID, int isArray, int arraySize);

/** Create a variable for each `t_declaration' inside the list `variables'.
 * Each new variable will be of type `varType'. */
extern void addVariablesFromDecls(
      t_program *program, int varType, t_listNode *variables);


#endif
