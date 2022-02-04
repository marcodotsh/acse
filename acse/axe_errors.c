/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 *
 * axe_errors.c
 * Formal Languages & Compilers Machine, 2007/2008
 *
 */

#include <assert.h>
#include "axe_errors.h"
#include "axe_utils.h"
#include "axe_cflow_graph.h"

/* @see Acse.y for these declarations */
extern int num_warning;
extern int num_error;
extern int line_num;


static const char *errorToString(int errorcode)
{
   const char *msg;

   switch (errorcode) {
      case AXE_OUT_OF_MEMORY: msg = "Out of memory"; break;
      case AXE_PROGRAM_NOT_INITIALIZED: msg = "Program not initialized"; break;
      case AXE_INVALID_INSTRUCTION: msg = "Invalid instruction"; break;
      case AXE_VARIABLE_ID_UNSPECIFIED: msg = "Variable ID unspecified"; break;
      case AXE_VARIABLE_ALREADY_DECLARED:
         msg = "Variable already declared";
         break;
      case AXE_INVALID_TYPE: msg = "Invalid type"; break;
      case AXE_FOPEN_ERROR: msg = "fopen failed"; break;
      case AXE_FCLOSE_ERROR: msg = "fclose failed"; break;
      case AXE_INVALID_INPUT_FILE: msg = "Wrong file pointer"; break;
      case AXE_FWRITE_ERROR: msg = "Error while writing on file"; break;
      case AXE_INVALID_DATA_FORMAT: msg = "Invalid data format"; break;
      case AXE_INVALID_OPCODE: msg = "Invalid opcode found"; break;
      case AXE_INVALID_REGISTER_INFO: msg = "Invalid register infos"; break;
      case AXE_INVALID_LABEL: msg = "Invalid label found"; break;
      case AXE_INVALID_LABEL_MANAGER: msg = "Invalid label manager"; break;
      case AXE_INVALID_ARRAY_SIZE: msg = "Invalid array size"; break;
      case AXE_INVALID_VARIABLE: msg = "Invalid variable found"; break;
      case AXE_INVALID_ADDRESS: msg = "Invalid address"; break;
      case AXE_INVALID_EXPRESSION: msg = "Invalid expression found"; break;
      case AXE_UNKNOWN_VARIABLE: msg = "Unknown variable found"; break;
      case AXE_NULL_DECLARATION: msg = "NULL declaration found"; break;
      case AXE_LABEL_ALREADY_ASSIGNED: msg = "label already assigned"; break;
      case AXE_INVALID_CFLOW_GRAPH:
         msg = "Invalid control-dataflow graph informations";
         break;
      case AXE_INVALID_REG_ALLOC:
         msg = "Invalid register allocator instance found";
         break;
      case AXE_REG_ALLOC_ERROR: msg = "register allocation failed"; break;
      case AXE_TRANSFORM_ERROR:
         msg = "Invalid operation while modifying the instructions";
         break;
      case AXE_SYNTAX_ERROR: msg = "Syntax error found"; break;
      case AXE_UNKNOWN_ERROR:
      default: msg = "Unknown error"; break;
   }

   return msg;
}

static const char *warningToString(int warningcode)
{
   const char *msg;

   switch (warningcode) {
      case WARN_DIVISION_BY_ZERO: msg = "division by zero"; break;
      case WARN_INVALID_SHIFT_AMOUNT:
         msg = "shift amount is less than 0 or greater than 31";
         break;
      case WARN_OVERFLOW: msg = "overflow"; break;
      default: msg = "<invalid warning>"; break;
   }

   return msg;
}

static void printMessage(const char *msg, const char *category)
{
   if (line_num >= 0)
      fprintf(stderr, "At line %d, %s: %s.\n", line_num, category, msg);
   else
      fprintf(stderr, "%s: %s.\n", category, msg);
}

void emitError(int errorcode)
{
   const char *msg;

   /* Convert the error code to a string */
   msg = errorToString(errorcode);
   /* print out the error message to the standard error */
   printMessage(msg, "error");

   /* update the value of num_error */
   num_error++;
}

void emitWarning(int warningcode)
{
   const char *msg;

   /* Convert the warning code to a string */
   msg = warningToString(warningcode);
   /* print out the warning message to the standard error */
   printMessage(msg, "warning");

   /* update the value of num_warning */
   num_warning++;
}

void emitSyntaxError(const char *message)
{
   /* print out the error message to the standard error */
   printMessage(message, "error");

   /* update the value of num_error */
   num_error++;
}

void fatalError(int errorcode)
{
   const char *msg;

   /* Convert the error code to a string */
   msg = errorToString(errorcode);
   /* print out the error message to the standard error */
   printMessage(msg, "fatal error");

   exit(1);
}
