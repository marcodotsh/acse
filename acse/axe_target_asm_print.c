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

void printAddress(t_axe_address *addr, FILE *fp);

/* Translate the assembler directives (definitions inside the data segment) */
static void translateDataSegment(t_program_infos *program, FILE *fp);

/* Translate all the instructions within the code segment */
static void translateCodeSegment(t_program_infos *program, FILE *fp);

static off_t printFormPadding(off_t formBegin, int formSize, FILE *fout);


extern const char *opcodeToString(int opcode)
{
   switch (opcode) {
      /*   Arithmetic */
      case OPC_ADD:       return "add";
      case OPC_SUB:       return "sub";
      case OPC_AND:       return "and";
      case OPC_OR:        return "or";
      case OPC_XOR:       return "xor";
      case OPC_MUL:       return "mul";
      case OPC_DIV:       return "div";
      case OPC_SLL:       return "sll";
      case OPC_SRL:       return "srl";
      case OPC_SRA:       return "sra";
      /*   Arithmetic with immediate */
      case OPC_ADDI:      return "addi";
      case OPC_SUBI:      return "subi";
      case OPC_ANDI:      return "andi";
      case OPC_ORI:       return "ori";
      case OPC_XORI:      return "xori";
      case OPC_MULI:      return "muli";
      case OPC_DIVI:      return "divi";
      case OPC_SLLI:      return "slli";
      case OPC_SRLI:      return "srli";
      case OPC_SRAI:      return "srai";
      /*   Comparison */
      case OPC_SEQ :      return "seq";
      case OPC_SNE :      return "sne";
      case OPC_SLT :      return "slt";
      case OPC_SLTU:      return "sltu";
      case OPC_SGE :      return "sge";
      case OPC_SGEU:      return "sgeu";
      case OPC_SGT :      return "sgt";
      case OPC_SGTU:      return "sgtu";
      case OPC_SLE :      return "sle";
      case OPC_SLEU:      return "sleu";
      /*   Jump, Branch */
      case OPC_J :        return "j";
      case OPC_BEQ :      return "beq";
      case OPC_BNE :      return "bne";
      case OPC_BLT :      return "blt";
      case OPC_BLTU:      return "bltu";
      case OPC_BGE :      return "bge";
      case OPC_BGEU:      return "bgeu";
      case OPC_BGT :      return "bgt";
      case OPC_BGTU:      return "bgtu";
      case OPC_BLE :      return "ble";
      case OPC_BLEU:      return "bleu";
      /*   Load/Store */
      case OPC_LW  :      return "lw";
      case OPC_SW  :      return "sw";
      case OPC_LI  :      return "li";
      case OPC_LA  :      return "la";
      /*   Other */
      case OPC_NOP:       return "nop";
      case OPC_ECALL:     return "ecall";
      case OPC_EBREAK:    return "ebreak";
      /*   Syscall */
      case OPC_HALT:      return "HALT";
      case OPC_AXE_READ:  return "READ";
      case OPC_AXE_WRITE: return "WRITE";
      /* (to be removed) */
      case OPC_OLD_ANDL:  return "OLD_ANDL";
      case OPC_OLD_ORL:   return "OLD_ORL";
      case OPC_OLD_EORL:  return "OLD_EORL";
      case OPC_OLD_ROTL:  return "OLD_ROTL";
      case OPC_OLD_ROTR:  return "OLD_ROTR";
      case OPC_OLD_NEG:   return "OLD_NEG";
      case OPC_OLD_SPCL:  return "OLD_SPCL";
      case OPC_OLD_ANDLI: return "OLD_ANDLI";
      case OPC_OLD_ORLI:  return "OLD_ORLI";
      case OPC_OLD_EORLI: return "OLD_EORLI";
      case OPC_OLD_ROTLI: return "OLD_ROTLI";
      case OPC_OLD_ROTRI: return "OLD_ROTRI";
      case OPC_OLD_NOTL:  return "OLD_NOTL";
      case OPC_OLD_NOTB:  return "OLD_NOTB";
      case OPC_OLD_JSR:   return "OLD_JSR";
      case OPC_OLD_RET:   return "OLD_RET";
      case OPC_OLD_BT:    return "OLD_BT";
      case OPC_OLD_BF:    return "OLD_BF";
      case OPC_OLD_BHI:   return "OLD_BHI";
      case OPC_OLD_BLS:   return "OLD_BLS";
      case OPC_OLD_BCC:   return "OLD_BCC";
      case OPC_OLD_BCS:   return "OLD_BCS";
      case OPC_OLD_BNE:   return "OLD_BNE";
      case OPC_OLD_BEQ:   return "OLD_BEQ";
      case OPC_OLD_BVC:   return "OLD_BVC";
      case OPC_OLD_BVS:   return "OLD_BVS";
      case OPC_OLD_BPL:   return "OLD_BPL";
      case OPC_OLD_BMI:   return "OLD_BMI";
      case OPC_OLD_BGE:   return "OLD_BGE";
      case OPC_OLD_BLT:   return "OLD_BLT";
      case OPC_OLD_BGT:   return "OLD_BGT";
      case OPC_OLD_BLE:   return "OLD_BLE";
      case OPC_OLD_LOAD:  return "OLD_LOAD";
      case OPC_OLD_STORE: return "OLD_STORE";
      case OPC_OLD_SEQ:   return "OLD_SEQ";
      case OPC_OLD_SGE:   return "OLD_SGE";
      case OPC_OLD_SGT:   return "OLD_SGT";
      case OPC_OLD_SLE:   return "OLD_SLE";
      case OPC_OLD_SLT:   return "OLD_SLT";
      case OPC_OLD_SNE:   return "OLD_SNE";
   }
   return "<unknown>";
}


#define FORMAT_AUTO  -1
#define FORMAT_OP     0   /* mnemonic rd, rs1, rs2    */
#define FORMAT_OPIMM  1   /* mnemonic rd, rs1, imm    */
#define FORMAT_LOAD   2   /* mnemonic rd, imm(rs1)    */
#define FORMAT_STORE  3   /* mnemonic rs2, imm(rs1)   */
#define FORMAT_BRANCH 4   /* mnemonic rs1, rs2, label */
#define FORMAT_JUMP   5   /* mnemonic label           */
#define FORMAT_LI     6   /* mnemonic rd, imm         */
#define FORMAT_LA     7   /* mnemonic rd, label       */
#define FORMAT_SYSTEM 8   /* mnemonic                 */

static int opcodeToFormat(int opcode)
{
   switch (opcode) {
      case OPC_ADD:
      case OPC_SUB:
      case OPC_AND:
      case OPC_OR :
      case OPC_XOR:
      case OPC_MUL:
      case OPC_DIV:
      case OPC_SLL:
      case OPC_SRL:
      case OPC_SRA:
      case OPC_SEQ :
      case OPC_SNE :
      case OPC_SLT :
      case OPC_SLTU:
      case OPC_SGE :
      case OPC_SGEU:
      case OPC_SGT :
      case OPC_SGTU:
      case OPC_SLE :
      case OPC_SLEU:
         return FORMAT_OP;
      case OPC_ADDI:
      case OPC_SUBI:
      case OPC_ANDI:
      case OPC_ORI :
      case OPC_XORI:
      case OPC_MULI:
      case OPC_DIVI:
      case OPC_SLLI:
      case OPC_SRLI:
      case OPC_SRAI:
      case OPC_SEQI :
      case OPC_SNEI :
      case OPC_SLTI :
      case OPC_SLTIU:
      case OPC_SGEI :
      case OPC_SGEIU:
      case OPC_SGTI :
      case OPC_SGTIU:
      case OPC_SLEI :
      case OPC_SLEIU:
         return FORMAT_OPIMM;
      case OPC_J:
         return FORMAT_JUMP;
      case OPC_BEQ :
      case OPC_BNE :
      case OPC_BLT :
      case OPC_BLTU:
      case OPC_BGE :
      case OPC_BGEU:
      case OPC_BGT :
      case OPC_BGTU:
      case OPC_BLE :
      case OPC_BLEU:
         return FORMAT_BRANCH;
      case OPC_LW:
         return FORMAT_LOAD;
      case OPC_SW:
         return FORMAT_STORE;
      case OPC_LI:
         return FORMAT_LI;
      case OPC_LA:
         return FORMAT_LA;
      case OPC_NOP:
      case OPC_ECALL:
      case OPC_EBREAK:
      case OPC_HALT:
         return FORMAT_SYSTEM;
   }
   return -1;
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
int printInstruction(t_axe_instruction *current_instruction, FILE *fp)
{
   int format;
   int rd, rs1, rs2;

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
   fputc(' ', fp);

   format = opcodeToFormat(current_instruction->opcode);
   switch (format) {
      case FORMAT_OP:
         printRegister(current_instruction->reg_dest, fp);
         fprintf(fp, ", ");
         printRegister(current_instruction->reg_src1, fp);
         fprintf(fp, ", ");
         printRegister(current_instruction->reg_src2, fp);
         break;
      case FORMAT_OPIMM:
         printRegister(current_instruction->reg_dest, fp);
         fprintf(fp, ", ");
         printRegister(current_instruction->reg_src1, fp);
         fprintf(fp, ", %d", current_instruction->immediate);
         break;
      case FORMAT_LOAD:
         printRegister(current_instruction->reg_dest, fp);
         fprintf(fp, ", %d(", current_instruction->immediate);
         printRegister(current_instruction->reg_src1, fp);
         fprintf(fp, ")");
         break;
      case FORMAT_STORE:
         printRegister(current_instruction->reg_src2, fp);
         fprintf(fp, ", %d(", current_instruction->immediate);
         printRegister(current_instruction->reg_src1, fp);
         fprintf(fp, ")");
         break;
      case FORMAT_BRANCH:
         printRegister(current_instruction->reg_src1, fp);
         fprintf(fp, ", ");
         printRegister(current_instruction->reg_src2, fp);
         fprintf(fp, ", ");
         printAddress(current_instruction->address, fp);
         break;
      case FORMAT_JUMP:
         printAddress(current_instruction->address, fp);
         break;
      case FORMAT_LI:
         printRegister(current_instruction->reg_dest, fp);
         fprintf(fp, ", %d", current_instruction->immediate);
         break;
      case FORMAT_LA:
         printRegister(current_instruction->reg_dest, fp);
         fprintf(fp, ", ");
         printAddress(current_instruction->address, fp);
         break;
      default:
      case FORMAT_SYSTEM:
         break;
   }

   if (current_instruction->user_comment)
   {
      printFormPadding(lastFormBegin, INSTR_WIDTH, fp);
      fprintf(fp, "/* %s */", current_instruction->user_comment);
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

      printInstruction(current_instruction, fp);
      if (fprintf(fp, "\n") < 0) {
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
      if (fprintf(fp, "(x%d)", reg->ID) < 0)
      {
         _error = fclose(fp);
         if (_error == EOF)
            notifyError(AXE_FCLOSE_ERROR);
         notifyError(AXE_FWRITE_ERROR);
      }
   }
   else
   {
      if (fprintf(fp, "x%d", reg->ID) < 0)
      {
         _error = fclose(fp);
         if (_error == EOF)
            notifyError(AXE_FCLOSE_ERROR);
         notifyError(AXE_FWRITE_ERROR);
      }
   }
}

void printAddress(t_axe_address *addr, FILE *fp)
{
   if (!fp)
      notifyError(AXE_INVALID_INPUT_FILE);
   if (!addr)
      notifyError(AXE_INVALID_ADDRESS);
   
   if (addr->type == ADDRESS_TYPE)
   {
      fprintf(fp, "%d", addr->addr < 0);
   }
   else
   {
      assert(addr->type == LABEL_TYPE);
      printLabel(addr->labelID, fp);
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

