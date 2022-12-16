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
#include <ctype.h>
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


t_axe_label * initializeLabel(int value)
{
   t_axe_label *result;

   /* create an instance of t_axe_label */
   result = (t_axe_label *)malloc(sizeof(t_axe_label));
   if (result == NULL)
      fatalError(AXE_OUT_OF_MEMORY);

   /* initialize the internal value of `result' */
   result->labelID = value;
   result->name = NULL;
   result->global = 0;
   result->isAlias = 0;

   /* return the just initialized new instance of `t_axe_label' */
   return result;
}

void finalizeLabel(t_axe_label *lab)
{
   if (lab->name)
      free(lab->name);
   free(lab);
}

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
   result->addressParam = NULL;
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

void finalizeLabels(t_list *labels)
{
   t_list *current_element;
   t_axe_label *current_label;

   current_element = labels;

   while (current_element != NULL)
   {
      /* retrieve the current label */
      current_label = (t_axe_label *) current_element->data;
      assert(current_label != NULL);

      /* free the memory associated with the current label */
      finalizeLabel(current_label);

      /* fetch the next label */
      current_element = current_element->next;
   }

   freeList(labels);
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
   result->data = NULL;
   result->current_register = 1; /* we are excluding the register R0 */
   result->labels = NULL;
   result->current_label_ID = 0;
   result->label_to_assign = NULL;

   /* Create the start label */
   l_start = newLabel(result);
   l_start->global = 1;
   setLabelName(result, l_start, "_start");
   assignLabel(result, l_start);
   
   /* postcondition: return an instance of `t_program_infos' */
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
   if (program->labels != NULL)
      finalizeLabels(program->labels);

   free(program);
}

int compareLabels(t_axe_label *labelA, t_axe_label *labelB)
{
   if ( (labelA == NULL) || (labelB == NULL) )
      return 0;

   if (labelA->labelID == labelB->labelID)
      return 1;
   return 0;
}

t_axe_label * newLabel(t_program_infos *program)
{
   t_axe_label *result;

   /* preconditions: program must be different from NULL */
   assert(program != NULL);
   
   /* initialize a new label */
   result = initializeLabel(program->current_label_ID);
   if (result == NULL)
      fatalError(AXE_OUT_OF_MEMORY);

   /* update the value of `current_label_ID' */
   program->current_label_ID++;

   /* add the new label to the list of labels */
   program->labels = addElement(program->labels, result, -1);

   /* return the new label */
   return result;
}


/* Set a name to a label without resolving duplicates */
void setRawLabelName(t_program_infos *program, t_axe_label *label,
      const char *finalName)
{
   t_list *i;

   /* check the entire list of labels because there might be two
    * label objects with the same ID and they need to be kept in sync */
   for (i = program->labels; i != NULL; i = i->next) {
      t_axe_label *thisLab = i->data;

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

void setLabelName(t_program_infos *program, t_axe_label *label,
      const char *name)
{
   int serial = -1, ok, allocatedSpace;
   char *sanitizedName, *finalName, *dstp;
   const char *srcp;

   /* remove all non a-zA-Z0-9_ characters */
   sanitizedName = calloc(strlen(name)+1, sizeof(char));
   srcp = name;
   for (dstp = sanitizedName; *srcp; srcp++) {
      if (*srcp == '_' || isalnum(*srcp))
         *dstp++ = *srcp;
   }

   /* append a sequential number to disambiguate labels with the same name */
   allocatedSpace = strlen(sanitizedName)+24;
   finalName = calloc(allocatedSpace, sizeof(char));
   snprintf(finalName, allocatedSpace, "%s", sanitizedName);
   do {
      t_list *i;
      ok = 1;
      for (i = program->labels; i != NULL; i = i->next) {
         t_axe_label *thisLab = i->data;
         char *thisLabName;
         int difference;

         if (thisLab->labelID == label->labelID)
            continue;
         
         thisLabName = getLabelName(thisLab);
         difference = strcmp(finalName, thisLabName);
         free(thisLabName);

         if (difference == 0) {
            ok = 0;
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
t_axe_label * assignLabel(t_program_infos *program, t_axe_label *label)
{
   t_list *li;

   /* test the preconditions */
   assert(program != NULL);
   assert(label != NULL);
   assert(label->labelID < program->current_label_ID);
   
   /* check if this label has already been assigned */
   for (li = program->instructions; li != NULL; li = li->next) {
      t_axe_instruction *instr = li->data;
      if (instr->label && compareLabels(instr->label, label))
         fatalError(AXE_LABEL_ALREADY_ASSIGNED);
   }

   /* test if the next instruction already has a label */
   if (program->label_to_assign != NULL)
   {
      /* It does: transform the label being assigned into an alias of the
       * label of the next instruction's label
       * All label aliases have the same ID and name. */

      /* Decide the name of the alias. If only one label has a name, that name
       * wins. Otherwise the name of the label with the lowest ID wins */
      char *name = program->label_to_assign->name;
      if (!name || 
            (label->labelID && 
            label->labelID < program->label_to_assign->labelID))
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
   }
   else
      program->label_to_assign = label;

   /* all went good */
   return label;
}

/* reserve a new label identifier */
t_axe_label *assignNewLabel(t_program_infos *program)
{
   t_axe_label *reserved_label;

   /* reserve a new label */
   reserved_label = newLabel(program);

   /* fix the label */
   return assignLabel(program, reserved_label);
}

char *getLabelName(t_axe_label *label)
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
void addInstruction(t_program_infos *program, t_axe_instruction *instr)
{
   t_list *ip;

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
      instr->addressParam = label;

   /* initialize the immediate field */
   instr->immediate = immediate;

   /* add the newly created instruction to the current program */
   if (program != NULL)
      addInstruction(program, instr);

   /* return the load instruction */
   return instr;
}

void removeInstructionAt(t_program_infos *program, t_list *instrLi)
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
            nextInst = genNOPInstruction(NULL);
            program->instructions = addAfter(program->instructions, instrLi, nextInst);
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
   finalizeInstruction(instrToRemove);
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

extern t_axe_data *genDataDirective(t_program_infos *program, int type,
      int value, t_axe_label *label)
{
   t_axe_data *res;

   assert(program);

   res = initializeData(type, value, label);
   program->data = addElement(program->data, res, -1);
   return res;
}

void setProgramEnd(t_program_infos *program)
{
   assert(program != NULL);

   if (program->label_to_assign != NULL)
   {
      genExit0Syscall(program);
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

      if (last_instr->opcode == OPC_CALL_EXIT_0)
         return;
   }

   genExit0Syscall(program);
   return;
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
