/// @file variables.c

#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include "symbols.h"
#include "gencode.h"
#include "utils.h"
#include "acse.h"
#include "target_info.h"


/* create and initialize an instance of `t_symbol' */
t_symbol *newSymbol(char *ID, t_symbolType type, int arraySize)
{
  t_symbol *result;

  /* allocate memory for the new variable */
  result = (t_symbol *)malloc(sizeof(t_symbol));
  if (result == NULL)
    fatalError("out of memory");

  /* initialize the content of `result' */
  result->type = type;
  result->arraySize = arraySize;
  result->ID = ID;
  result->label = NULL;

  /* return the just created and initialized instance of t_symbol */
  return result;
}


/* finalize an instance of `t_symbol' */
void deleteSymbol(t_symbol *s)
{
  free(s);
}


t_symbol *createSymbol(
    t_program *program, char *ID, t_symbolType type, int arraySize)
{
  // Test the preconditions
  if (type != TYPE_INT && type != TYPE_INT_ARRAY)
    fatalError("invalid type");

  // Check if another symbol already exists with the same ID
  t_symbol *existingSym = getSymbol(program, ID);
  if (existingSym != NULL) {
    emitError("variable '%s' already declared", ID);
    return NULL;
  }

  // Allocate and initialize a new symbol object
  t_symbol *res = newSymbol(ID, type, arraySize);

  // Reserve a new label for the variable
  char *lblName = calloc(strlen(ID) + 8, sizeof(char));
  if (!lblName)
    fatalError("out of memory");
  sprintf(lblName, "l_%s", ID);
  res->label = createLabel(program);
  setLabelName(program, res->label, lblName);
  free(lblName);

  // Insert a static declaration for the symbol
  int sizeOfVar;
  if (type == TYPE_INT_ARRAY) {
    // Check if the array size is valid
    if (arraySize <= 0) {
      emitError("invalid size %d for array %s", arraySize, ID);
      return NULL;
    }
    sizeOfVar = (4 / TARGET_PTR_GRANULARITY) * arraySize;
  } else {
    sizeOfVar = 4 / TARGET_PTR_GRANULARITY;
  }
  genDataDeclaration(program, DATA_SPACE, sizeOfVar, res->label);

  // Now we can add the new variable to the program
  program->variables = listInsert(program->variables, res, -1);
  return res;
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
  deleteList(variables);
}


bool isArray(t_symbol *symbol)
{
  // Just check if the type field corresponds to one of the known array types
  if (symbol->type == TYPE_INT_ARRAY)
    return true;
  return false;
}


static bool compareVariableWithIDString(void *a, void *b)
{
  t_symbol *var = (t_symbol *)a;
  char *str = (char *)b;
  return strcmp(var->ID, str) == 0;
}

t_symbol *getSymbol(t_program *program, char *ID)
{
  /* search inside the list of variables */
  t_listNode *elementFound = listFindWithCallback(
      program->variables, ID, compareVariableWithIDString);

  /* if the element is found return it to the caller. Otherwise return NULL. */
  if (elementFound != NULL)
    return (t_symbol *)elementFound->data;

  return NULL;
}


t_regID genLoadVariable(t_program *program, t_symbol *var)
{
  // Check if the symbol is an array; in that case bail out
  if (isArray(var))
    fatalError("'%s' is an array", var->ID); // TODO not fatal
  // Generate a LW from the address specified by the label
  t_regID reg = getNewRegister(program);
  genLWGlobal(program, reg, var->label);
  return reg;
}


void genStoreVariable(t_program *program, t_symbol *var, t_expressionValue val)
{
  // Check if the symbol is an array; in that case bail out
  if (isArray(var))
    fatalError("'%s' is a scalar", var->ID); // TODO not fatal
  // Materialize the expression value
  t_regID r_val = genConvertExpValueToRegister(program, val);
  // Reserve a new register which is a temporary required
  // by the pseudo-instruction
  t_regID r_temp = getNewRegister(program);
  // Generate a SW to the address specified by the label
  genSWGlobal(program, r_val, var->label, r_temp);
}


/** Generate instructions that load the address of an element of an array in a
 * register.
 * @param program The program where the array belongs.
 * @param array   The symbol object that refers to an array.
 * @param index   An expression that refers to a specific element of the array.
 * @returns The identifier of the register that (at runtime) will contain the
 *          address of the array element at position `index'. */
t_regID genLoadArrayAddress(
    t_program *program, t_symbol *array, t_expressionValue index)
{
  // Retrieve the label associated with the given identifier
  if (!isArray(array))
    fatalError("'%s' is a scalar", array->ID);
  t_label *label = array->label;

  // Generate a load of the base address using LA
  t_regID mova_register = getNewRegister(program);
  genLA(program, mova_register, label);

  /* We are making the following assumption:
   * the type can only be an INTEGER_TYPE */
  int sizeofElem = 4 / TARGET_PTR_GRANULARITY;

  if (index.type == CONSTANT) {
    if (index.immediate != 0) {
      genADDI(
          program, mova_register, mova_register, index.immediate * sizeofElem);
    }
  } else {
    t_regID idxReg;
    assert(index.type == REGISTER);

    idxReg = index.registerId;
    if (sizeofElem != 1) {
      idxReg = getNewRegister(program);
      genMULI(program, idxReg, index.registerId, sizeofElem);
    }

    genADD(program, mova_register, mova_register, idxReg);
  }

  /* return the identifier of the register that contains
   * the value of the array slot */
  return mova_register;
}


void genStoreArrayElement(t_program *program, t_symbol *array,
    t_expressionValue index, t_expressionValue data)
{
  t_regID address = genLoadArrayAddress(program, array, index);

  if (data.type == REGISTER) {
    /* load the value indirectly into `mova_register' */
    genSW(program, data.registerId, 0, address);
  } else {
    t_regID imm_register = getNewRegister(program);
    genLI(program, imm_register, data.immediate);

    /* load the value indirectly into `load_register' */
    genSW(program, imm_register, 0, address);
  }
}


t_regID genLoadArrayElement(
    t_program *program, t_symbol *array, t_expressionValue index)
{
  t_regID load_register, address;

  /* retrieve the address of the array slot */
  address = genLoadArrayAddress(program, array, index);

  /* get a new register */
  load_register = getNewRegister(program);

  /* load the value into `load_register' */
  genLW(program, load_register, 0, address);

  /* return the register ID that holds the required data */
  return load_register;
}
