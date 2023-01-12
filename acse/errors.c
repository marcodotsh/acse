/// @file errors.c

#include <assert.h>
#include "errors.h"
#include "utils.h"
#include "cflow_graph.h"

/* @see Acse.y for these declarations */
extern int num_warning;
extern int num_error;
extern int line_num;


static const char *errorToString(int errorcode)
{
   const char *msg;

   switch (errorcode) {
      case ERROR_OUT_OF_MEMORY: msg = "Out of memory"; break;
      case ERROR_INVALID_INSTRUCTION: msg = "Invalid instruction"; break;
      case ERROR_VARIABLE_ALREADY_DECLARED:
         msg = "Variable already declared";
         break;
      case ERROR_INVALID_TYPE: msg = "Invalid type"; break;
      case ERROR_FOPEN_ERROR: msg = "fopen failed"; break;
      case ERROR_FCLOSE_ERROR: msg = "fclose failed"; break;
      case ERROR_FWRITE_ERROR: msg = "Error while writing on file"; break;
      case ERROR_INVALID_DATA_FORMAT: msg = "Invalid data format"; break;
      case ERROR_INVALID_OPCODE: msg = "Invalid opcode found"; break;
      case ERROR_INVALID_ARRAY_SIZE: msg = "Invalid array size"; break;
      case ERROR_INVALID_EXPRESSION: msg = "Invalid expression found"; break;
      case ERROR_LABEL_ALREADY_ASSIGNED: msg = "label already assigned"; break;
      case ERROR_INVALID_CFLOW_GRAPH:
         msg = "Invalid control-dataflow graph informations";
         break;
      case ERROR_INVALID_REG_ALLOC:
         msg = "Invalid register allocator instance found";
         break;
      case ERROR_REG_ALLOC_ERROR: msg = "register allocation failed"; break;
      case ERROR_VARIABLE_TYPE_MISMATCH: msg = "type of the variable does not match"; break;
      case ERROR_VARIABLE_NOT_DECLARED: msg = "variable not declared"; break;
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
