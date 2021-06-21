/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_struct.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 * Fundamental data structures
 */

#ifndef _AXE_STRUCT_H
#define _AXE_STRUCT_H

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "axe_constants.h"
#include "axe_labels.h"
#include "collections.h"

typedef struct t_axe_declaration
{
   int isArray;           /* must be TRUE if the current variable is an array */
   int arraySize;         /* the size of the array. This information is useful
                           * only if the field `isArray' is TRUE */
   int init_val;          /* initial value of the current variable. */
   char *ID;              /* variable identifier (should never be a NULL pointer
                           * or an empty string "") */
} t_axe_declaration;

typedef struct t_while_statement
{
   t_axe_label *label_condition;   /* this label points to the expression
                                    * that is used as loop condition */
   t_axe_label *label_end;         /* this label points to the instruction
                                    * that follows the while construct */
} t_while_statement;

/* create an instance that will mantain infos about a while statement */
extern t_while_statement createWhileStatement();

/* create an instance of `t_axe_variable' */
extern t_axe_declaration *initializeDeclaration(
      char *ID, int isArray, int arraySize, int init_val);

#endif
