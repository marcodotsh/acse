/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * axe_struct.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#include "axe_struct.h"

t_while_statement createWhileStatement()
{
   t_while_statement statement;

   /* initialize the WHILE informations */
   statement.label_condition = NULL;
   statement.label_end = NULL;

   /* return a new instance of `t_while_statement' */
   return statement;
}

t_axe_declaration * initializeDeclaration
      (char *ID, int isArray, int arraySize, int init_val)
{
   t_axe_declaration *result;

   /* allocate memory for the new declaration */
   result = (t_axe_declaration *)
         malloc(sizeof(t_axe_declaration));

   /* check the postconditions */
   if (result == NULL)
      return NULL;

   /* initialize the content of `result' */
   result->isArray = isArray;
   result->arraySize = arraySize;
   result->ID = ID;
   result->init_val = init_val;

   /* return the just created and initialized instance of t_axe_declaration */
   return result;
}

