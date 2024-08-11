/// @file program.c

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "errors.h"
#include "program.h"
#include "scanner.h"
#include "codegen.h"
#include "target_info.h"
#include "target_asm_print.h"


static t_label *newLabel(unsigned int value)
{
  t_label *result = (t_label *)malloc(sizeof(t_label));
  if (result == NULL)
    fatalError("out of memory");
  result->labelID = value;
  result->name = NULL;
  result->global = 0;
  result->isAlias = 0;
  return result;
}

void deleteLabel(t_label *lab)
{
  free(lab->name);
  free(lab);
}

void deleteLabels(t_listNode *labels)
{
  t_listNode *curNode = labels;
  while (curNode != NULL) {
    t_label *curLabel = (t_label *)curNode->data;
    deleteLabel(curLabel);
    curNode = curNode->next;
  }
  deleteList(labels);
}


t_instrArg *newInstrArg(t_regID ID)
{
  t_instrArg *result = (t_instrArg *)malloc(sizeof(t_instrArg));
  if (result == NULL)
    fatalError("out of memory");
  result->ID = ID;
  result->mcRegWhitelist = NULL;
  return result;
}

/* create and initialize an instance of `t_instruction' */
t_instruction *newInstruction(int opcode)
{
  t_instruction *result = (t_instruction *)malloc(sizeof(t_instruction));
  if (result == NULL)
    fatalError("out of memory");
  result->opcode = opcode;
  result->rDest = NULL;
  result->rSrc1 = NULL;
  result->rSrc2 = NULL;
  result->immediate = 0;
  result->label = NULL;
  result->addressParam = NULL;
  result->comment = NULL;
  return result;
}

/* finalize an instruction info. */
void deleteInstruction(t_instruction *inst)
{
  if (inst == NULL)
    return;
  if (inst->rDest != NULL) {
    deleteList(inst->rDest->mcRegWhitelist);
    free(inst->rDest);
  }
  if (inst->rSrc1 != NULL) {
    deleteList(inst->rSrc1->mcRegWhitelist);
    free(inst->rSrc1);
  }
  if (inst->rSrc2 != NULL) {
    deleteList(inst->rSrc2->mcRegWhitelist);
    free(inst->rSrc2);
  }
  free(inst->comment);
  free(inst);
}

void deleteInstructions(t_listNode *instructions)
{
  if (instructions == NULL)
    return;

  t_listNode *curNode = instructions;
  while (curNode != NULL) {
    t_instruction *curInstr = (t_instruction *)curNode->data;
    deleteInstruction(curInstr);
    curNode = curNode->next;
  }

  deleteList(instructions);
}


t_symbol *newSymbol(char *ID, t_symbolType type, int arraySize)
{
  t_symbol *result = (t_symbol *)malloc(sizeof(t_symbol));
  if (result == NULL)
    fatalError("out of memory");
  result->type = type;
  result->arraySize = arraySize;
  result->ID = ID;
  result->label = NULL;
  return result;
}

static void deleteSymbol(t_symbol *s)
{
  free(s->ID);
  free(s);
}

static void deleteSymbols(t_listNode *variables)
{
  if (variables == NULL)
    return;

  t_listNode *curNode = variables;
  while (curNode != NULL) {
    t_symbol *curSymbol = (t_symbol *)curNode->data;
    deleteSymbol(curSymbol);
    curNode = curNode->next;
  }

  deleteList(variables);
}


t_program *newProgram(void)
{
  t_program *result = (t_program *)malloc(sizeof(t_program));
  if (result == NULL)
    fatalError("out of memory");
  result->symbols = NULL;
  result->instructions = NULL;
  result->firstUnusedReg = 1; // we are excluding register R0
  result->labels = NULL;
  result->firstUnusedLblID = 0;
  result->pendingLabel = NULL;

  // Create the start label
  t_label *lStart = createLabel(result);
  lStart->global = 1;
  setLabelName(result, lStart, "_start");
  assignLabel(result, lStart);

  return result;
}

void deleteProgram(t_program *program)
{
  if (program == NULL)
    return;
  deleteSymbols(program->symbols);
  deleteInstructions(program->instructions);
  deleteLabels(program->labels);
  free(program);
}


t_label *createLabel(t_program *program)
{
  t_label *result = newLabel(program->firstUnusedLblID);
  if (result == NULL)
    fatalError("out of memory");
  program->firstUnusedLblID++;
  program->labels = listInsert(program->labels, result, -1);
  return result;
}

/* Set a name to a label without resolving duplicates */
void setRawLabelName(t_program *program, t_label *label, const char *finalName)
{
  t_listNode *i;

  // check the entire list of labels because there might be two
  // label objects with the same ID and they need to be kept in sync
  for (i = program->labels; i != NULL; i = i->next) {
    t_label *thisLab = i->data;

    if (thisLab->labelID == label->labelID) {
      // found! remove old name
      free(thisLab->name);
      // change to new name
      if (finalName)
        thisLab->name = strdup(finalName);
      else
        thisLab->name = NULL;
    }
  }
}

void setLabelName(t_program *program, t_label *label, const char *name)
{
  // remove all non a-zA-Z0-9_ characters
  char *sanitizedName = calloc(strlen(name) + 1, sizeof(char));
  if (!sanitizedName)
    fatalError("out of memory");
  const char *srcp = name;
  for (char *dstp = sanitizedName; *srcp; srcp++) {
    if (*srcp == '_' || isalnum(*srcp))
      *dstp++ = *srcp;
  }

  // append a sequential number to disambiguate labels with the same name
  size_t allocatedSpace = strlen(sanitizedName) + 24;
  char *finalName = calloc(allocatedSpace, sizeof(char));
  if (!finalName)
    fatalError("out of memory");
  snprintf(finalName, allocatedSpace, "%s", sanitizedName);
  int serial = -1;
  bool ok;
  do {
    t_listNode *i;
    ok = true;
    for (i = program->labels; i != NULL; i = i->next) {
      t_label *thisLab = i->data;
      char *thisLabName;
      int difference;

      if (thisLab->labelID == label->labelID)
        continue;

      thisLabName = getLabelName(thisLab);
      difference = strcmp(finalName, thisLabName);
      free(thisLabName);

      if (difference == 0) {
        ok = false;
        snprintf(finalName, allocatedSpace, "%s_%d", sanitizedName, ++serial);
        break;
      }
    }
  } while (!ok);

  free(sanitizedName);
  setRawLabelName(program, label, finalName);
  free(finalName);
}

void assignLabel(t_program *program, t_label *label)
{
  // check if this label has already been assigned
  for (t_listNode *li = program->instructions; li != NULL; li = li->next) {
    t_instruction *instr = li->data;
    if (instr->label && instr->label->labelID == label->labelID)
      fatalError("bug: label already assigned");
  }

  // test if the next instruction already has a label
  if (program->pendingLabel != NULL) {
    // It does: transform the label being assigned into an alias of the
    // label of the next instruction's label
    // All label aliases have the same ID and name.

    // Decide the name of the alias. If only one label has a name, that name
    // wins. Otherwise the name of the label with the lowest ID wins
    char *name = program->pendingLabel->name;
    if (!name ||
        (label->labelID && label->labelID < program->pendingLabel->labelID))
      name = label->name;
    // copy the name
    if (name)
      name = strdup(name);

    // Change ID and name
    label->labelID = (program->pendingLabel)->labelID;
    setRawLabelName(program, label, name);

    // Promote both labels to global if at least one is global
    if (label->global)
      program->pendingLabel->global = 1;
    else if (program->pendingLabel->global)
      label->global = 1;

    // mark the label as an alias
    label->isAlias = 1;

    free(name);
  } else {
    program->pendingLabel = label;
  }
}

char *getLabelName(t_label *label)
{
  char *buf;

  if (label->name) {
    buf = strdup(label->name);
  } else {
    buf = calloc(24, sizeof(char));
    snprintf(buf, 24, "l_%d", label->labelID);
  }

  return buf;
}


void addInstruction(t_program *program, t_instruction *instr)
{
  static t_fileLocation lastFileLoc = {NULL, -1};

  // assign the currently pending label if there is one
  instr->label = program->pendingLabel;
  program->pendingLabel = NULL;

  // add a comment with the line number
  if (curFileLoc.row >= 0 &&
      (curFileLoc.file != lastFileLoc.file ||
          curFileLoc.row != lastFileLoc.row)) {
    size_t fileNameLen = strlen(curFileLoc.file);
    size_t strBufSz = fileNameLen + 10 + 1;
    instr->comment = calloc(strBufSz, sizeof(char));
    if (instr->comment) {
      snprintf(
          instr->comment, strBufSz, "%s:%d", curFileLoc.file, curFileLoc.row+1);
    }
  }
  lastFileLoc = curFileLoc;

  // update the list of instructions
  program->instructions = listInsert(program->instructions, instr, -1);
}

t_instruction *genInstruction(t_program *program, int opcode, t_regID rd,
    t_regID rs1, t_regID rs2, t_label *label, int immediate)
{
  t_instruction *instr = newInstruction(opcode);
  if (rd != REG_INVALID)
    instr->rDest = newInstrArg(rd);
  if (rs1 != REG_INVALID)
    instr->rSrc1 = newInstrArg(rs1);
  if (rs2 != REG_INVALID)
    instr->rSrc2 = newInstrArg(rs2);
  if (label)
    instr->addressParam = label;
  instr->immediate = immediate;

  // Add the newly created instruction to the current program. This function
  // may be called with program set to NULL to just create the instruction
  // without inserting it, so we need to handle this case as well.
  if (program != NULL)
    addInstruction(program, instr);

  return instr;
}

void removeInstructionAt(t_program *program, t_listNode *instrLi)
{
  t_instruction *instrToRemove = (t_instruction *)instrLi->data;

  // move the label and/or the comment to the next instruction
  if (instrToRemove->label || instrToRemove->comment) {
    // find the next instruction, if it exists
    t_listNode *nextPos = instrLi->next;
    t_instruction *nextInst = NULL;
    if (nextPos)
      nextInst = nextPos->data;

    // move the label
    if (instrToRemove->label) {
      // generate a nop if there was no next instruction or if the next
      // instruction is already labeled
      if (!nextInst || (nextInst->label)) {
        nextInst = genNOP(NULL);
        program->instructions =
            listInsertAfter(program->instructions, instrLi, nextInst);
      }
      nextInst->label = instrToRemove->label;
      instrToRemove->label = NULL;
    }

    // move the comment, if possible; otherwise it will be discarded
    if (nextInst && instrToRemove->comment && !nextInst->comment) {
      nextInst->comment = instrToRemove->comment;
      instrToRemove->comment = NULL;
    }
  }

  // remove the instruction
  program->instructions = listRemoveNode(program->instructions, instrLi);
  deleteInstruction(instrToRemove);
}

t_regID getNewRegister(t_program *program)
{
  t_regID result = program->firstUnusedReg;
  program->firstUnusedReg++;
  return result;
}


t_symbol *createSymbol(
    t_program *program, char *ID, t_symbolType type, int arraySize)
{
  // Check validity of type
  if (type != TYPE_INT && type != TYPE_INT_ARRAY)
    fatalError("bug: invalid type");
  // Check array size validity
  if (type == TYPE_INT_ARRAY && arraySize <= 0) {
    emitError(curFileLoc, "invalid size %d for array %s", arraySize, ID);
    return NULL;
  }

  // Check if another symbol already exists with the same ID
  t_symbol *existingSym = getSymbol(program, ID);
  if (existingSym != NULL) {
    emitError(curFileLoc, "variable '%s' already declared", ID);
    return NULL;
  }

  // Allocate and initialize a new symbol object
  t_symbol *res = newSymbol(ID, type, arraySize);

  // Reserve a new label for the variable
  res->label = createLabel(program);

  // Set the name of the label
  char *lblName = calloc(strlen(ID) + 8, sizeof(char));
  if (!lblName)
    fatalError("out of memory");
  sprintf(lblName, "l_%s", ID);
  setLabelName(program, res->label, lblName);
  free(lblName);

  // Now we can add the new variable to the program
  program->symbols = listInsert(program->symbols, res, -1);
  return res;
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
  // search inside the list of variables
  t_listNode *elementFound = listFindWithCallback(
      program->symbols, ID, compareVariableWithIDString);

  // if the element is found return it to the caller. Otherwise return NULL.
  if (elementFound != NULL)
    return (t_symbol *)elementFound->data;

  return NULL;
}


void genProgramEpilog(t_program *program)
{
  if (program->pendingLabel != NULL) {
    genExit0Syscall(program);
    return;
  }

  if (program->instructions != NULL) {
    t_listNode *lastNode = listGetLastNode(program->instructions);
    t_instruction *lastInstr = (t_instruction *)lastNode->data;
    if (lastInstr->opcode == OPC_CALL_EXIT_0)
      return;
  }

  genExit0Syscall(program);
  return;
}

void dumpProgram(t_program *program, FILE *fout)
{
  fprintf(fout, "**************************\n");
  fprintf(fout, "          PROGRAM         \n");
  fprintf(fout, "**************************\n\n");

  fprintf(fout, "-----------\n");
  fprintf(fout, " VARIABLES\n");
  fprintf(fout, "-----------\n");
  t_listNode *curVarNode = program->symbols;
  while (curVarNode) {
    t_symbol *var = curVarNode->data;
    fprintf(fout, "[%s]\n", var->ID);

    if (var->type == TYPE_INT) {
      fprintf(fout, "   type = int\n");
    } else if (var->type == TYPE_INT_ARRAY) {
      fprintf(fout, "   type = int[%d]\n", var->arraySize);
    } else {
      fprintf(fout, "   type = invalid\n");
    }
    char *labelName = getLabelName(var->label);
    fprintf(fout, "   label = %s (ID=%d)\n", labelName, var->label->labelID);
    free(labelName);

    curVarNode = curVarNode->next;
  }

  fprintf(fout, "\n--------------\n");
  fprintf(fout, " INSTRUCTIONS\n");
  fprintf(fout, "--------------\n");
  t_listNode *curInstNode = program->instructions;
  while (curInstNode) {
    t_instruction *instr = curInstNode->data;
    if (instr == NULL)
      fprintf(fout, "(null)");
    else
      printInstruction(instr, fout, false);
    fprintf(fout, "\n");
    curInstNode = curInstNode->next;
  }

  fflush(fout);
}
