/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 *
 * program.h
 * Formal Languages & Compilers Machine, 2007/2008
 *
 * Contains t_program_infos and some functions for label management
 * (reserve, fix, assign)
 */

#ifndef PROGRAM_H
#define PROGRAM_H

#include "list.h"


typedef struct t_axe_label {
   unsigned int labelID; /* Unique identifier for the label */
   char *name;           /* Name of the label. If NULL, the name will be 
                          * automatically generated in the form L<ID>. */
   int global;           /* zero for local labels, non-zero for global labels.*/
   int isAlias;
} t_axe_label;

/* registers */
#define REG_INVALID -1
#define REG_0 0

typedef struct t_axe_register {
   int ID;       /* an identifier of the register */
   t_list *mcRegWhitelist; /* the list of machine registers where this
                            * variable can be allocated. NULL if any register
                            * is allowed. */
} t_axe_register;

/* a symbolic assembly instruction */
typedef struct t_axe_instruction {
   t_axe_label *label;        /* a label associated with the current
                               * instruction */
   int opcode;                /* instruction opcode (for example: ADD) */
   t_axe_register *reg_dest;  /* destination register */
   t_axe_register *reg_src1;  /* first source register */
   t_axe_register *reg_src2;  /* second source register */
   int immediate;             /* immediate value */
   t_axe_label *addressParam; /* an address operand */
   char *user_comment;        /* if defined it is set to the source code
                               * instruction that generated the current
                               * assembly. This string will be written
                               * into the output code as a comment */
} t_axe_instruction;

/* DIRECTIVE TYPES */
#define DIR_WORD 0
#define DIR_SPACE 1

/* this structure is used in order to define assembler directives.
 * Directives are used in many cases such the definition of variables
 * inside the data segment. Every instance `t_axe_data' contains
 * all the informations about a single directive.
 * An example is the directive .word that is required when the assembler
 * must reserve a word of data inside the data segment. */
typedef struct t_axe_data {
   int directiveType;    /* the type of the current directive
                          * (for example: DIR_WORD) */
   int value;            /* the value associated with the directive */
   t_axe_label *labelID; /* label associated with the current data */
} t_axe_data;

typedef struct t_program_infos {
   t_list *labels;
   t_list *instructions;
   t_list *data;
   t_list *variables;
   int current_register;
   unsigned int current_label_ID;
   t_axe_label *label_to_assign;
} t_program_infos;


/* Program */

/* initialize the informations associated with the program. This function is
 * called at the beginning of the translation process. This function
 * is called once: its only purpouse is to initialize an instance of the struct
 * `t_program_infos' that will contain all the informations about the program
 * that will be compiled */
extern t_program_infos *allocProgramInfos(void);

/* finalize all the data structures associated with `program' */
extern void finalizeProgramInfos(t_program_infos *program);


/* Labels */

/* reserve a new label identifier and return the identifier to the caller */
extern t_axe_label *newLabel(t_program_infos *program);

/* assign the given label identifier to the next instruction. Returns
 * the label assigned; otherwise (an error occurred) NULL */
extern t_axe_label *assignLabel(t_program_infos *program, t_axe_label *label);

/* reserve and fix a new label. It returns either the label assigned or
 * NULL if an error occurred */
extern t_axe_label *assignNewLabel(t_program_infos *program);

/* Sets the name of a label to the specified string. Note: if another label
 * with the same name already exists, the name assigned to this label will be
 * modified to remove any ambiguity. */
extern void setLabelName(t_program_infos *program, t_axe_label *label,
      const char *name);

/* Returns a dynamically allocated string that contains the name of the given
 * label. */
extern char *getLabelName(t_axe_label *label);

/* return TRUE if the two labels hold the same identifier */
extern int compareLabels(t_axe_label *labelA, t_axe_label *labelB);


/* Instructions */

/* get a register still not used. This function returns
 * the ID of the register found*/
extern int getNewRegister(t_program_infos *program);

/* Set the list of allowed machine registers for a specific register object
 * to the specified list of register identifiers. The list must be terminated
 * by REG_INVALID or -1. */
extern void setMCRegisterWhitelist(t_axe_register *regObj, ...);

/* add a new instruction to the current program. This function is directly
 * called by all the functions defined in `gencode.h' */
extern void addInstruction(t_program_infos *program, t_axe_instruction *instr);

extern t_axe_instruction *genInstruction(t_program_infos *program, int opcode,
      int r_dest, int r_src1, int r_src2, t_axe_label *label, int immediate);

/* remove an instruction from the program, given its link in the instruction
 * list. */
extern void removeInstructionLink(t_program_infos *program, t_list *instrLi);


/* Directives */

extern t_axe_data *genDataDirective(t_program_infos *program, int type, 
      int value, t_axe_label *label);


/* Utility */

/* Notify the end of the program. This function is directly called
 * from the parser when the parsing process is ended */
extern void setProgramEnd(t_program_infos *program);

/* print information about the program in the specified file */
extern void printProgramInfos(t_program_infos *program, FILE *fout);


#endif
