/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 *
 * utils.c
 * Formal Languages & Compilers Machine, 2007/2008
 *
 */

#include <assert.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdarg.h>
#include "utils.h"
#include "gencode.h"
#include "cflow_graph.h"
#include "reg_alloc.h"
#include "options.h"
#include "errors.h"
#include "target_info.h"
#include "variables.h"


int debugPrintf(const char *fmt, ...)
{
#ifndef NDEBUG
   int res;
   va_list args;

   va_start(args, fmt);
   res = vprintf(fmt, args);
   va_end(args);
   return res;
#else
   return 0;
#endif
}


t_declaration *newDeclaration(char *ID, int isArray, int arraySize)
{
   t_declaration *result;
   char *name;

   /* allocate memory for the new declaration */
   result = (t_declaration *)malloc(sizeof(t_declaration));
   if (result == NULL)
      fatalError(AXE_OUT_OF_MEMORY);

   name = strdup(ID);
   if (!name) {
      free(result);
      fatalError(AXE_OUT_OF_MEMORY);
   }

   /* initialize the content of `result' */
   result->isArray = isArray;
   result->arraySize = arraySize;
   result->ID = name;

   /* return the just created and initialized instance of t_declaration */
   return result;
}


void addVariablesFromDecls(
      t_program *program, int varType, t_listNode *variables)
{
   t_listNode *current_element;
   t_declaration *current_decl;

   /* preconditions */
   assert(program != NULL);

   /* initialize `current_element' */
   current_element = variables;

   while (current_element != NULL) {
      /* retrieve the current declaration infos */
      current_decl = (t_declaration *)current_element->data;
      assert(current_decl != NULL);

      /* create and assign a new variable to program */
      createVariable(program, current_decl->ID, varType, current_decl->isArray,
            current_decl->arraySize);

      /* update the value of `current_element' */
      current_element = current_element->next;
   }

   /* free the linked list */
   /* initialize `current_element' */
   current_element = variables;

   while (current_element != NULL) {
      /* retrieve the current declaration infos */
      current_decl = (t_declaration *)current_element->data;

      /* assertion -- must always be verified */
      assert(current_decl != NULL);

      /* associate a register to each declared variable
       * that is not an array type */
      if (!(current_decl->isArray))
         getRegLocationOfScalar(program, current_decl->ID);

      /* free the memory associated with the current declaration */
      free(current_decl);

      /* update the value of `current_element' */
      current_element = current_element->next;
   }

   freeList(variables);
}
