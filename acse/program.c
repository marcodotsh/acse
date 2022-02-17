/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * 
 * engine.c
 * Formal Languages & Compilers Machine, 2007/2008
 * 
 */

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "program.h"
#include "errors.h"
#include "gencode.h"
#include "target_info.h"
#include "target_asm_print.h"
#include "options.h"
#include "variables.h"

/* global line number (defined in Acse.y) */
extern int line_num;
/* last line number inserted in an instruction as a comment */
int prev_line_num = -1;

/* Finalize the memory associated with an instruction */
static void finalizeInstructions(t_list *instructions);

/* Finalize the data segment */
static void finalizeDataSegment(t_list *dataDirectives);

/* create and initialize an instance of `t_axe_register' */
t_axe_register * initializeRegister(int ID)
{
   t_axe_register *result;

   /* create an instance of `t_axe_register' */
   result = (t_axe_register *)malloc(sizeof(t_axe_register));
   if (result == NULL)
      fatalError(AXE_OUT_OF_MEMORY);

   /* initialize the new label */
   result->ID = ID;
   result->mcRegWhitelist = NULL;

   /* return the label */
   return result;
}

/* create and initialize an instance of `t_axe_instruction' */
t_axe_instruction * initializeInstruction(int opcode)
{
   t_axe_instruction *result;

   /* create an instance of `t_axe_data' */
   result = (t_axe_instruction *)malloc(sizeof(t_axe_instruction));
   if (result == NULL)
      fatalError(AXE_OUT_OF_MEMORY);

   /* ininitialize the fields of `result' */
   result->opcode = opcode;
   result->reg_dest = NULL;
   result->reg_src1 = NULL;
   result->reg_src2 = NULL;
   result->immediate = 0;
   result->label = NULL;
   result->address = NULL;
   result->user_comment = NULL;

   /* return `result' */
   return result;
}

/* create and initialize an instance of `t_axe_data' */
t_axe_data * initializeData(int directiveType, int value, t_axe_label *label)
{
   t_axe_data *result;

   /* create an instance of `t_axe_data' */
   result = (t_axe_data *) malloc(sizeof(t_axe_data));
   if (result == NULL)
      fatalError(AXE_OUT_OF_MEMORY);

   /* initialize the new directive */
   result->directiveType = directiveType;
   result->value = value;
   result->labelID = label;

   /* return the new data */
   return result;
}

/* finalize an instruction info. */
void finalizeInstruction(t_axe_instruction *inst)
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
   if (inst->address != NULL) {
      free(inst->address);
   }
   if (inst->user_comment != NULL) {
      free(inst->user_comment);
   }

   free(inst);
}

/* finalize a data info. */
void finalizeData(t_axe_data *data)
{
   if (data != NULL)
      free(data);
}

t_axe_address * initializeAddress(int type, int address, t_axe_label *label)
{
   t_axe_address *result;

   result = (t_axe_address *)malloc(sizeof(t_axe_address));
   if (result == NULL)
      fatalError(AXE_OUT_OF_MEMORY);

   /* initialize the new instance of `t_axe_address' */
   result->type = type;
   result->addr = address;
   result->labelID = label;

   /* return the new address */
   return result;
}

void finalizeDataSegment(t_list *dataDirectives)
{
   t_list *current_element;
   t_axe_data *current_data;

   /* nothing to finalize */
   if (dataDirectives == NULL)
      return;

   current_element = dataDirectives;
   while(current_element != NULL)
   {
      /* retrieve the current instruction */
      current_data = (t_axe_data *) current_element->data;
      if (current_data != NULL)
         finalizeData(current_data);

      current_element = current_element->next;
   }

   /* free the list of instructions */
   freeList(dataDirectives);
}

void finalizeInstructions(t_list *instructions)
{
   t_list *current_element;
   t_axe_instruction *current_instr;

   /* nothing to finalize */
   if (instructions == NULL)
      return;

   current_element = instructions;
   while(current_element != NULL)
   {
      /* retrieve the current instruction */
      current_instr = (t_axe_instruction *) current_element->data;
      if (current_instr != NULL)
         finalizeInstruction(current_instr);

      current_element = current_element->next;
   }

   /* free the list of instructions */
   freeList(instructions);
}

/* initialize an instance of `t_program_infos' */
t_program_infos * allocProgramInfos(void)
{
   t_program_infos *result;
   t_axe_label *l_start;

   /* initialize the local variable `result' */
   result = (t_program_infos *)malloc(sizeof(t_program_infos));
   if (result == NULL)
      fatalError(AXE_OUT_OF_MEMORY);

   /* initialize the new instance of `result' */
   result->variables = NULL;
   result->instructions = NULL;
   result->instrInsPtrStack = addElement(NULL, NULL, -1);
   result->data = NULL;
   result->current_register = 1; /* we are excluding the register R0 */
   result->lmanager = initializeLabelManager();

   /* Create the start label */
   l_start = newLabelID(result->lmanager, 1);
   setLabelName(result->lmanager, l_start, "_start");
   assignLabelID(result->lmanager, l_start);
   
   /* postcondition: return an instance of `t_program_infos' */
   return result;
}

void printProgramInfos(t_program_infos *program, FILE *fout)
{
   t_list *cur_var, *cur_inst;

   fprintf(fout,"**************************\n");
   fprintf(fout,"          PROGRAM         \n");
   fprintf(fout,"**************************\n\n");

   fprintf(fout,"-----------\n");
   fprintf(fout," VARIABLES\n");
   fprintf(fout,"-----------\n");
   cur_var = program->variables;
   while (cur_var) {
      int reg;

      t_axe_variable *var = cur_var->data;
      fprintf(fout, "[%s]\n", var->ID);

      fprintf(fout, "   type = ");
      if (var->type == INTEGER_TYPE)
         fprintf(fout, "int");
      else
         fprintf(fout, "(invalid)");
      
      if (var->isArray) {
         fprintf(fout, ", array size = %d", var->arraySize);
      } else {
         fprintf(fout, ", scalar initial value = %d", var->init_val);
      }
      fprintf(fout, "\n");

      if (var->isArray) {
         char *labelName = getLabelName(var->label);
         fprintf(fout, "   label = %s (ID=%d)\n", labelName, var->label->labelID);
         free(labelName);
      }

      fprintf(fout, "   location = ");

      reg = getRegLocationOfScalar(program, var->ID);
      if (reg == REG_INVALID)
         fprintf(fout, "N/A");
      else
         fprintf(fout, "R%d", reg);
      fprintf(fout, "\n");

      cur_var = cur_var->next;
   }

   fprintf(fout,"\n--------------\n");
   fprintf(fout," INSTRUCTIONS\n");
   fprintf(fout,"--------------\n");
   cur_inst = program->instructions;
   while (cur_inst) {
      t_axe_instruction *instr = cur_inst->data;
      if (instr == NULL)
         fprintf(fout, "(null)");
      else
         printInstruction(instr, fout, 0);
      fprintf(fout, "\n");
      cur_inst = cur_inst->next;
   }

   fflush(fout);
}

/* add an instruction at the tail of the list `program->instructions'. */
void addInstruction(t_program_infos *program, t_axe_instruction *instr)
{
   t_list *ip;

   /* test the preconditions */
   assert(program != NULL);
   assert(instr != NULL);
   assert(program->lmanager != NULL);

   instr->label = getLastPendingLabel(program->lmanager);

   if (line_num >= 0 && line_num != prev_line_num) {
      instr->user_comment = calloc(20, sizeof(char));
      if (instr->user_comment) {
         snprintf(instr->user_comment, 20, "line %d", line_num);
      }
   }
   prev_line_num = line_num;

   /* update the list of instructions */
   ip = program->instrInsPtrStack->data;
   program->instructions = addAfter(program->instructions, ip, instr);
   if (ip)
      program->instrInsPtrStack->data = ip->next;
   else
      program->instrInsPtrStack->data = program->instructions;
}

t_axe_instruction *genInstruction(t_program_infos *program, int opcode,
      int r_dest, int r_src1, int r_src2, t_axe_label *label, int immediate)
{
   t_axe_instruction *instr;

   /* create an instance of `t_axe_instruction' */
   instr = initializeInstruction(opcode);

   /* initialize the instruction's registers */
   if (r_dest != REG_INVALID)
      instr->reg_dest = initializeRegister(r_dest);
   if (r_src1 != REG_INVALID)
      instr->reg_src1 = initializeRegister(r_src1);
   if (r_src2 != REG_INVALID)
      instr->reg_src2 = initializeRegister(r_src2);

   /* attach an address if needed */
   if (label)
      instr->address = initializeAddress(LABEL_TYPE, 0, label);

   /* initialize the immediate field */
   instr->immediate = immediate;

   /* add the newly created instruction to the current program */
   if (program != NULL)
      addInstruction(program, instr);

   /* return the load instruction */
   return instr;
}

void setMCRegisterWhitelist(t_axe_register *regObj, ...)
{
   t_list *res = NULL;
   va_list args;
   int cur;

   va_start(args, regObj);
   cur = va_arg(args, int);
   while (cur != REG_INVALID) {
      res = addElement(res, INT_TO_LIST_DATA(cur), -1);
      cur = va_arg(args, int);
   }
   va_end(args);

   if (regObj->mcRegWhitelist)
      freeList(regObj->mcRegWhitelist);
   regObj->mcRegWhitelist = res;
}

void removeInstructionLink(t_program_infos *program, t_list *instrLi)
{
   t_list *ipi;
   t_axe_instruction *instrToRemove = (t_axe_instruction *)instrLi->data;

   /* move the label and/or the comment to the next instruction */
   if (instrToRemove->label || instrToRemove->user_comment) {
      /* find the next instruction, if it exists */
      t_list *nextPos = instrLi->next;
      t_axe_instruction *nextInst = NULL;
      if (nextPos)
         nextInst = nextPos->data;
         
      /* move the label */
      if (instrToRemove->label) {
         /* generate a nop if there was no next instruction or if the next instruction
          * is already labeled */
         if (!nextInst || (nextInst->label)) {
            pushInstrInsertionPoint(program, instrLi);
            nextInst = genNOPInstruction(program);
            popInstrInsertionPoint(program);
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

   /* fixup the insertion pointer stack */
   ipi = program->instrInsPtrStack;
   while (ipi) {
      if (ipi->data && ipi->data == instrLi)
         ipi->data = instrLi->prev;
      ipi = ipi->next;
   }

   /* remove the instruction */
   program->instructions = removeElementLink(program->instructions, instrLi);
   finalizeInstruction(instrToRemove);
}

void pushInstrInsertionPoint(t_program_infos *p, t_list *ip)
{
   prev_line_num = -1;
   p->instrInsPtrStack = addFirst(p->instrInsPtrStack, ip);
}

t_list *popInstrInsertionPoint(t_program_infos *p)
{
   t_list *ip;
   t_axe_label *label;

   prev_line_num = -1;
   ip = p->instrInsPtrStack->data;

   /* affix the currently pending label, if needed */
   label = getLastPendingLabel(p->lmanager);
   if (label) {
      t_list *labelPos = ip ? ip->next : NULL;
      t_axe_instruction *instrToLabel;
      if (!labelPos)
         instrToLabel = genNOPInstruction(p);
      else
         instrToLabel = labelPos->data;
      instrToLabel->label = label;
   }

   p->instrInsPtrStack = removeFirst(p->instrInsPtrStack);
   return ip;
}

/* reserve a new label identifier for future uses */
t_axe_label *newNamedLabel(t_program_infos *program, const char *name)
{
   t_axe_label *label;

   /* test the preconditions */
   assert(program != NULL);
   assert(program->lmanager != NULL);

   label = newLabelID(program->lmanager, 0);
   if (name)
      setLabelName(program->lmanager, label, name);
   return label;
}

t_axe_label * newLabel(t_program_infos *program)
{
   return newNamedLabel(program, NULL);
}

/* assign a new label identifier to the next instruction */
t_axe_label * assignLabel(t_program_infos *program, t_axe_label *label)
{
   t_list *li;

   /* test the preconditions */
   assert(program != NULL);
   assert(program->lmanager != NULL);
   
   for (li = program->instructions; li != NULL; li = li->next) {
      t_axe_instruction *instr = li->data;
      if (instr->label && compareLabels(instr->label, label))
         fatalError(AXE_LABEL_ALREADY_ASSIGNED);
   }

   /* fix the label */
   return assignLabelID(program->lmanager, label);
}

/* reserve a new label identifier */
t_axe_label *assignNewNamedLabel(t_program_infos *program, const char *name)
{
   t_axe_label *reserved_label;

   /* reserve a new label */
   reserved_label = newNamedLabel(program, name);

   /* fix the label */
   return assignLabel(program, reserved_label);
}

t_axe_label * assignNewLabel(t_program_infos *program)
{
   return assignNewNamedLabel(program, NULL);
}

int getNewRegister(t_program_infos *program)
{
   int result;
   
   /* test the preconditions */
   assert(program != NULL);

   result = program->current_register;
   program->current_register++;
   
   /* return the current label identifier */
   return result;
}

void finalizeProgramInfos(t_program_infos *program)
{
   if (program == NULL)
      return;
   if (program->variables != NULL)
      finalizeVariables(program->variables);
   if (program->instructions != NULL)
      finalizeInstructions(program->instructions);
   if (program->data != NULL)
      finalizeDataSegment(program->data);
   if (program->lmanager != NULL)
      finalizeLabelManager(program->lmanager);

   free(program);
}
