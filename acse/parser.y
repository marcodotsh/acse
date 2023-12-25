/// @file parser.y
/// @brief The bison grammar file that describes the LANCE language and
///        the semantic actions used to translate it to assembly code.

%{
#include <stdio.h>     
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include "program.h"
#include "target_asm_print.h"
#include "target_transform.h"
#include "target_info.h"
#include "errors.h"
#include "list.h"
#include "gencode.h"
#include "utils.h"
#include "symbols.h"
#include "cflow_graph.h"
#include "reg_alloc.h"
#include "options.h"

/* Global Variables */

/* This variable will keep track of the source code line number.
 *   Every time a newline is encountered while scanning the input file, the
 * lexer increases this variable by 1.
 *   This way, when the parser returns an error or a warning, it will look at
 * this variable to report where the error happened. */
int line_num;

/* The number of errors and warnings found in the code. */
int num_error;
int num_warning;    
              
/* The singleton instance of `program'.
 *   An instance of `t_program' holds in its internal structure all the
 * fundamental informations about the program being compiled:
 *   - the list of instructions
 *   - the list of data directives (static allocations)
 *   - the list of variables
 *   - the list of labels
 *   - ... */
t_program *program;

/* This global variable is the file read by the Flex-generated scanner */
extern FILE *yyin;


/* Forward Declarations */

/* yylex() is the function generated by Flex that returns a the next token
 * scanned from the input. */
int yylex(void);

/* yyerror() is a function defined later in this file used by the bison-
 * generated parser to notify that a syntax error occurred. */
void yyerror(const char*);

%}

/* EXPECT DECLARATION: notifies bison that the grammar contains the specified
 * number of shift-reduce conflicts. If the number of conflicts matches what is
 * specified, bison will automatically resolve them. */
%expect 1

/* AXIOM DECLARATION. The starting non-terminal of the grammar will be
 * `program'. */
%start program

/******************************************************************************
 * UNION DECLARATION
 *
 * Specifies the set of types available for the semantic values of terminal
 * and non-terminal symbols.
 ******************************************************************************/
%union {
  int integer;
  char *string;
  t_symbol *var;
  t_expressionValue expr;
  t_listNode *list;
  t_label *label;
  t_whileStatement while_stmt;
}

/******************************************************************************
 * TOKEN SYMBOL DECLARATIONS
 *
 * Here we declare all the token symbols that can be produced by the scanner.
 * Bison will automatically produce a #define that assigns a number (or token
 * identifier) to each one of these tokens.
 *   We also declare the type for the semantic values of some of these tokens.
 ******************************************************************************/

%token EOF_TOK /* end of file */
%token LPAR RPAR LSQUARE RSQUARE LBRACE RBRACE
%token COMMA SEMI PLUS MINUS MUL_OP DIV_OP
%token AND_OP OR_OP NOT_OP
%token ASSIGN LT GT SHL_OP SHR_OP EQ NOTEQ LTEQ GTEQ
%token ANDAND OROR
%token TYPE
%token RETURN
%token READ WRITE

/* These are the tokens with a semantic value of the given type. */
%token <label> DO
%token <while_stmt> WHILE
%token <label> IF
%token <label> ELSE
%token <string> IDENTIFIER
%token <integer> NUMBER

/******************************************************************************
 * NON-TERMINAL SYMBOL SEMANTIC VALUE TYPE DECLARATIONS
 *
 * Here we declare the type of the semantic values of non-terminal symbols.
 *   We only declare the type of non-terminal symbols of which we actually use
 * their semantic value.
 ******************************************************************************/

%type <var> var_id
%type <expr> exp
%type <label> if_stmt

/******************************************************************************
 * OPERATOR PRECEDENCE AND ASSOCIATIVITY
 *
 * Precedence is given by the declaration order. Associativity is given by the
 * specific keyword used (%left, %right).
 ******************************************************************************/

%left COMMA
%left ASSIGN
%left OROR
%left ANDAND
%left OR_OP
%left AND_OP
%left EQ NOTEQ
%left LT GT LTEQ GTEQ
%left SHL_OP SHR_OP
%left MINUS PLUS
%left MUL_OP DIV_OP
%right NOT_OP

/******************************************************************************
 * GRAMMAR AND SEMANTIC ACTIONS
 *
 * The grammar of the language follows. The semantic actions are the pieces of
 * C code enclosed in {} brackets: they are executed when the rule has been
 * parsed and recognized up to the point where the semantic action appears.
 ******************************************************************************/
%% 

/* `program' is the starting non-terminal of the grammar.
 * A program is composed by:
 *   1. declarations (zero or more);
 *   2. A list of instructions. (at least one instruction!).
 * When the rule associated with the non-terminal `program' is executed,
 * the parser notifies it to the `program' singleton instance. */
program : var_declarations statements EOF_TOK
        {
          /* Notify the end of the program. Once called
           * the function `setProgramEnd' - if necessary -
           * introduces a `HALT' instruction into the
           * list of instructions. */
          setProgramEnd(program);

          /* return from yyparse() */
          YYACCEPT;
        }
;

var_declarations: var_declarations var_declaration
                | %empty
;

var_declaration : TYPE declaration_list SEMI
;

declaration_list: declaration_list COMMA declaration
                | declaration
;

declaration : IDENTIFIER
            {
              createSymbol(program, $1, TYPE_INT, 0);
            }
            | IDENTIFIER LSQUARE NUMBER RSQUARE
            {
              createSymbol(program, $1, TYPE_INT_ARRAY, $3);
            }
;

/* A block of code can be either a single statement or
 * a set of statements enclosed between braces */
code_block: statement                 { /* does nothing */ }
          | LBRACE statements RBRACE  { /* does nothing */ }
;

/* One or more code statements */
statements: statements statement  { /* does nothing */ }
          | statement             { /* does nothing */ }
;

/* A statement can be either an assignment statement or a control statement
 * or a read/write statement or a semicolon */
statement : assign_statement SEMI     { /* does nothing */ }
          | control_statement         { /* does nothing */ }
          | read_write_statement SEMI { /* does nothing */ }
          | SEMI                      { genNOPInstruction(program); }
;

control_statement : if_statement            { /* does nothing */ }
                  | while_statement         { /* does nothing */ }
                  | do_while_statement SEMI { /* does nothing */ }
                  | return_statement SEMI   { /* does nothing */ }
;

read_write_statement: read_statement  { /* does nothing */ }
                    | write_statement { /* does nothing */ }
;

assign_statement: var_id LSQUARE exp RSQUARE ASSIGN exp
                {
                  /* Notify to `program' that the value $6
                   * have to be assigned to the location
                   * addressed by $1[$3]. Where $1 is obviously
                   * the array/pointer object, $3 is an expression
                   * that holds an integer value. That value will be
                   * used as an index for the array $1 */
                  genStoreArrayElement(program, $1, $3, $6);
                }
                | var_id ASSIGN exp
                {
                  genStoreVariable(program, $1, $3);
                }
;
        
if_statement: if_stmt
            {
              /* fix the `label_else' */
              assignLabel(program, $1);
            }
            | if_stmt ELSE
            {
              /* reserve a new label that points to the address where to
               * jump if `exp' is verified */
              $2 = createLabel(program);

              /* exit from the if-else */
              genJInstruction(program, $2);

              /* fix the `label_else' */
              assignLabel(program, $1);
            }
            code_block
            {
              /* fix the `label_else' */
              assignLabel(program, $2);
            }
;
        
if_stmt : IF
        {
          /* the label that points to the address where to jump if
           * `exp' is not verified */
          $1 = createLabel(program);
        }
        LPAR exp RPAR
        {
          if ($4.type == CONSTANT) {
            if ($4.immediate == 0)
              genJInstruction(program, $1);
          } else {
            /* if `exp' returns FALSE, jump to the label $1 */
            genBEQInstruction(program, $4.registerId, REG_0, $1);
          }
        }
        code_block
        {
          $$ = $1;
        }
;

while_statement : WHILE
                {
                  /* reserve and fix a new label */
                  $1.label_condition = assignNewLabel(program);
                }
                LPAR exp RPAR
                {
                  /* reserve a new label. This new label will point
                   * to the first instruction after the while code
                   * block */
                  $1.label_end = createLabel(program);

                  if ($4.type == CONSTANT) {
                    if ($4.immediate == 0)
                      genJInstruction(program, $1.label_end);
                  } else {
                    /* if `exp' returns FALSE, jump to the label 
                     * $1.label_end */
                    genBEQInstruction(program, $4.registerId, REG_0, 
                                $1.label_end);
                  }
                }
                code_block
                {
                  /* jump to the beginning of the loop */
                  genJInstruction(program, $1.label_condition);

                  /* fix the label `label_end' */
                  assignLabel(program, $1.label_end);
                }
;
            
do_while_statement: DO
                  {
                    /* the label that points to the address where to jump if
                     * `exp' is not verified */
                    $1 = createLabel(program);

                    /* fix the label */
                    assignLabel(program, $1);
                  }
                  code_block WHILE LPAR exp RPAR
                  {
                    if ($6.type == CONSTANT) {
                      if ($6.immediate != 0)
                        genJInstruction(program, $1);
                    } else {
                      /* if `exp' returns TRUE, jump to the label $1 */
                      genBNEInstruction(program, $6.registerId, REG_0, $1);
                    }
                  }
;

return_statement: RETURN
                {
                  /* insert an HALT instruction */
                  genExit0Syscall(program);
                }
;

read_statement: READ LPAR var_id RPAR 
              {
                t_regID r_tmp = getNewRegister(program);
                genReadIntSyscall(program, r_tmp);
                genStoreVariable(program, $3, registerExpressionValue(r_tmp));
              }
;
        
write_statement : WRITE LPAR exp RPAR 
                {
                  t_regID location;
                  if ($3.type == CONSTANT) {
                    /* load `immediate' into a new register. Returns the new
                     * register identifier or REG_INVALID if an error occurs */
                    location = getNewRegister(program);
                    genLIInstruction(program, location, $3.immediate);
                  } else {
                    location = $3.registerId;
                  }
                  /* write to standard output an integer value */
                  genPrintIntSyscall(program, location);

                  /* write a newline to standard output */
                  location = getNewRegister(program);
                  genLIInstruction(program, location, '\n');
                  genPrintCharSyscall(program, location);
                }
;

exp : NUMBER
    { 
      $$ = constantExpressionValue($1);
    }
    | var_id 
    {
      t_regID r_value = genLoadVariable(program, $1);
      $$ = registerExpressionValue(r_value);
    }
    | var_id LSQUARE exp RSQUARE
    {
      /* load the value IDENTIFIER[exp]
        * into `arrayElement' */
      t_regID reg = genLoadArrayElement(program, $1, $3);

      /* create a new expression */
      $$ = registerExpressionValue(reg);
    }
    | NOT_OP exp
    {
      if ($2.type == CONSTANT) {
        /* CONSTANT (constant) expression: compute the value at
          * compile-time and place the result in a new CONSTANT
          * expression */
        $$ = constantExpressionValue(!($2.immediate));
      } else {
        /* REGISTER expression: generate the code that will compute
         * the result at compile time */

        /* Reserve a new register for the result */
        t_regID res_reg = getNewRegister(program);

        /* Generate a SUBI instruction which will store the negated
          * logic value into the register we reserved */
        genSEQInstruction(program, res_reg, $2.registerId, REG_0);

        /* Return a REGISTER expression with the result register */
        $$ = registerExpressionValue(res_reg);
      }
    }
    | MINUS exp
    {
      if ($2.type == CONSTANT) {
        $$ = constantExpressionValue(-($2.immediate));
      } else {
        t_regID res = getNewRegister(program);
        genSUBInstruction(program, res, REG_0, $2.registerId);
        $$ = registerExpressionValue(res);
      }
    }
    | exp AND_OP exp 
    {
      if ($1.type == CONSTANT && $3.type == CONSTANT) {
        $$ = constantExpressionValue($1.immediate & $3.immediate);
      } else {
        t_regID rd = getNewRegister(program);
        t_regID rs1 = genConvertExpValueToRegister(program, $1);
        if ($3.type == REGISTER) {
          genANDInstruction(program, rd, rs1, $3.registerId);
        } else if ($3.type == CONSTANT) {
          genANDIInstruction(program, rd, rs1, $3.immediate);
        }
        $$ = registerExpressionValue(rd);
      }
    }
    | exp OR_OP exp
    {
      if ($1.type == CONSTANT && $3.type == CONSTANT) {
        $$ = constantExpressionValue($1.immediate | $3.immediate);
      } else {
        t_regID rd = getNewRegister(program);
        t_regID rs1 = genConvertExpValueToRegister(program, $1);
        if ($3.type == REGISTER) {
          genORInstruction(program, rd, rs1, $3.registerId);
        } else if ($3.type == CONSTANT) {
          genORIInstruction(program, rd, rs1, $3.immediate);
        }
        $$ = registerExpressionValue(rd);
      }
    }
    | exp PLUS exp
    {
      if ($1.type == CONSTANT && $3.type == CONSTANT) {
        $$ = constantExpressionValue($1.immediate + $3.immediate);
      } else {
        t_regID rd = getNewRegister(program);
        t_regID rs1 = genConvertExpValueToRegister(program, $1);
        if ($3.type == REGISTER) {
          genADDInstruction(program, rd, rs1, $3.registerId);
        } else if ($3.type == CONSTANT) {
          genADDIInstruction(program, rd, rs1, $3.immediate);
        }
        $$ = registerExpressionValue(rd);
      }
    }
    | exp MINUS exp
    {
      if ($1.type == CONSTANT && $3.type == CONSTANT) {
        $$ = constantExpressionValue($1.immediate - $3.immediate);
      } else {
        t_regID rd = getNewRegister(program);
        t_regID rs1 = genConvertExpValueToRegister(program, $1);
        if ($3.type == REGISTER) {
          genSUBInstruction(program, rd, rs1, $3.registerId);
        } else if ($3.type == CONSTANT) {
          genSUBIInstruction(program, rd, rs1, $3.immediate);
        }
        $$ = registerExpressionValue(rd);
      }
    }
    | exp MUL_OP exp
    {
      if ($1.type == CONSTANT && $3.type == CONSTANT) {
        $$ = constantExpressionValue($1.immediate * $3.immediate);
      } else {
        t_regID rd = getNewRegister(program);
        t_regID rs1 = genConvertExpValueToRegister(program, $1);
        if ($3.type == REGISTER) {
          genMULInstruction(program, rd, rs1, $3.registerId);
        } else if ($3.type == CONSTANT) {
          genMULIInstruction(program, rd, rs1, $3.immediate);
        }
        $$ = registerExpressionValue(rd);
      }
    }
    | exp DIV_OP exp
    {
      if ($1.type == CONSTANT && $3.type == CONSTANT) {
        if ($3.immediate == 0) {
          emitWarning(WARN_DIVISION_BY_ZERO);
          $$ = constantExpressionValue(INT_MAX);
        } else if ($1.immediate == INT_MIN && $3.immediate == -1) {
          emitWarning(WARN_OVERFLOW);
          $$ = constantExpressionValue(INT_MIN);
        } else {
          $$ = constantExpressionValue($1.immediate / $3.immediate);
        }
      } else {
        t_regID rd = getNewRegister(program);
        t_regID rs1 = genConvertExpValueToRegister(program, $1);
        if ($3.type == REGISTER) {
          genDIVInstruction(program, rd, rs1, $3.registerId);
        } else if ($3.type == CONSTANT) {
          genDIVIInstruction(program, rd, rs1, $3.immediate);
        }
        $$ = registerExpressionValue(rd);
      }
    }
    | exp LT exp
    {
      if ($1.type == CONSTANT && $3.type == CONSTANT) {
        $$ = constantExpressionValue($1.immediate < $3.immediate);
      } else {
        t_regID rd = getNewRegister(program);
        t_regID rs1 = genConvertExpValueToRegister(program, $1);
        if ($3.type == REGISTER) {
          genSLTInstruction(program, rd, rs1, $3.registerId);
        } else if ($3.type == CONSTANT) {
          genSLTIInstruction(program, rd, rs1, $3.immediate);
        }
        $$ = registerExpressionValue(rd);
      }
    }
    | exp GT exp
    {
      if ($1.type == CONSTANT && $3.type == CONSTANT) {
        $$ = constantExpressionValue($1.immediate > $3.immediate);
      } else {
        t_regID rd = getNewRegister(program);
        t_regID rs1 = genConvertExpValueToRegister(program, $1);
        if ($3.type == REGISTER) {
          genSGTInstruction(program, rd, rs1, $3.registerId);
        } else if ($3.type == CONSTANT) {
          genSGTIInstruction(program, rd, rs1, $3.immediate);
        }
        $$ = registerExpressionValue(rd);
      }
    }
    | exp EQ exp
    {
      if ($1.type == CONSTANT && $3.type == CONSTANT) {
        $$ = constantExpressionValue($1.immediate == $3.immediate);
      } else {
        t_regID rd = getNewRegister(program);
        t_regID rs1 = genConvertExpValueToRegister(program, $1);
        if ($3.type == REGISTER) {
          genSEQInstruction(program, rd, rs1, $3.registerId);
        } else if ($3.type == CONSTANT) {
          genSEQIInstruction(program, rd, rs1, $3.immediate);
        }
        $$ = registerExpressionValue(rd);
      }
    }
    | exp NOTEQ exp
    {
      if ($1.type == CONSTANT && $3.type == CONSTANT) {
        $$ = constantExpressionValue($1.immediate != $3.immediate);
      } else {
        t_regID rd = getNewRegister(program);
        t_regID rs1 = genConvertExpValueToRegister(program, $1);
        if ($3.type == REGISTER) {
          genSNEInstruction(program, rd, rs1, $3.registerId);
        } else if ($3.type == CONSTANT) {
          genSNEIInstruction(program, rd, rs1, $3.immediate);
        }
        $$ = registerExpressionValue(rd);
      }
    }
    | exp LTEQ exp
    {
      if ($1.type == CONSTANT && $3.type == CONSTANT) {
        $$ = constantExpressionValue($1.immediate <= $3.immediate);
      } else {
        t_regID rd = getNewRegister(program);
        t_regID rs1 = genConvertExpValueToRegister(program, $1);
        if ($3.type == REGISTER) {
          genSLEInstruction(program, rd, rs1, $3.registerId);
        } else if ($3.type == CONSTANT) {
          genSLEIInstruction(program, rd, rs1, $3.immediate);
        }
        $$ = registerExpressionValue(rd);
      }
    }
    | exp GTEQ exp
    {
      if ($1.type == CONSTANT && $3.type == CONSTANT) {
        $$ = constantExpressionValue($1.immediate >= $3.immediate);
      } else {
        t_regID rd = getNewRegister(program);
        t_regID rs1 = genConvertExpValueToRegister(program, $1);
        if ($3.type == REGISTER) {
          genSGEInstruction(program, rd, rs1, $3.registerId);
        } else if ($3.type == CONSTANT) {
          genSGEIInstruction(program, rd, rs1, $3.immediate);
        }
        $$ = registerExpressionValue(rd);
      }
    }
    | exp SHL_OP exp
    {
      if ($1.type == CONSTANT && $3.type == CONSTANT) {
        if ($3.immediate < 0 || $3.immediate >= 32) {
          emitWarning(WARN_INVALID_SHIFT_AMOUNT);
        }
        $$ = constantExpressionValue($1.immediate << ($3.immediate & 0x1F));
      } else {
        t_regID rd = getNewRegister(program);
        t_regID rs1 = genConvertExpValueToRegister(program, $1);
        if ($3.type == REGISTER) {
          genSLLInstruction(program, rd, rs1, $3.registerId);
        } else if ($3.type == CONSTANT) {
          genSLLIInstruction(program, rd, rs1, $3.immediate);
        }
        $$ = registerExpressionValue(rd);
      }
    }
    | exp SHR_OP exp
    {
      if ($1.type == CONSTANT && $3.type == CONSTANT) {
        if ($3.immediate < 0 || $3.immediate >= 32) {
          emitWarning(WARN_INVALID_SHIFT_AMOUNT);
        }
        int constRes = SHIFT_RIGHT_ARITH($1.immediate, $3.immediate & 0x1F);
        $$ = constantExpressionValue(constRes);
      } else {
        t_regID rd = getNewRegister(program);
        t_regID rs1 = genConvertExpValueToRegister(program, $1);
        if ($3.type == REGISTER) {
          genSRAInstruction(program, rd, rs1, $3.registerId);
        } else if ($3.type == CONSTANT) {
          genSRAIInstruction(program, rd, rs1, $3.immediate);
        }
        $$ = registerExpressionValue(rd);
      }
    }
    | exp ANDAND exp
    {
      if ($1.type == CONSTANT && $3.type == CONSTANT) {
        $$ = constantExpressionValue($1.immediate && $3.immediate);
      } else {
        t_regID rd = getNewRegister(program);
        t_expressionValue normLhs = genNormalizeBooleanExpValue(program, $1);
        t_expressionValue normRhs = genNormalizeBooleanExpValue(program, $3);
        t_regID rs1 = genConvertExpValueToRegister(program, normLhs);
        if (normRhs.type == REGISTER) {
          genANDInstruction(program, rd, rs1, normRhs.registerId);
        } else if (normRhs.type == CONSTANT) {
          genANDIInstruction(program, rd, rs1, normRhs.immediate);
        }
        $$ = registerExpressionValue(rd);
      }
    }
    | exp OROR exp
    {
      if ($1.type == CONSTANT && $3.type == CONSTANT) {
        $$ = constantExpressionValue($1.immediate || $3.immediate);
      } else {
        t_regID rd = getNewRegister(program);
        t_expressionValue normLhs = genNormalizeBooleanExpValue(program, $1);
        t_expressionValue normRhs = genNormalizeBooleanExpValue(program, $3);
        t_regID rs1 = genConvertExpValueToRegister(program, normLhs);
        if (normRhs.type == REGISTER) {
          genORInstruction(program, rd, rs1, normRhs.registerId);
        } else if (normRhs.type == CONSTANT) {
          genORIInstruction(program, rd, rs1, normRhs.immediate);
        }
        $$ = registerExpressionValue(rd);
      }
    }
    | LPAR exp RPAR
    {
      $$ = $2;
    }
;

var_id: IDENTIFIER
      {
        t_symbol *var = getSymbol(program, $1);
        if (var == NULL) {
          emitError(ERROR_VARIABLE_NOT_DECLARED);
          YYERROR;
        }
        $$ = var;
        free($1);
      }
;

%%
/******************************************************************************
 * Parser wrapper function
 ******************************************************************************/

int parseProgram(t_program *program)
{
  /* Initialize all the global variables */
  line_num = 1;
  num_error = 0;
  num_warning = 0;

  /* Open the input file */
  if (compilerOptions.inputFileName != NULL) {
    yyin = fopen(compilerOptions.inputFileName, "r");
    if (yyin == NULL)
      fatalError(ERROR_FOPEN_ERROR);
    debugPrintf(" -> Reading input from \"%s\"\n", compilerOptions.inputFileName);
  } else {
    yyin = stdin;
    debugPrintf(" -> Reading from standard input\n");
  }
  
  /* parse the program */
  yyparse();

  /* Check if an error occurred while parsing and in that case exit now */
  if (num_error > 0)
    return 1;

#ifndef NDEBUG
  char *logFileName = getLogFileName("frontend");
  debugPrintf(" -> Writing the output of parsing to \"%s\"\n", logFileName);
  FILE *logFile = fopen(logFileName, "w");
  dumpProgram(program, logFile);
  fclose(logFile);
  free(logFileName);
#endif

  /* do not attach a line number to the instructions generated by the
   * transformations that follow. */
  line_num = -1;
  return 0;
}

/******************************************************************************
 * MAIN
 ******************************************************************************/

int main(int argc, char *argv[])
{
  /* Read the options on the command line */
  int error = parseCompilerOptions(argc, argv);
  if (error != 0)
    return error;

  /* initialize the translation infos */
  program = newProgram();

  debugPrintf("Parsing the input program\n");
  error = parseProgram(program);

  if (error != 0) {
    /* Syntax errors have happened... */
    fprintf(stderr, "Input contains errors, no assembly file written.\n");
    fprintf(stderr, "%d error(s) found.\n", num_error);
  } else {
    debugPrintf("Lowering of pseudo-instructions to machine instructions.\n");
    doTargetSpecificTransformations(program);

    debugPrintf("Performing register allocation.\n");
    doRegisterAllocation(program);

    debugPrintf("Writing the assembly file.\n");
    writeAssembly(program);
  }
  
  debugPrintf("Finalizing the compiler data structures.\n");
  deleteProgram(program);

  debugPrintf("Done.\n");
  return 0;
}

/******************************************************************************
 * YYERROR
 ******************************************************************************/
void yyerror(const char* msg)
{
  emitSyntaxError(msg);
}
