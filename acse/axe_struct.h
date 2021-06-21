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

typedef struct t_axe_register
{
   int ID;        /* an identifier of the register */
   int indirect;  /* a boolean value: 1 if the register value is a pointer */
   t_list *mcRegWhitelist;  /* the list of machine registers where this 
                             * variable can be allocated. NULL if any register 
                             * is allowed. */
}t_axe_register;

/* ADDRESS TYPES */
#define ADDRESS_TYPE 0
#define LABEL_TYPE 1

typedef struct t_axe_address
{
   int addr;               /* a Program Counter */
   t_axe_label *labelID;   /* a label identifier */
   int type;               /* one of ADDRESS_TYPE or LABEL_TYPE */
}t_axe_address;

/* data types */
#define INTEGER_TYPE 0
#define UNKNOWN_TYPE -1

/* A structure that defines the internal data of a `Acse variable' */
typedef struct t_axe_variable
{
   int type;      /* a valid data type */
   int isArray;   /* must be TRUE if the current variable is an array */
   int arraySize; /* the size of the array. This information is useful only
                   * if the field `isArray' is TRUE */
   int init_val;  /* initial value of the current variable. Actually it is
                   * implemented as a integer value. `int' is
                   * the only supported type at the moment,
                   * future developments could consist of a modification of
                   * the supported type system. Thus, maybe init_val will be
                   * modified in future. */
   char *ID;               /* variable identifier (should never be a NULL
                            * pointer or an empty string "") */
   t_axe_label *labelID;   /* a label that refers to the location
                            * of the variable inside the data segment */
} t_axe_variable;

/* a symbolic assembly instruction */
typedef struct t_axe_instruction
{
   int opcode;                   /* instruction opcode (for example: ADD) */
   t_axe_register *reg_1;        /* destination register */
   t_axe_register *reg_2;        /* first source register */
   t_axe_register *reg_3;        /* second source register */
   int immediate;                /* immediate value */
   t_axe_address *address;       /* an address operand */
   int mcFlags;                  /* flags; used only by the backend */
   char *user_comment;           /* if defined it is set to the source code
                                  * instruction that generated the current
                                  * assembly. This string will be written
                                  * into the output code as a comment */
   t_axe_label *labelID;        /* a label associated with the current
                                  * instruction */
}t_axe_instruction;

/* DIRECTIVE TYPES */
#define DIR_WORD 0
#define DIR_SPACE 1
#define DIR_INVALID -1

/* this structure is used in order to define assembler directives.
 * Directives are used in many cases such the definition of variables
 * inside the data segment. Every instance `t_axe_data' contains
 * all the informations about a single directive.
 * An example is the directive .word that is required when the assembler
 * must reserve a word of data inside the data segment. */
typedef struct t_axe_data
{
   int directiveType;      /* the type of the current directive
                            * (for example: DIR_WORD) */
   int value;              /* the value associated with the directive */
   t_axe_label *labelID;   /* label associated with the current data */
}t_axe_data;

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

/* create an instance of `t_axe_register' */
extern t_axe_register *initializeRegister(int ID, int indirect);

/* create an instance of `t_axe_instruction' */
extern t_axe_instruction *initializeInstruction(int opcode);

/* create an instance of `t_axe_address' */
extern t_axe_address *initializeAddress(int type, int address, t_axe_label *label);

/* create an instance of `t_axe_data' */
extern t_axe_data *initializeData(int directiveType, int value, t_axe_label *label);

/* create an instance of `t_axe_variable' */
extern t_axe_variable *initializeVariable(
      char *ID, int type, int isArray, int arraySize, int init_val);

/* finalize an instance of `t_axe_variable' */
extern void finalizeVariable(t_axe_variable *variable);

/* create an instance of `t_axe_variable' */
extern t_axe_declaration *initializeDeclaration(
      char *ID, int isArray, int arraySize, int init_val);

/* finalize an instruction info. */
extern void finalizeInstruction(t_axe_instruction *inst);

/* finalize a data info. */
extern void finalizeData(t_axe_data *data);

#endif
