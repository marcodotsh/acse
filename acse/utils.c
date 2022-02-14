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
#include "labels.h"
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

t_axe_declaration *initializeDeclaration(
      char *ID, int isArray, int arraySize, int init_val)
{
   t_axe_declaration *result;

   /* allocate memory for the new declaration */
   result = (t_axe_declaration *)malloc(sizeof(t_axe_declaration));
   if (result == NULL)
      fatalError(AXE_OUT_OF_MEMORY);

   /* initialize the content of `result' */
   result->isArray = isArray;
   result->arraySize = arraySize;
   result->ID = ID;
   result->init_val = init_val;

   /* return the just created and initialized instance of t_axe_declaration */
   return result;
}

static void free_new_variables(t_list *variables)
{
   t_list *current_element;
   t_axe_declaration *current_decl;

   /* preconditions */
   assert(variables != NULL);

   /* initialize the value of `current_element' */
   current_element = variables;
   while (current_element != NULL)
   {
      current_decl = (t_axe_declaration *)current_element->data;
      if (current_decl != NULL)
         free(current_decl);

      current_element = current_element->next;
   }

   /* free the memory associated with the list `variables' */
   freeList(variables);
}

void addVariablesFromDecls(t_program_infos *program, int varType, t_list *variables)
{
   t_list *current_element;
   t_axe_declaration *current_decl;

   /* preconditions */
   assert(program != NULL);

   /* initialize `current_element' */
   current_element = variables;

   while (current_element != NULL)
   {
      /* retrieve the current declaration infos */
      current_decl = (t_axe_declaration *)current_element->data;
      assert(current_decl != NULL);

      /* create and assign a new variable to program */
      createVariable(program, current_decl->ID, varType, current_decl->isArray, current_decl->arraySize, current_decl->init_val);

      /* update the value of `current_element' */
      current_element = current_element->next;
   }

   /* free the linked list */
   /* initialize `current_element' */
   current_element = variables;

   while (current_element != NULL)
   {
      /* retrieve the current declaration infos */
      current_decl = (t_axe_declaration *)current_element->data;

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

void setProgramEnd(t_program_infos *program)
{
   assert(program != NULL);

   if (isAssigningLabel(program->lmanager))
   {
      genHALTInstruction(program);
      return;
   }

   if (program->instructions != NULL)
   {
      t_axe_instruction *last_instr;
      t_list *last_element;

      /* get the last element of the list */
      last_element = getLastElement(program->instructions);
      assert(last_element != NULL);

      /* retrieve the last instruction */
      last_instr = (t_axe_instruction *)last_element->data;
      assert(last_instr != NULL);

      if (last_instr->opcode == OPC_HALT)
         return;
   }

   genHALTInstruction(program);
   return;
}
