/// @file engine.c

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include "program.h"
#include "errors.h"
#include "gencode.h"
#include "target_info.h"
#include "target_asm_print.h"
#include "options.h"
#include "symbols.h"

/* global line number (defined in Acse.y) */
extern int line_num;
/* last line number inserted in an instruction as a comment */
int prev_line_num = -1;


static t_label *newLabel(int value)
{
  t_label *result;

  /* create an instance of t_label */
  result = (t_label *)malloc(sizeof(t_label));
  if (result == NULL)
    fatalError(ERROR_OUT_OF_MEMORY);

  /* initialize the internal value of `result' */
  result->labelID = value;
  result->name = NULL;
  result->global = 0;
  result->isAlias = 0;

  /* return the just initialized new instance of `t_label' */
  return result;
}

void deleteLabel(t_label *lab)
{
  if (lab->name)
    free(lab->name);
  free(lab);
}

/* create and initialize an instance of `t_instrArg' */
t_instrArg *newInstrArg(int ID)
{
  t_instrArg *result;

  /* create an instance of `t_instrArg' */
  result = (t_instrArg *)malloc(sizeof(t_instrArg));
  if (result == NULL)
    fatalError(ERROR_OUT_OF_MEMORY);

  /* initialize the new label */
  result->ID = ID;
  result->mcRegWhitelist = NULL;

  /* return the label */
  return result;
}

/* create and initialize an instance of `t_instruction' */
t_instruction *newInstruction(int opcode)
{
  t_instruction *result;

  /* create an instance of `t_global' */
  result = (t_instruction *)malloc(sizeof(t_instruction));
  if (result == NULL)
    fatalError(ERROR_OUT_OF_MEMORY);

  /* ininitialize the fields of `result' */
  result->opcode = opcode;
  result->reg_dest = NULL;
  result->reg_src1 = NULL;
  result->reg_src2 = NULL;
  result->immediate = 0;
  result->label = NULL;
  result->addressParam = NULL;
  result->user_comment = NULL;

  /* return `result' */
  return result;
}

/* create and initialize an instance of `t_global' */
t_global *newGlobal(int directiveType, int value, t_label *label)
{
  t_global *result;

  /* create an instance of `t_global' */
  result = (t_global *)malloc(sizeof(t_global));
  if (result == NULL)
    fatalError(ERROR_OUT_OF_MEMORY);

  /* initialize the new directive */
  result->directiveType = directiveType;
  result->value = value;
  result->labelID = label;

  /* return the new data */
  return result;
}

/* finalize an instruction info. */
void deleteInstruction(t_instruction *inst)
{
  /* preconditions */
  if (inst == NULL)
    return;

  /* free memory */
  if (inst->reg_dest != NULL) {
    freeList(inst->reg_dest->mcRegWhitelist);
    free(inst->reg_dest);
  }
  if (inst->reg_src1 != NULL) {
    freeList(inst->reg_src1->mcRegWhitelist);
    free(inst->reg_src1);
  }
  if (inst->reg_src2 != NULL) {
    freeList(inst->reg_src2->mcRegWhitelist);
    free(inst->reg_src2);
  }
  if (inst->user_comment != NULL) {
    free(inst->user_comment);
  }

  free(inst);
}

/* finalize a data info. */
void deleteGlobal(t_global *data)
{
  if (data != NULL)
    free(data);
}

void deleteGlobals(t_listNode *dataDirectives)
{
  t_listNode *current_element;
  t_global *current_data;

  /* nothing to finalize */
  if (dataDirectives == NULL)
    return;

  current_element = dataDirectives;
  while (current_element != NULL) {
    /* retrieve the current instruction */
    current_data = (t_global *)current_element->data;
    if (current_data != NULL)
      deleteGlobal(current_data);

    current_element = current_element->next;
  }

  /* free the list of instructions */
  freeList(dataDirectives);
}

void deleteInstructions(t_listNode *instructions)
{
  t_listNode *current_element;
  t_instruction *current_instr;

  /* nothing to finalize */
  if (instructions == NULL)
    return;

  current_element = instructions;
  while (current_element != NULL) {
    /* retrieve the current instruction */
    current_instr = (t_instruction *)current_element->data;
    if (current_instr != NULL)
      deleteInstruction(current_instr);

    current_element = current_element->next;
  }

  /* free the list of instructions */
  freeList(instructions);
}

void deleteLabels(t_listNode *labels)
{
  t_listNode *current_element;
  t_label *current_label;

  current_element = labels;

  while (current_element != NULL) {
    /* retrieve the current label */
    current_label = (t_label *)current_element->data;
    assert(current_label != NULL);

    /* free the memory associated with the current label */
    deleteLabel(current_label);

    /* fetch the next label */
    current_element = current_element->next;
  }

  freeList(labels);
}

/* initialize an instance of `t_program' */
t_program *newProgram(void)
{
  t_program *result;
  t_label *l_start;

  /* initialize the local variable `result' */
  result = (t_program *)malloc(sizeof(t_program));
  if (result == NULL)
    fatalError(ERROR_OUT_OF_MEMORY);

  /* initialize the new instance of `result' */
  result->variables = NULL;
  result->instructions = NULL;
  result->data = NULL;
  result->current_register = 1; /* we are excluding the register R0 */
  result->labels = NULL;
  result->current_label_ID = 0;
  result->label_to_assign = NULL;

  /* Create the start label */
  l_start = createLabel(result);
  l_start->global = 1;
  setLabelName(result, l_start, "_start");
  assignLabel(result, l_start);

  /* postcondition: return an instance of `t_program' */
  return result;
}

void deleteProgram(t_program *program)
{
  if (program == NULL)
    return;
  if (program->variables != NULL)
    deleteSymbols(program->variables);
  if (program->instructions != NULL)
    deleteInstructions(program->instructions);
  if (program->data != NULL)
    deleteGlobals(program->data);
  if (program->labels != NULL)
    deleteLabels(program->labels);

  free(program);
}

int compareLabels(t_label *labelA, t_label *labelB)
{
  if ((labelA == NULL) || (labelB == NULL))
    return 0;

  if (labelA->labelID == labelB->labelID)
    return 1;
  return 0;
}

t_label *createLabel(t_program *program)
{
  t_label *result;

  /* preconditions: program must be different from NULL */
  assert(program != NULL);

  /* initialize a new label */
  result = newLabel(program->current_label_ID);
  if (result == NULL)
    fatalError(ERROR_OUT_OF_MEMORY);

  /* update the value of `current_label_ID' */
  program->current_label_ID++;

  /* add the new label to the list of labels */
  program->labels = addElement(program->labels, result, -1);

  /* return the new label */
  return result;
}


/* Set a name to a label without resolving duplicates */
void setRawLabelName(t_program *program, t_label *label, const char *finalName)
{
  t_listNode *i;

  /* check the entire list of labels because there might be two
   * label objects with the same ID and they need to be kept in sync */
  for (i = program->labels; i != NULL; i = i->next) {
    t_label *thisLab = i->data;

    if (thisLab->labelID == label->labelID) {
      /* found! remove old name */
      free(thisLab->name);
      /* change to new name */
      if (finalName)
        thisLab->name = strdup(finalName);
      else
        thisLab->name = NULL;
    }
  }
}

void setLabelName(t_program *program, t_label *label, const char *name)
{
  int serial = -1, allocatedSpace;
  bool ok;
  char *sanitizedName, *finalName, *dstp;
  const char *srcp;

  /* remove all non a-zA-Z0-9_ characters */
  sanitizedName = calloc(strlen(name) + 1, sizeof(char));
  srcp = name;
  for (dstp = sanitizedName; *srcp; srcp++) {
    if (*srcp == '_' || isalnum(*srcp))
      *dstp++ = *srcp;
  }

  /* append a sequential number to disambiguate labels with the same name */
  allocatedSpace = strlen(sanitizedName) + 24;
  finalName = calloc(allocatedSpace, sizeof(char));
  snprintf(finalName, allocatedSpace, "%s", sanitizedName);
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

/* assign a new label identifier to the next instruction */
t_label *assignLabel(t_program *program, t_label *label)
{
  t_listNode *li;

  /* test the preconditions */
  assert(program != NULL);
  assert(label != NULL);
  assert(label->labelID < program->current_label_ID);

  /* check if this label has already been assigned */
  for (li = program->instructions; li != NULL; li = li->next) {
    t_instruction *instr = li->data;
    if (instr->label && compareLabels(instr->label, label))
      fatalError(ERROR_LABEL_ALREADY_ASSIGNED);
  }

  /* test if the next instruction already has a label */
  if (program->label_to_assign != NULL) {
    /* It does: transform the label being assigned into an alias of the
     * label of the next instruction's label
     * All label aliases have the same ID and name. */

    /* Decide the name of the alias. If only one label has a name, that name
     * wins. Otherwise the name of the label with the lowest ID wins */
    char *name = program->label_to_assign->name;
    if (!name ||
        (label->labelID && label->labelID < program->label_to_assign->labelID))
      name = label->name;
    /* copy the name */
    if (name)
      name = strdup(name);

    /* Change ID and name */
    label->labelID = (program->label_to_assign)->labelID;
    setRawLabelName(program, label, name);

    /* Promote both labels to global if at least one is global */
    if (label->global)
      program->label_to_assign->global = 1;
    else if (program->label_to_assign->global)
      label->global = 1;

    /* mark the label as an alias */
    label->isAlias = 1;

    free(name);
  } else
    program->label_to_assign = label;

  /* all went good */
  return label;
}

/* reserve a new label identifier */
t_label *assignNewLabel(t_program *program)
{
  t_label *reserved_label;

  /* reserve a new label */
  reserved_label = createLabel(program);

  /* fix the label */
  return assignLabel(program, reserved_label);
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

/* add an instruction at the tail of the list `program->instructions'. */
void addInstruction(t_program *program, t_instruction *instr)
{
  t_listNode *ip;

  /* test the preconditions */
  assert(program != NULL);
  assert(instr != NULL);

  /* assign the currently pending label if there is one */
  instr->label = program->label_to_assign;
  program->label_to_assign = NULL;

  /* add a comment with the line number */
  if (line_num >= 0 && line_num != prev_line_num) {
    instr->user_comment = calloc(20, sizeof(char));
    if (instr->user_comment) {
      snprintf(instr->user_comment, 20, "line %d", line_num);
    }
  }
  prev_line_num = line_num;

  /* update the list of instructions */
  program->instructions = addElement(program->instructions, instr, -1);
}

t_instruction *genInstruction(t_program *program, int opcode, int r_dest,
    int r_src1, int r_src2, t_label *label, int immediate)
{
  t_instruction *instr;

  /* create an instance of `t_instruction' */
  instr = newInstruction(opcode);

  /* initialize the instruction's registers */
  if (r_dest != REG_INVALID)
    instr->reg_dest = newInstrArg(r_dest);
  if (r_src1 != REG_INVALID)
    instr->reg_src1 = newInstrArg(r_src1);
  if (r_src2 != REG_INVALID)
    instr->reg_src2 = newInstrArg(r_src2);

  /* attach an address if needed */
  if (label)
    instr->addressParam = label;

  /* initialize the immediate field */
  instr->immediate = immediate;

  /* add the newly created instruction to the current program */
  if (program != NULL)
    addInstruction(program, instr);

  /* return the load instruction */
  return instr;
}

void removeInstructionAt(t_program *program, t_listNode *instrLi)
{
  t_listNode *ipi;
  t_instruction *instrToRemove = (t_instruction *)instrLi->data;

  /* move the label and/or the comment to the next instruction */
  if (instrToRemove->label || instrToRemove->user_comment) {
    /* find the next instruction, if it exists */
    t_listNode *nextPos = instrLi->next;
    t_instruction *nextInst = NULL;
    if (nextPos)
      nextInst = nextPos->data;

    /* move the label */
    if (instrToRemove->label) {
      /* generate a nop if there was no next instruction or if the next
       * instruction is already labeled */
      if (!nextInst || (nextInst->label)) {
        nextInst = genNOPInstruction(NULL);
        program->instructions =
            addAfter(program->instructions, instrLi, nextInst);
      }
      nextInst->label = instrToRemove->label;
      instrToRemove->label = NULL;
    }

    /* move the comment, if possible; otherwise it will be discarded */
    if (nextInst && instrToRemove->user_comment && !nextInst->user_comment) {
      nextInst->user_comment = instrToRemove->user_comment;
      instrToRemove->user_comment = NULL;
    }
  }

  /* remove the instruction */
  program->instructions = removeElement(program->instructions, instrLi);
  deleteInstruction(instrToRemove);
}

int getNewRegister(t_program *program)
{
  int result;

  /* test the preconditions */
  assert(program != NULL);

  result = program->current_register;
  program->current_register++;

  /* return the current label identifier */
  return result;
}

t_global *genDataDirective(
    t_program *program, int type, int value, t_label *label)
{
  t_global *res;

  assert(program);

  res = newGlobal(type, value, label);
  program->data = addElement(program->data, res, -1);
  return res;
}

void setProgramEnd(t_program *program)
{
  assert(program != NULL);

  if (program->label_to_assign != NULL) {
    genExit0Syscall(program);
    return;
  }

  if (program->instructions != NULL) {
    t_instruction *last_instr;
    t_listNode *last_element;

    /* get the last element of the list */
    last_element = getLastElement(program->instructions);
    assert(last_element != NULL);

    /* retrieve the last instruction */
    last_instr = (t_instruction *)last_element->data;
    assert(last_instr != NULL);

    if (last_instr->opcode == OPC_CALL_EXIT_0)
      return;
  }

  genExit0Syscall(program);
  return;
}

void dumpProgram(t_program *program, FILE *fout)
{
  t_listNode *cur_var, *cur_inst;

  fprintf(fout, "**************************\n");
  fprintf(fout, "          PROGRAM         \n");
  fprintf(fout, "**************************\n\n");

  fprintf(fout, "-----------\n");
  fprintf(fout, " VARIABLES\n");
  fprintf(fout, "-----------\n");
  cur_var = program->variables;
  while (cur_var) {
    int reg;

    t_symbol *var = cur_var->data;
    fprintf(fout, "[%s]\n", var->ID);

    if (var->type == TYPE_INT) {
      fprintf(fout, "   type = int\n");
      fprintf(fout, "   temporary register = t%d\n", var->reg_location);
    } else if (var->type == TYPE_INT_ARRAY) {
      fprintf(fout, "   type = int[%d]\n", var->arraySize);
      char *labelName = getLabelName(var->label);
      fprintf(fout, "   label = %s (ID=%d)\n", labelName, var->label->labelID);
      free(labelName);
    } else {
      fprintf(fout, "   type = invalid\n");
    }

    cur_var = cur_var->next;
  }

  fprintf(fout, "\n--------------\n");
  fprintf(fout, " INSTRUCTIONS\n");
  fprintf(fout, "--------------\n");
  cur_inst = program->instructions;
  while (cur_inst) {
    t_instruction *instr = cur_inst->data;
    if (instr == NULL)
      fprintf(fout, "(null)");
    else
      printInstruction(instr, fout, 0);
    fprintf(fout, "\n");
    cur_inst = cur_inst->next;
  }

  fflush(fout);
}
