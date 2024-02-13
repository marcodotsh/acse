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
 */

/* Errors */
#define NO_ERROR 0
#define ERROR_INVALID_CFLOW_GRAPH 23
#define ERROR_REG_ALLOC_ERROR 25

extern int num_error;

char *getLogFileName(const char *logType);

/** Prints a warning message depending on the given code.
 * @param warningcode The code of the warning. */
void emitWarning(const char *format, ...);

/** Prints an error message depending on the given code.
 * Does not terminate the program.
 * @param errorcode The error code. */
void emitError(const char *format, ...);

/** Prints the specified error message and terminates the program.
 * @param errorcode The error code. */
__attribute__((noreturn)) void fatalError(const char *format, ...);

#endif
