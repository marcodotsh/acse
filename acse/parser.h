/// @file parser.h
/// @brief Header file associated to parser.y

#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "program.h"

/** The global object representing the current intermediate compilation
 * outputs. */
extern t_program *program;

/** During parsing, this variable keeps track of the source code line number.
 * Every time a newline is encountered while scanning the input file, the
 * lexer increases this variable by 1. */
extern int line_num;

/** Performs the initial syntactic-driven translation of the source code. The
 * generated code is put into the global `program' data structure.
 * @param fp The source code file being compiled
 * @returns The number of syntax errors encountered. */
int parseProgram(FILE *fp);

/* yyerror() is a function defined later in this file used by the bison-
 * generated parser to notify that a syntax error occurred. */
void yyerror(const char *msg);

#endif
