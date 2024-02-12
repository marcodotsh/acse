/** @file scanner.l
 ** @brief The Flex file describing the tokens used by the LANCE language. */

%{
#include <string.h>
#include "utils.h"
#include "list.h"
#include "symbols.h"
#include "parser.h"
#include "parser.tab.h"

%}

%option noyywrap
%x comment

/******************************************************************************
 * DEFINITIONS
 ******************************************************************************/
DIGIT                     [0-9]
ID                        [a-zA-Z_][a-zA-Z0-9_]*

/******************************************************************************
 * RULES
 ******************************************************************************/
%%

"\r\n"                    { ++line_num; }
"\n"                      { ++line_num; }

[ \t\f\v]+                { /* Ignore whitespace. */ }

"//"[^\n]*                { /* ignore comment lines */ }
"/*"                      BEGIN(comment);

<comment>[^*\n]*
<comment>[^*\n]*\n        { ++line_num; }
<comment>"*"+[^*/\n]*     
<comment>"*"+[^*/\n]*\n   { ++line_num; }
<comment>"*"+"/"          BEGIN(INITIAL);

"{"               { return LBRACE; }
"}"               { return RBRACE; }
"["               { return LSQUARE; }
"]"               { return RSQUARE; }
"("               { return LPAR; }
")"               { return RPAR; }
";"               { return SEMI; }
"+"               { return PLUS; }
"-"               { return MINUS; }
"*"               { return MUL_OP; }
"/"               { return DIV_OP; }
"&"               { return AND_OP; }
"|"               { return OR_OP; }
"!"               { return NOT_OP; }
"="               { return ASSIGN; }
"<"               { return LT; }
">"               { return GT; }
"<<"              { return SHL_OP; }
">>"              { return SHR_OP; }
"=="              { return EQ; }
"!="              { return NOTEQ; }
"<="              { return LTEQ; }
">="              { return GTEQ; }
"&&"              { return ANDAND; }
"||"              { return OROR; }
","               { return COMMA; }

"do"              { return DO; }
"else"            { return ELSE; }
"if"              { return IF; }
"int"             { return TYPE; }
"while"           { return WHILE; }
"return"          { return RETURN; }
"read"            { return READ; }
"write"           { return WRITE; }

{ID}              { yylval.string = strdup(yytext); return IDENTIFIER; }
{DIGIT}+          { yylval.integer = atoi(yytext);
                    return NUMBER; }

.                 { yyerror("unexpected token");
                    return -1; /* invalid token */ }
<INITIAL><<EOF>>  { return EOF_TOK; }
