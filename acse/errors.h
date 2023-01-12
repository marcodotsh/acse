/// @file errors.h
/// @brief Error handling

#ifndef ERRORS_H
#define ERRORS_H

/* Warnings */
#define WARN_DIVISION_BY_ZERO 1
#define WARN_INVALID_SHIFT_AMOUNT 2
#define WARN_OVERFLOW 3

/* Errors */
#define NO_ERROR 0
#define ERROR_OUT_OF_MEMORY 1
#define ERROR_INVALID_INSTRUCTION 3
#define ERROR_VARIABLE_ALREADY_DECLARED 5
#define ERROR_INVALID_TYPE 6
#define ERROR_FOPEN_ERROR 7
#define ERROR_FCLOSE_ERROR 8
#define ERROR_FWRITE_ERROR 10
#define ERROR_INVALID_DATA_FORMAT 11
#define ERROR_INVALID_OPCODE 12
#define ERROR_INVALID_ARRAY_SIZE 15
#define ERROR_INVALID_EXPRESSION 18
#define ERROR_LABEL_ALREADY_ASSIGNED 20
#define ERROR_INVALID_CFLOW_GRAPH 23
#define ERROR_INVALID_REG_ALLOC 24
#define ERROR_REG_ALLOC_ERROR 25
#define ERROR_VARIABLE_TYPE_MISMATCH 28
#define ERROR_VARIABLE_NOT_DECLARED 29

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
