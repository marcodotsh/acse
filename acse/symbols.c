/// @file variables.c

#include <assert.h>
#include <stdbool.h>
#include "symbols.h"
#include "gencode.h"
#include "utils.h"
#include "errors.h"
#include "target_info.h"
#include "expressions.h"


/* create and initialize an instance of `t_symbol' */
t_symbol *newVariable(char *ID, t_symbolType type, int arraySize)
{
  t_symbol *result;

  /* allocate memory for the new variable */
  result = (t_symbol *)malloc(sizeof(t_symbol));
  if (result == NULL)
    fatalError(ERROR_OUT_OF_MEMORY);

  /* initialize the content of `result' */
  result->type = type;
  result->arraySize = arraySize;
  result->ID = ID;
  result->label = NULL;
  result->reg_location = REG_INVALID;

  /* return the just created and initialized instance of t_symbol */
  return result;
}


/* finalize an instance of `t_symbol' */
void deleteVariable(t_symbol *variable)
{
  free(variable);
}


t_symbol *createSymbol(
    t_program *program, char *ID, t_symbolType type, int arraySize)
{
  t_symbol *var, *variableFound;
  int sizeofElem;
  int sy_error;
  char *lblName;

  /* test the preconditions */
  assert(program != NULL);
  assert(ID != NULL);

  if (type != TYPE_INT && type != TYPE_INT_ARRAY)
    fatalError(ERROR_INVALID_TYPE);

  /* check if another variable already exists with the same ID */
  variableFound = getSymbol(program, ID);
  if (variableFound != NULL) {
    emitError(ERROR_VARIABLE_ALREADY_DECLARED);
    return NULL;
  }

  /* check if the array size is valid */
  if (type == TYPE_INT_ARRAY && arraySize <= 0) {
    emitError(ERROR_INVALID_ARRAY_SIZE);
    return NULL;
  }

  /* initialize a new variable */
  var = newVariable(ID, type, arraySize);

  if (type == TYPE_INT_ARRAY) {
    /* arrays are stored in memory and need a variable location */
    lblName = calloc(strlen(ID) + 8, sizeof(char));
    if (!lblName)
      fatalError(ERROR_OUT_OF_MEMORY);
    sprintf(lblName, "l_%s", ID);
    var->label = createLabel(program);
    setLabelName(program, var->label, lblName);
    free(lblName);

    /* statically declare the memory for the array */
    sizeofElem = 4 / TARGET_PTR_GRANULARITY;
    genDataDirective(
        program, DIR_SPACE, var->arraySize * sizeofElem, var->label);
  } else {
    /* scalars are stored in registers */
    var->reg_location = getNewRegister(program);
  }

  /* now we can add the new variable to the program */
  program->variables = addElement(program->variables, var, -1);
  return var;
}


void deleteSymbols(t_listNode *variables)
{
  t_listNode *current_element;
  t_symbol *current_var;

  if (variables == NULL)
    return;

  /* initialize the `current_element' */
  current_element = variables;
  while (current_element != NULL) {
    current_var = (t_symbol *)current_element->data;
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


bool isArray(t_symbol *symbol)
{
  // Just check if the type field corresponds to one of the known array types
  if (symbol->type == TYPE_INT_ARRAY)
    return true;
  return false;
}


bool compareVariables(void *Var_A, void *Var_B)
{
  t_symbol *va;
  t_symbol *vb;

  if (Var_A == NULL)
    return Var_B == NULL;

  if (Var_B == NULL)
    return false;

  va = (t_symbol *)Var_A;
  vb = (t_symbol *)Var_B;

  /* test if the name is the same */
  return strcmp(va->ID, vb->ID) == 0;
}


t_symbol *getSymbol(t_program *program, char *ID)
{
  t_symbol search_pattern;
  t_listNode *elementFound;

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
    return (t_symbol *)elementFound->data;

  return NULL;
}


int getRegLocationOfVariable(t_program *program, t_symbol *var)
{
  int sy_error;
  int location;

  /* preconditions: ID and program shouldn't be NULL pointer */
  assert(var != NULL);
  assert(program != NULL);

  if (isArray(var)) {
    emitError(ERROR_VARIABLE_TYPE_MISMATCH);
    return REG_INVALID;
  }

  location = var->reg_location;
  return location;
}


t_label *getMemLocationOfArray(t_program *program, t_symbol *array)
{
  t_symbol *var;

  assert(array != NULL);
  assert(program != NULL);

  if (!isArray(array)) {
    emitError(ERROR_VARIABLE_TYPE_MISMATCH);
    return NULL;
  }
  assert(array->label != NULL);

  return array->label;
}


void genStoreArrayElement(t_program *program, t_symbol *array, t_expressionValue index,
    t_expressionValue data)
{
  int address = genLoadArrayAddress(program, array, index);

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


int genLoadArrayElement(t_program *program, t_symbol *array, t_expressionValue index)
{
  int load_register;
  int address;

  /* retrieve the address of the array slot */
  address = genLoadArrayAddress(program, array, index);

  /* get a new register */
  load_register = getNewRegister(program);

  /* load the value into `load_register' */
  genLWInstruction(program, load_register, 0, address);

  /* return the register ID that holds the required data */
  return load_register;
}


int genLoadArrayAddress(t_program *program, t_symbol *array, t_expressionValue index)
{
  int mova_register, sizeofElem;
  t_label *label;

  /* preconditions */
  assert(program != NULL);
  assert(array != NULL);

  /* retrieve the label associated with the given
   * identifier */
  label = getMemLocationOfArray(program, array);

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

  if (index.type == CONSTANT) {
    if (index.immediate != 0) {
      genADDIInstruction(
          program, mova_register, mova_register, index.immediate * sizeofElem);
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
