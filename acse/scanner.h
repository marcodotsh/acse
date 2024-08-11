/// @file scanner.h
/// @brief Header file associated to scanner.y

#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>
#include "errors.h"

/**
 * @addtogroup parser
 * @{
 */

/** The current location in the file being read by the scanner. */
extern t_fileLocation curFileLoc;

/** The file currently being read by the scanner.
 *  @note This global variable is used by Flex-generated code. */
extern FILE *yyin;

/** Scans the input up to the next token.
 *  @note This function is defined in Flex-generated code.
 *  @return The next token identifier. */
int yylex(void);

/**
 * @}
 */

#endif
