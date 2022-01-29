/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_errors.h
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 * Error handling
 */

#ifndef _AXE_ERRORS_H
#define _AXE_ERRORS_H

/* WARNINGS */
#define WARN_DIVISION_BY_ZERO 1
#define WARN_INVALID_SHIFT_AMOUNT 2
#define WARN_OVERFLOW 3

/* errorcodes */
#define AXE_OK 0
#define AXE_OUT_OF_MEMORY 1
#define AXE_PROGRAM_NOT_INITIALIZED 2
#define AXE_INVALID_INSTRUCTION 3
#define AXE_VARIABLE_ID_UNSPECIFIED 4
#define AXE_VARIABLE_ALREADY_DECLARED 5
#define AXE_INVALID_TYPE 6
#define AXE_FOPEN_ERROR 7
#define AXE_FCLOSE_ERROR 8
#define AXE_INVALID_INPUT_FILE 9
#define AXE_FWRITE_ERROR 10
#define AXE_INVALID_DATA_FORMAT 11
#define AXE_INVALID_OPCODE 12
#define AXE_INVALID_REGISTER_INFO 13
#define AXE_INVALID_LABEL 14
#define AXE_INVALID_ARRAY_SIZE 15
#define AXE_INVALID_VARIABLE 16
#define AXE_INVALID_ADDRESS 17
#define AXE_INVALID_EXPRESSION 18
#define AXE_UNKNOWN_VARIABLE 19
#define AXE_LABEL_ALREADY_ASSIGNED 20
#define AXE_INVALID_LABEL_MANAGER 21
#define AXE_NULL_DECLARATION 22
#define AXE_INVALID_CFLOW_GRAPH 23
#define AXE_INVALID_REG_ALLOC 24
#define AXE_REG_ALLOC_ERROR 25
#define AXE_TRANSFORM_ERROR 26
#define AXE_SYNTAX_ERROR 27
#define AXE_UNKNOWN_ERROR 28

extern void printWarningMessage(int warningcode);
extern void fatalError(int axe_errorcode);
extern void checkConsistency();

#endif
