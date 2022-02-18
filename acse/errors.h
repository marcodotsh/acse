/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * errors.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 * Error handling
 */

#ifndef ERRORS_H
#define ERRORS_H

/* Warnings */
#define WARN_DIVISION_BY_ZERO 1
#define WARN_INVALID_SHIFT_AMOUNT 2
#define WARN_OVERFLOW 3

/* Errors */
#define AXE_OK 0
#define AXE_OUT_OF_MEMORY 1
#define AXE_INVALID_INSTRUCTION 3
#define AXE_VARIABLE_ALREADY_DECLARED 5
#define AXE_INVALID_TYPE 6
#define AXE_FOPEN_ERROR 7
#define AXE_FCLOSE_ERROR 8
#define AXE_FWRITE_ERROR 10
#define AXE_INVALID_DATA_FORMAT 11
#define AXE_INVALID_OPCODE 12
#define AXE_INVALID_ARRAY_SIZE 15
#define AXE_INVALID_EXPRESSION 18
#define AXE_LABEL_ALREADY_ASSIGNED 20
#define AXE_INVALID_CFLOW_GRAPH 23
#define AXE_INVALID_REG_ALLOC 24
#define AXE_REG_ALLOC_ERROR 25
#define AXE_VARIABLE_TYPE_MISMATCH 28
#define AXE_VARIABLE_NOT_DECLARED 29

/** Prints a warning message depending on the given code.
 * @param warningcode The code of the warning. */
extern void emitWarning(int warningcode);

/** Prints an error message depending on the given code.
 * Does not terminate the program.
 * @param errorcode The error code. */
extern void emitError(int errorcode);

/** Prints the given syntax error message.
 * @param message A description of the specific syntax error. */
extern void emitSyntaxError(const char *message);

/** Prints the specified error message and terminates the program.
 * @param errorcode The error code. */
extern __attribute__((noreturn)) void fatalError(int errorcode);

#endif
