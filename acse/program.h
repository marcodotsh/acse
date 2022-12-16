/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 *
 * program.h
 * Formal Languages & Compilers Machine, 2007/2008
 *
 * Contains t_program and some functions for label management
 * (reserve, fix, assign)
 */

#ifndef PROGRAM_H
#define PROGRAM_H

#include "list.h"


typedef struct t_label {
   unsigned int labelID; /* Unique identifier for the label */
   char *name;           /* Name of the label. If NULL, the name will be
                          * automatically generated in the form L<ID>. */
   int global;           /* zero for local labels, non-zero for global labels.*/
   int isAlias;
} t_label;

/* registers */
#define REG_INVALID -1
#define REG_0       0

typedef struct t_instrArg {
   int ID;                     /* an identifier of the register */
   t_listNode *mcRegWhitelist; /* the list of machine registers where this
                                * variable can be allocated. NULL if any
                                * register is allowed. */
} t_instrArg;

/* a symbolic assembly instruction */
typedef struct t_instruction {
   t_label *label;        /* a label associated with the current
                           * instruction */
   int opcode;            /* instruction opcode (for example: ADD) */
   t_instrArg *reg_dest;  /* destination register */
   t_instrArg *reg_src1;  /* first source register */
   t_instrArg *reg_src2;  /* second source register */
   int immediate;         /* immediate value */
   t_label *addressParam; /* an address operand */
   char *user_comment;    /* if defined it is set to the source code
                           * instruction that generated the current
                           * assembly. This string will be written
                           * into the output code as a comment */
} t_instruction;

/* DIRECTIVE TYPES */
#define DIR_WORD  0
#define DIR_SPACE 1

/* this structure is used in order to define assembler directives.
 * Directives are used in many cases such the definition of variables
 * inside the data segment. Every instance `t_global' contains
 * all the informations about a single directive.
 * An example is the directive .word that is required when the assembler
 * must reserve a word of data inside the data segment. */
typedef struct t_global {
   int directiveType; /* the type of the current directive
                       * (for example: DIR_WORD) */
   int value;         /* the value associated with the directive */
   t_label *labelID;  /* label associated with the current data */
} t_global;

typedef struct t_program {
   t_listNode *labels;
   t_listNode *instructions;
   t_listNode *data;
   t_listNode *variables;
   int current_register;
   unsigned int current_label_ID;
   t_label *label_to_assign;
} t_program;


/* Program */

/* initialize the informations associated with the program. This function is
 * called at the beginning of the translation process. This function
 * is called once: its only purpouse is to initialize an instance of the struct
 * `t_program' that will contain all the informations about the program
 * that will be compiled */
extern t_program *newProgram(void);

/* finalize all the data structures associated with `program' */
extern void deleteProgram(t_program *program);


/* Labels */

/* reserve a new label identifier and return the identifier to the caller */
extern t_label *createLabel(t_program *program);

/* assign the given label identifier to the next instruction. Returns
 * the label assigned; otherwise (an error occurred) NULL */
extern t_label *assignLabel(t_program *program, t_label *label);

/* reserve and fix a new label. It returns either the label assigned or
 * NULL if an error occurred */
extern t_label *assignNewLabel(t_program *program);

/* Sets the name of a label to the specified string. Note: if another label
 * with the same name already exists, the name assigned to this label will be
 * modified to remove any ambiguity. */
extern void setLabelName(t_program *program, t_label *label, const char *name);

/* Returns a dynamically allocated string that contains the name of the given
 * label. */
extern char *getLabelName(t_label *label);

/* return TRUE if the two labels hold the same identifier */
extern int compareLabels(t_label *labelA, t_label *labelB);


/* Instructions */

/* get a register still not used. This function returns
 * the ID of the register found*/
extern int getNewRegister(t_program *program);

/* add a new instruction to the current program. This function is directly
 * called by all the functions defined in `gencode.h' */
extern t_instruction *genInstruction(t_program *program, int opcode, int r_dest,
      int r_src1, int r_src2, t_label *label, int immediate);

/* remove an instruction from the program, given its link in the instruction
 * list. */
extern void removeInstructionAt(t_program *program, t_listNode *instrLi);


/* Directives */

extern t_global *genDataDirective(
      t_program *program, int type, int value, t_label *label);


/* Utility */

/* Notify the end of the program. This function is directly called
 * from the parser when the parsing process is ended */
extern void setProgramEnd(t_program *program);

/* print information about the program in the specified file */
extern void dumpProgram(t_program *program, FILE *fout);


#endif
