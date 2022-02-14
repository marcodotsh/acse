/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 *
 * variables.c
 * Formal Languages & Compilers Machine, 2007/2008
 *
 */

#include <assert.h>
#include "variables.h"
#include "gencode.h"
#include "utils.h"
#include "errors.h"
#include "target_info.h"
#include "expressions.h"


/* finalize an instance of `t_axe_variable' */
void finalizeVariable(t_axe_variable *variable)
{
   free(variable);
}

/* create and initialize an instance of `t_axe_variable' */
t_axe_variable *initializeVariable(
      char *ID, int type, int isArray, int arraySize, int init_val)
{
   t_axe_variable *result;

   /* allocate memory for the new variable */
   result = (t_axe_variable *)malloc(sizeof(t_axe_variable));
   if (result == NULL)
      fatalError(AXE_OUT_OF_MEMORY);

   /* initialize the content of `result' */
   result->type = type;
   result->isArray = isArray;
   result->arraySize = arraySize;
   result->ID = ID;
   result->init_val = init_val;
   result->label = NULL;
   result->reg_location = REG_INVALID;

   /* return the just created and initialized instance of t_axe_variable */
   return result;
}

/* create a new variable */
void createVariable(t_program_infos *program, char *ID, int type, int isArray,
      int arraySize, int init_val)
{
   t_axe_variable *var, *variableFound;
   int sizeofElem;
   t_axe_data *new_data_info;
   int sy_error;

   /* test the preconditions */
   assert(program != NULL);
   assert(ID != NULL);

   if (type != INTEGER_TYPE)
      fatalError(AXE_INVALID_TYPE);

   /* check if another variable already exists with the same ID */
   variableFound = getVariable(program, ID);
   if (variableFound != NULL) {
      emitError(AXE_VARIABLE_ALREADY_DECLARED);
      return;
   }

   /* check if the array size is valid */
   if (isArray && arraySize <= 0) {
      emitError(AXE_INVALID_ARRAY_SIZE);
      return;
   }

   /* initialize a new variable */
   var = initializeVariable(ID, type, isArray, arraySize, init_val);

   if (isArray) {
      /* arrays are stored in memory and need a variable location */
      var->label = newLabel(program);
      setLabelName(program->lmanager, var->label, ID);

      /* create an instance of `t_axe_data' */
      sizeofElem = 4 / TARGET_PTR_GRANULARITY;
      new_data_info =
            initializeData(DIR_SPACE, var->arraySize * sizeofElem, var->label);

      /* update the list of directives */
      program->data = addElement(program->data, new_data_info, -1);
   } else {
      /* scalars are stored in registers */
      var->reg_location = genLoadImmediate(program, init_val);
   }

   /* now we can add the new variable to the program */
   program->variables = addElement(program->variables, var, -1);
}

void finalizeVariables(t_list *variables)
{
   t_list *current_element;
   t_axe_variable *current_var;

   if (variables == NULL)
      return;

   /* initialize the `current_element' */
   current_element = variables;
   while (current_element != NULL) {
      current_var = (t_axe_variable *)current_element->data;
      if (current_var != NULL) {
         if (current_var->ID != NULL)
            free(current_var->ID);

         free(current_var);
      }

      current_element = current_element->next;
   }

   /* free the list of variables */
   freeList(variables);
}

int compareVariables(void *Var_A, void *Var_B)
{
   t_axe_variable *va;
   t_axe_variable *vb;

   if (Var_A == NULL)
      return Var_B == NULL;

   if (Var_B == NULL)
      return 0;

   va = (t_axe_variable *)Var_A;
   vb = (t_axe_variable *)Var_B;

   /* test if the name is the same */
   return (!strcmp(va->ID, vb->ID));
}

t_axe_variable *getVariable(t_program_infos *program, char *ID)
{
   t_axe_variable search_pattern;
   t_list *elementFound;

   /* preconditions */
   assert(program != NULL);
   assert(ID != NULL);

   /* initialize the pattern */
   search_pattern.ID = ID;

   /* search inside the list of variables */
   elementFound = findElementWithCallback(
         program->variables, &search_pattern, compareVariables);

   /* if the element is found return it to the caller. Otherwise return NULL. */
   if (elementFound != NULL)
      return (t_axe_variable *)elementFound->data;

   return NULL;
}

int getRegLocationOfScalar(t_program_infos *program, char *ID)
{
   int sy_error;
   int location;
   t_axe_variable *var;

   /* preconditions: ID and program shouldn't be NULL pointer */
   assert(ID != NULL);
   assert(program != NULL);

   /* get the location of the variable with the given ID */
   var = getVariable(program, ID);
   if (var == NULL) {
      emitError(AXE_VARIABLE_NOT_DECLARED);
      return REG_INVALID;
   }
   if (var->isArray) {
      emitError(AXE_VARIABLE_TYPE_MISMATCH);
      return REG_INVALID;
   }

   location = var->reg_location;
   return location;
}

t_axe_label *getMemLocationOfArray(t_program_infos *program, char *ID)
{
   t_axe_variable *var;

   assert(ID != NULL);
   assert(program != NULL);

   var = getVariable(program, ID);

   if (var == NULL) {
      emitError(AXE_VARIABLE_NOT_DECLARED);
      return NULL;
   }
   if (!var->isArray) {
      emitError(AXE_VARIABLE_TYPE_MISMATCH);
      return NULL;
   }
   assert(var->label != NULL);

   return var->label;
}

void genStoreArrayElement(t_program_infos *program, char *ID,
      t_axe_expression index, t_axe_expression data)
{
   int address;

   address = genLoadArrayAddress(program, ID, index);

   if (data.type == REGISTER) {
      /* load the value indirectly into `mova_register' */
      genSWInstruction(program, data.registerId, 0, address);
   } else {
      int imm_register;

      imm_register = genLoadImmediate(program, data.immediate);

      /* load the value indirectly into `load_register' */
      genSWInstruction(program, imm_register, 0, address);
   }
}

int genLoadArrayElement(
      t_program_infos *program, char *ID, t_axe_expression index)
{
   int load_register;
   int address;

   /* retrieve the address of the array slot */
   address = genLoadArrayAddress(program, ID, index);

   /* get a new register */
   load_register = getNewRegister(program);

   /* load the value into `load_register' */
   genLWInstruction(program, load_register, 0, address);

   /* return the register ID that holds the required data */
   return load_register;
}

int genLoadArrayAddress(
      t_program_infos *program, char *ID, t_axe_expression index)
{
   int mova_register, sizeofElem;
   t_axe_label *label;

   /* preconditions */
   assert(program != NULL);
   assert(ID != NULL);

   /* retrieve the label associated with the given
    * identifier */
   label = getMemLocationOfArray(program, ID);

   /* test if an error occurred */
   if (label == NULL)
      return REG_INVALID;

   /* get a new register */
   mova_register = getNewRegister(program);

   /* generate the MOVA instruction */
   genLAInstruction(program, mova_register, label);

   /* We are making the following assumption:
    * the type can only be an INTEGER_TYPE */
   sizeofElem = 4 / TARGET_PTR_GRANULARITY;

   if (index.type == IMMEDIATE) {
      if (index.immediate != 0) {
         genADDIInstruction(program, mova_register, mova_register,
               index.immediate * sizeofElem);
      }
   } else {
      int idxReg;
      assert(index.type == REGISTER);

      idxReg = index.registerId;
      if (sizeofElem != 1) {
         idxReg = getNewRegister(program);
         genMULIInstruction(program, idxReg, index.registerId, sizeofElem);
      }

      genADDInstruction(program, mova_register, mova_register, idxReg);
   }

   /* return the identifier of the register that contains
    * the value of the array slot */
   return mova_register;
}
