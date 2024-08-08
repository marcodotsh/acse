/// @file parser.h
/// @brief Header file associated to parser.y

#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "program.h"
#include "codegen.h"

/**
 * @defgroup semdef Semantic Definitions
 * @brief Structure definitions used for semantic analysis
 * @{
 */

/** Utility structure used to store information about an if statement. */
typedef struct {
  t_label *lElse;  ///< Label to the else part
  t_label *lExit;  ///< Label to the first instruction after the statement
} t_ifStmt;

/** Utility structure used to store information about a while statement. */
typedef struct {
  t_label *lLoop;  ///< Label to the beginning of the loop
  t_label *lExit;  ///< Label to the first instruction after the loop
} t_whileStmt;

/**
 * @}
 */

/**
 * @addtogroup pipeline
 * @{
 */

/** Performs the initial syntactic-driven translation of the source code.
 * @param fp The source code file being compiled
 * @returns The program object produced */
t_program *parseProgram(char *fn);

/**
 * @}
 */

/* yyerror() is a function defined later in this file used by the bison-
 * generated parser to notify that a syntax error occurred. */
void yyerror(const char *msg);

#endif
