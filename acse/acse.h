/// @file acse.h
/// @brief Main file of the ACSE compiler

#ifndef ACSE_H
#define ACSE_H

#include <stdio.h>

/**
 * \mainpage ACSE: Advanced Compiler System for Education
 * 
 * ACSE (Advanced Compiler System for Education) is a simple compiler
 * developed for educational purposes for the course "Formal Languages and
 * Compilers". ACSE, together with its supporting tools, aims to be a
 * representative example of a complete computing system – albeit simplified –
 * in order to illustrate what happens behind the scenes when a program is
 * compiled and then executed.
 * 
 * This compiler which accepts a program in a simplified C-like language called
 * LANCE (Language for Compiler Education), and produces a compiled program in
 * standard RISC-V RV32IM assembly language.
 * 
 * ## Naming conventions and code style
 * 
 * All symbols are in lower camel case. Additionally, ACSE employs a consistent
 * naming convention for all functions, in order to introduce a simple form of
 * namespacing which groups together each function based on their role.
 * Specifically:
 *  - Functions that allocate and initialize a structure are named starting
 *    with "new"
 *  - Functions that allocate and initialize a structure, and then add it to
 *    a parent object are named starting with "create"
 *  - Functions that deinitialize and deallocate a structure are named
 *    starting with "delete"
 *  - Functions that operate on a given structure type are named with a prefix
 *    that represents the structure. Exception is made for functions that
 *    modify a t_program structure, which may have no specific prefix.
 *  - Functions that add instructions to the program are named starting with
 *    "gen".
 */

/**
 * @defgroup pipeline Compilation Pipeline
 * @brief Top-level functions that drive the compilation process
 */

extern int num_error;

/** In debug builds (NDEBUG not defined), prints a message on the standard
 * output like `printf'. Otherwise, does nothing and returns zero. */
int debugPrintf(const char *fmt, ...);

char *getLogFileName(const char *logType);

/** Prints an error message depending on the given code.
 * Does not terminate the program.
 * @param errorcode The error code. */
void emitError(const char *format, ...);

/** Prints the specified error message and terminates the program.
 * @param errorcode The error code. */
__attribute__((noreturn)) void fatalError(const char *format, ...);

#endif
