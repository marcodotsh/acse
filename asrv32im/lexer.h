#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include "errors.h"
#include "object.h"

typedef int t_tokenID;
enum {
  TOK_UNRECOGNIZED = -1,
  TOK_EOF = 0,
  TOK_NEWLINE,
  TOK_ID,
  TOK_LOCAL_REF,
  TOK_NUMBER,
  TOK_CHARACTER,
  TOK_STRING,
  TOK_COMMA,
  TOK_COLON,
  TOK_LPAR,
  TOK_RPAR,
  TOK_REGISTER,
  TOK_TEXT,
  TOK_DATA,
  TOK_SPACE,
  TOK_WORD,
  TOK_HALF,
  TOK_BYTE,
  TOK_ASCII,
  TOK_ALIGN,
  TOK_BALIGN,
  TOK_GLOBAL,
  TOK_HI,
  TOK_LO,
  TOK_PCREL_HI,
  TOK_PCREL_LO,
  TOK_MNEMONIC
};

typedef struct {
  t_tokenID id;
  t_fileLocation location;
  const char *begin;
  const char *end;
  union {
    char *id;
    int32_t localRef;
    int32_t number;
    char *string;
    t_instrRegID reg;
    t_instrOpcode mnemonic;
  } value;
} t_token;

typedef struct t_lexer t_lexer;


t_lexer *newLexer(const char *fn);
void deleteLexer(t_lexer *lex);

t_token *lexNextToken(t_lexer *lex);
void deleteToken(t_token *tok);

#endif
