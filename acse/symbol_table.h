/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * symbol_table.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 * Internal functions for managing t_symbol_table.
 */

#ifndef _SYMBOL_TABLE_H
#define _SYMBOL_TABLE_H

#include <stdio.h>

/* errorcodes */
typedef int t_s_table_error;
#define SY_TABLE_OK 0
#define SY_ALREADY_DEFINED -1
#define SY_UNDEFINED -2
#define SY_TABLE_NOT_INITIALIZED -3
#define SY_MEMALLOC_ERROR -4
#define SY_INVALID_REQUEST -5

/* invalid location info */
#define SY_LOCATION_UNSPECIFIED -1

/* Typedef for the opaque struct t_symbol_table */
struct t_symbol_table;
typedef struct t_symbol_table t_symbol_table;

/* a symbol inside the sy_table. An element of the symbol table is composed by
 * three fields: <ID>, <type> and <Location>.
 * `ID' is a not-NULL string that is used as key identifier for a symbol
 * inside the table.
 * `type' is an integer value that is used to determine the correct type
 * of a symbol. Valid values for `type' are defined into "sy_table_constants.h".
 * `reg_location' refers to a register location (i.e. which register contains
 * the value of `ID'). */
typedef struct
{
   char *ID;            /* symbol identifier */
   int type;            /* type associated with the symbol */
   int reg_location;    /* a register location */
}t_symbol;

/* put a symbol into the symbol table */
extern t_s_table_error putSym(t_symbol_table *table, char *ID, int type);

/* set the location of the symbol with ID as identifier */
extern t_s_table_error setLocation(t_symbol_table *table, char *ID, int reg);

/* get the location of the symbol with the given ID */
extern int getLocation(t_symbol_table *table, char *ID,
      t_s_table_error *errorcode);

/* get the type associated with the symbol with ID as identifier */
extern int getTypeFromID(t_symbol_table *table, char *ID);

/* initialize the symbol table */
extern t_symbol_table * initializeSymbolTable();

/* finalize the symbol table */
extern t_s_table_error finalizeSymbolTable(t_symbol_table *table);

/* given a register identifier (location), it returns the ID of the variable
 * stored inside the register `location'. This function returns NULL
 * if the location is an invalid location. */
extern char *getSymbolNameFromReg(
      t_symbol_table *table, int location, int *errorcode);

#ifndef NDEBUG
/* This function prints out to the file `fout' the content of the
 * symbol table given as input. The resulting text is formatted in
 * the following way: <ID> -- <TYPE> -- <REGISTER> */
extern void printSymbolTable(t_symbol_table *table, FILE *fout);
#endif

#endif
