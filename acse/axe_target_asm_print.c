/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 * Daniele Cattaneo
 * Politecnico di Milano, 2020
 * 
 * axe_mace_asm_print.h
 * Formal Languages & Compilers Machine, 2007-2020
 */

#include <assert.h>
#include "axe_target_asm_print.h"
#include "axe_target_info.h"

#define LABEL_WIDTH (3*2)
#define INSTR_WIDTH (3*7)

extern int errorcode;

/* print out a label to the file `fp' */
void printLabel(t_axe_label *label, FILE *fp);

/* print out an opcode to the file `fp' */
static void printOpcode(int opcode, FILE *fp);

/* print out a register information to the file `fp' */
static void printRegister(t_axe_register *reg, FILE *fp);

/* Translate the assembler directives (definitions inside the data segment) */
static void translateDataSegment(t_program_infos *program, FILE *fp);

/* Translate all the instructions within the code segment */
static void translateCodeSegment(t_program_infos *program, FILE *fp);

static off_t printFormPadding(off_t formBegin, int formSize, FILE *fout);


extern const char *opcodeToString(int opcode)
{
   const char *opcode_to_string;

   switch(opcode)
   {
      case OPC_ADD : opcode_to_string = "ADD"; break;
      case OPC_SUB : opcode_to_string = "SUB"; break;
      case OPC_AND : opcode_to_string = "ANDB"; break;
      case OPC_OR : opcode_to_string = "ORB"; break;
      case OPC_XOR : opcode_to_string = "EORB"; break;
      case OPC_MUL : opcode_to_string = "MUL"; break;
      case OPC_DIV : opcode_to_string = "DIV"; break;
      case OPC_SLL : opcode_to_string = "SHL"; break;
      case OPC_SRL : opcode_to_string = "SHR"; break;
      case OPC_ADDI : opcode_to_string = "ADDI"; break;
      case OPC_SUBI : opcode_to_string = "SUBI"; break;
      case OPC_ANDI : opcode_to_string = "ANDBI"; break;
      case OPC_ORI : opcode_to_string = "ORBI"; break;
      case OPC_XORI : opcode_to_string = "EORBI"; break;
      case OPC_MULI : opcode_to_string = "MULI"; break;
      case OPC_DIVI : opcode_to_string = "DIVI"; break;
      case OPC_SLLI : opcode_to_string = "SHLI"; break;
      case OPC_SRLI : opcode_to_string = "SHRI"; break;
      case OPC_NOP : opcode_to_string = "NOP"; break;
      case OPC_HALT : opcode_to_string = "HALT"; break;
      case OPC_OLD_ANDL : opcode_to_string = "ANDL"; break;
      case OPC_OLD_ORL : opcode_to_string = "ORL"; break;
      case OPC_OLD_EORL : opcode_to_string = "EORL"; break;
      case OPC_OLD_ROTL : opcode_to_string = "ROTL"; break;
      case OPC_OLD_ROTR : opcode_to_string = "ROTR"; break;
      case OPC_OLD_NEG : opcode_to_string = "NEG"; break;
      case OPC_OLD_SPCL : opcode_to_string = "SPCL"; break;
      case OPC_OLD_ANDLI : opcode_to_string = "ANDLI"; break;
      case OPC_OLD_ORLI : opcode_to_string = "ORLI"; break;
      case OPC_OLD_EORLI : opcode_to_string = "EORLI"; break;
      case OPC_OLD_ROTLI : opcode_to_string = "ROTLI"; break;
      case OPC_OLD_ROTRI : opcode_to_string = "ROTRI"; break;
      case OPC_OLD_NOTL : opcode_to_string = "NOTL"; break;
      case OPC_OLD_NOTB : opcode_to_string = "NOTB"; break;
      case OPC_OLD_MOVA : opcode_to_string = "MOVA"; break;
      case OPC_OLD_JSR : opcode_to_string = "JSR"; break;
      case OPC_OLD_RET : opcode_to_string = "RET"; break;
      case OPC_OLD_BT : opcode_to_string = "BT"; break;
      case OPC_OLD_BF : opcode_to_string = "BF"; break;
      case OPC_OLD_BHI : opcode_to_string = "BHI"; break;
      case OPC_OLD_BLS : opcode_to_string = "BLS"; break;
      case OPC_OLD_BCC : opcode_to_string = "BCC"; break;
      case OPC_OLD_BCS : opcode_to_string = "BCS"; break;
      case OPC_OLD_BNE : opcode_to_string = "BNE"; break;
      case OPC_OLD_BEQ : opcode_to_string = "BEQ"; break;
      case OPC_OLD_BVC : opcode_to_string = "BVC"; break;
      case OPC_OLD_BVS : opcode_to_string = "BVS"; break;
      case OPC_OLD_BPL : opcode_to_string = "BPL"; break;
      case OPC_OLD_BMI : opcode_to_string = "BMI"; break;
      case OPC_OLD_BGE : opcode_to_string = "BGE"; break;
      case OPC_OLD_BLT : opcode_to_string = "BLT"; break;
      case OPC_OLD_BGT : opcode_to_string = "BGT"; break;
      case OPC_OLD_BLE : opcode_to_string = "BLE"; break;
      case OPC_OLD_LOAD : opcode_to_string = "LOAD"; break;
      case OPC_OLD_STORE : opcode_to_string = "STORE"; break;
      case OPC_OLD_SEQ : opcode_to_string = "SEQ"; break;
      case OPC_OLD_SGE : opcode_to_string = "SGE"; break;
      case OPC_OLD_SGT : opcode_to_string = "SGT"; break;
      case OPC_OLD_SLE : opcode_to_string = "SLE"; break;
      case OPC_OLD_SLT : opcode_to_string = "SLT"; break;
      case OPC_OLD_SNE : opcode_to_string = "SNE"; break;
      case OPC_AXE_READ : opcode_to_string = "READ"; break;
      case OPC_AXE_WRITE : opcode_to_string = "WRITE"; break;
      default : opcode_to_string = "<unknown>";
   }

   return opcode_to_string;
}


void writeAssembly(t_program_infos *program, char *output_file)
{
   FILE *fp;
   int _error;

   /* test the preconditions */
   if (program == NULL)
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);

   /* If necessary, set the value of `output_file' to "output.asm" */
   if (output_file == NULL)
   {
      /* set "output.asm" as output file name */
      output_file = "output.asm";
   }

#ifndef NDEBUG
   fprintf(stdout, "\n\n*******************************************\n");
   fprintf(stdout, "INITIALIZING OUTPUT FILE: %s. \n", output_file);
   fprintf(stdout, "CODE SEGMENT has a size of %d instructions \n"
         , getLength(program->instructions));
   fprintf(stdout, "DATA SEGMENT has a size of %d elements \n"
         , getLength(program->data));
   fprintf(stdout, "NUMBER OF LABELS : %d. \n"
         , getLabelCount(program->lmanager));
   fprintf(stdout, "*******************************************\n\n");
#endif
   
   /* open a new file */
   fp = fopen(output_file, "w");
   if (fp == NULL)
      notifyError(AXE_FOPEN_ERROR);

   /* print the data segment */
   translateDataSegment(program, fp);

   /* print the code segment */
   translateCodeSegment(program, fp);

   /* close the file and return */
   _error = fclose(fp);
   if (_error == EOF)
      notifyError(AXE_FCLOSE_ERROR);
}

/* translate each instruction in his assembler symbolic representation */
int translateInstruction(t_program_infos *program, t_axe_instruction *current_instruction, FILE *fp)
{
   off_t lastFormBegin = ftello(fp);
   if (current_instruction->labelID != NULL)
   {
      printLabel(current_instruction->labelID, fp);
      fprintf(fp, ":");
      lastFormBegin = printFormPadding(lastFormBegin, LABEL_WIDTH, fp);
   }
   else
   {
      /* create a string identifier for the label */
      lastFormBegin = printFormPadding(lastFormBegin, LABEL_WIDTH, fp);
   }

   /* print the opcode */
   printOpcode(current_instruction->opcode, fp);

   if (  (current_instruction->opcode == OPC_HALT)
         || (current_instruction->opcode == OPC_NOP) )
   {
      /* do nothing */
   }
   else
   {
      if (fputc(' ', fp) == EOF)
      {
         return 1;
      }

      if (current_instruction->reg_dest != NULL)
      {
         printRegister(current_instruction->reg_dest, fp);
         
         if (fputc(' ', fp) == EOF)
         {
            return 1;
         }
      }
      if (current_instruction->reg_src1 != NULL)
      {
         printRegister(current_instruction->reg_src1, fp);
         if (errorcode != AXE_OK)
            return 1;

         if (fputc(' ', fp) == EOF)
         {
            return 1;
         }
      }
      if (current_instruction->reg_src2 != NULL)
      {
         printRegister(current_instruction->reg_src2, fp);
      }
      else if (current_instruction->address != NULL)
      {
         if ((current_instruction->address)->type == ADDRESS_TYPE)
         {
            if (fprintf(fp, "%d", (current_instruction->address)->addr) < 0)
            {
               return 1;
            }
         }
         else
         {
            assert((current_instruction->address)->type == LABEL_TYPE);
            printLabel((current_instruction->address)->labelID, fp);
         }
      }
      else if (fprintf(fp, "#%d", current_instruction->immediate) < 0)
      {
         return 1;
      }
   }

   if (current_instruction->user_comment)
   {
      printFormPadding(lastFormBegin, INSTR_WIDTH, fp);
      fprintf(fp, "/* %s */", current_instruction->user_comment);
   }

   if (fprintf(fp, "\n") < 0) {
      return 1;
   }
   return 0;
}

/* translate each instruction in its assembler symbolic representation */
void translateCodeSegment(t_program_infos *program, FILE *fp)
{
   t_list *current_element;
   t_axe_instruction *current_instruction;
   int _error;
   
   /* preconditions */
   if (fp == NULL)
      notifyError(AXE_INVALID_INPUT_FILE);

   if (program == NULL)
   {
      _error = fclose(fp);
      if (_error == EOF)
         notifyError(AXE_FCLOSE_ERROR);
      notifyError(AXE_PROGRAM_NOT_INITIALIZED);
   }

   /* initialize the current_element */
   current_element = program->instructions;

   /* write the .text directive */
   if (current_element != NULL)
   {
      printFormPadding(ftello(fp), LABEL_WIDTH, fp);
      if (fprintf(fp, ".text\n") < 0)
      {
         _error = fclose(fp);
         if (_error == EOF)
            notifyError(AXE_FCLOSE_ERROR);
         notifyError(AXE_FWRITE_ERROR);
      }
   }

   while (current_element != NULL)
   {
      /* retrieve the current instruction */
      current_instruction = (t_axe_instruction *) LDATA(current_element);
      assert(current_instruction != NULL);
      assert(current_instruction->opcode != OPC_INVALID);

      if (translateInstruction(program, current_instruction, fp)) {
         _error = fclose(fp);
         if (_error == EOF)
            notifyError(AXE_FCLOSE_ERROR);
         notifyError(AXE_FWRITE_ERROR);
      }

      /* loop termination condition */
      current_element = LNEXT(current_element);
   }
}

void translateDataSegment(t_program_infos *program, FILE *fp)
{
   t_list *current_element;
   t_axe_data *current_data;
   int _error;
   int fprintf_error;
   
   /* preconditions */
   if (fp == NULL)
      notifyError(AXE_INVALID_INPUT_FILE);

   /* initialize the local variable `fprintf_error' */
   fprintf_error = 0;
   
   if (program == NULL)
   {
      _error = fclose(fp);
      if (_error == EOF)
         notifyError(AXE_FCLOSE_ERROR);

      notifyError(AXE_PROGRAM_NOT_INITIALIZED);
   }

   /* initialize the value of `current_element' */
   current_element = program->data;

   /* write the .data directive */
   if (current_element != NULL)
   {
      printFormPadding(ftello(fp), LABEL_WIDTH, fp);
      if (fprintf(fp, ".data\n") < 0)
      {
         _error = fclose(fp);
         if (_error == EOF)
            notifyError(AXE_FCLOSE_ERROR);
         notifyError(AXE_FWRITE_ERROR);
      }
   }

   /* iterate all the elements inside the data segment */
   while (current_element != NULL)
   {
      off_t lastFormBegin = ftello(fp);

      /* retrieve the current data element */
      current_data = (t_axe_data *) LDATA(current_element);

      /* assertions */
      assert (current_data->directiveType != DIR_INVALID);

      /* create a string identifier for the label */
      if (current_data->labelID != NULL)
      {
         printLabel(current_data->labelID, fp);
         fprintf_error = fprintf(fp, ":");
      }
      printFormPadding(lastFormBegin, LABEL_WIDTH, fp);

      /* test if an error occurred while executing the `fprintf' function */
      if (fprintf_error < 0)
      {
         _error = fclose(fp);
         if (_error == EOF)
            notifyError(AXE_FCLOSE_ERROR);
         notifyError(AXE_FWRITE_ERROR);
      }

      /* print the directive identifier */
      if (current_data->directiveType == DIR_WORD)
      {
         if (fprintf(fp, ".WORD ") < 0)
         {
            _error = fclose(fp);
            if (_error == EOF)
               notifyError(AXE_FCLOSE_ERROR);
            notifyError(AXE_FWRITE_ERROR);
         }
      }
      
      else if (current_data->directiveType == DIR_SPACE)
      {
         if (fprintf(fp, ".SPACE ") < 0)
         {
            _error = fclose(fp);
            if (_error == EOF)
               notifyError(AXE_FCLOSE_ERROR);
            notifyError(AXE_FWRITE_ERROR);
         }
      }

      /* print the value associated with the directive */
      if (fprintf(fp, "%d\n", current_data->value) < 0)
      {
         _error = fclose(fp);
         if (_error == EOF)
            notifyError(AXE_FCLOSE_ERROR);
         notifyError(AXE_FWRITE_ERROR);
      }

      /* loop termination condition */
      current_element = LNEXT(current_element);
   }
}

void printOpcode(int opcode, FILE *fp)
{
   const char *opcode_to_string;
   int _error;
   
   /* preconditions: fp must be different from NULL */
   if (fp == NULL)
      notifyError(AXE_INVALID_INPUT_FILE);

   opcode_to_string = opcodeToString(opcode);
   
   /* postconditions */
   if (fprintf(fp, "%s", opcode_to_string) < 0)
   {
      _error = fclose(fp);
      if (_error == EOF)
         notifyError(AXE_FCLOSE_ERROR);
      notifyError(AXE_FWRITE_ERROR);
   }
}

void printLabel(t_axe_label *label, FILE *fp)
{
   if (!label)
      return;
   
   if (label->name) {
      fputs(label->name, fp);
   } else {
      fprintf(fp, "L%u", label->labelID);
   }
}

void printRegister(t_axe_register *reg, FILE *fp)
{
   int _error;
   
   /* preconditions: fp must be different from NULL */
   if (fp == NULL)
      notifyError(AXE_INVALID_INPUT_FILE);
   if (reg == NULL)
   {
      _error = fclose(fp);
      if (_error == EOF)
         notifyError(AXE_FCLOSE_ERROR);
      notifyError(AXE_INVALID_REGISTER_INFO);
   }
   if (reg->ID == REG_INVALID)
   {
      _error = fclose(fp);
      if (_error == EOF)
         notifyError(AXE_FCLOSE_ERROR);
      notifyError(AXE_INVALID_REGISTER_INFO);
   }

   if (reg->indirect)
   {
      if (fprintf(fp, "(R%d)", reg->ID) < 0)
      {
         _error = fclose(fp);
         if (_error == EOF)
            notifyError(AXE_FCLOSE_ERROR);
         notifyError(AXE_FWRITE_ERROR);
      }
   }
   else
   {
      if (fprintf(fp, "R%d", reg->ID) < 0)
      {
         _error = fclose(fp);
         if (_error == EOF)
            notifyError(AXE_FCLOSE_ERROR);
         notifyError(AXE_FWRITE_ERROR);
      }
   }
}

off_t printFormPadding(off_t formBegin, int formSize, FILE *fout)
{
   off_t currentLoc = ftello(fout);
   off_t padding = formSize - (currentLoc - formBegin);
   if (padding > 1) {
      off_t i;
      for (i = 0; i < padding - 1; i++) {
         putc(' ', fout);
      }
   }
   putc(' ', fout);
   return ftello(fout);
}

