#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "lexer.h"


struct t_lexer {
  char *buf;
  size_t bufSize;
  char *nextTokenPtr;
  int row, column;
  char *lookahead;
};


static char *lexRangeToString(const char *begin, const char *end)
{
  size_t len = (size_t)(end - begin);
  char *res = malloc(len+1);
  memcpy(res, begin, len);
  res[len] = '\0';
  return res;
}


t_lexer *newLexer(FILE *fp)
{
  t_lexer *lex = calloc(1, sizeof(t_lexer));
  if (!lex)
    return NULL;
  
  fseek(fp, 0, SEEK_END);
  ssize_t fileSize = ftello(fp);
  if (fileSize < 0) {
    free(lex);
    return NULL;
  }
  fseek(fp, 0, SEEK_SET);

  if (fileSize == 0) {
    lex->buf = strdup("");
  } else {
    lex->buf = malloc(fileSize+1);
    if (!lex->buf) {
      free(lex);
      return NULL;
    } else {
      size_t readSz = fread(lex->buf, 1, fileSize, fp);
      lex->buf[readSz] = '\0';
      lex->bufSize = readSz;
    }
  }
  fclose(fp);

  lex->nextTokenPtr = lex->buf;
  lex->row = lex->column = 0;
  lex->lookahead = lex->buf;
  return lex;
}

void deleteLexer(t_lexer *lex)
{
  if (lex == NULL)
    return;
  free(lex->buf);
  free(lex);
}


static bool lexAcceptChar(t_lexer *lex, char c)
{
  if (*lex->lookahead == c) {
    lex->lookahead++;
    return true;
  }
  return false;
}

static bool lexAcceptString(t_lexer *lex, const char *pattern)
{
  int i;
  for (i = 0; pattern[i] != '\0'; i++) {
    if (lex->lookahead[i] != pattern[i])
      return false;
  }
  lex->lookahead += i;
  return true;
}

static char lexAcceptSet(t_lexer *lex, const char *set)
{
  for (int i = 0; set[i] != '\0'; i++) {
    if (*lex->lookahead == set[i]) {
      lex->lookahead++;
      return set[i];
    }
  }
  return '\0';
}

static bool lexAcceptNewline(t_lexer *lex)
{
  return lexAcceptString(lex, "\r\n") || lexAcceptString(lex, "\n");
}

static int lexAcceptIdentifier(t_lexer *lex)
{
  if (!(isalpha(*lex->lookahead) || *lex->lookahead == '_'))
    return 0;
  int n = 0;
  while (isalnum(*lex->lookahead) || *lex->lookahead == '_') {
    lex->lookahead++;
    n++;
  }
  return n;
}

static bool lexIdentEquals(t_lexer *lex, const char *str)
{
  char *p = lex->nextTokenPtr;
  char *end = lex->lookahead;
  while (p != end) {
    if (toupper(*p) != toupper(*str))
      return false;
    p++;
    str++;
  }
  return *str == '\0';
}


static void lexAdvance(t_lexer *lex)
{
  while (lex->nextTokenPtr != lex->lookahead) {
    char c = *lex->nextTokenPtr++;
    assert(c != '\0');
    if (c == '\r') {
      // ignore
    } else if (c == '\n') {
      lex->column = 0;
      lex->row++;
    } else {
      lex->column++;
    }
  }
}

static t_token *lexNewToken(t_lexer *lex, t_tokenID id)
{
  t_token *tok = calloc(1, sizeof(t_token));
  if (!tok)
    abort();
  
  tok->row = lex->row;
  tok->column = lex->column;
  tok->id = id;
  tok->begin = lex->nextTokenPtr;
  tok->end = lex->lookahead;
  lexAdvance(lex);
  return tok;
}

void deleteToken(t_token *tok)
{
  if (!tok)
    return;
  if (tok->id == TOK_ID)
    free(tok->value.id);
  else if (tok->id == TOK_STRING || tok->id == TOK_CHARACTER)
    free(tok->value.string);
}


static void lexSkipWhitespaceAndComments(t_lexer *lex)
{
  int state = 0;
  while (state != -1 && *lex->lookahead != '\0') {
    if (state == 0) {
      // normal whitespace
      if (!lexAcceptSet(lex, "\t ")) {
        // non-whitespace, might be a comment
        if (lexAcceptString(lex, "/*")) {
          // beginning of a C-style block comment
          state = 1;
        } else if (lexAcceptString(lex, "//")) {
          // beginning of a C++-style line comment
          state = 2;
        } else if (lexAcceptChar(lex, '#')) {
          // beginning of a RISC-V-style line comment
          state = 2;
        } else {
          // end of whitespace
          state = -1;
        }
      }

    } else if (state == 1) {
      // in a block comment
      if (lexAcceptString(lex, "*/")) {
        // end of the block comment
        state = 0;
      } else {
        // accept any character
        lex->lookahead++;
      }

    } else if (state == 2) {
      // in a line comment
      if (lexAcceptNewline(lex)) {
        // end of the line comment
        state = 0;
      } else {
        lex->lookahead++;
      }
    }
  }

  lexAdvance(lex);
}


static bool lexIsDigit(char c, int base)
{
  c = toupper(c);
  if (c < '0')
    return false;
  if (base <= 10)
    return (c - '0') < base;
  if ((c - '0') < 10)
    return true;
  if (c < 'A')
    return false;
  return (c - 'A' + 10) < base;
}

typedef enum {
  STRTOI_OK,
  STRTOI_OVERFLOW,
  STRTOI_NO_DIGITS
} t_strToIntErr;

static t_strToIntErr lexStringToU32(char **next, int base, uint32_t *res)
{
  if (!lexIsDigit(**next, base))
    return STRTOI_NO_DIGITS;
  
  uint32_t maxNoOfl = UINT32_MAX / base;
  uint32_t lastDigitMax = UINT32_MAX % base;
  *res = 0;
  while (*res <= maxNoOfl) {
    uint32_t digit;
    if ('0' <= **next && **next <= '9')
      digit = **next - '0';
    else
      digit = toupper(**next) - 'A' + 10;
    if (*res == maxNoOfl && digit > lastDigitMax)
      break;
    *res = (*res * base) + digit;
    (*next)++;
    if (!lexIsDigit(**next, base))
      break;
  }
  
  if (lexIsDigit(**next, base))
    return STRTOI_OVERFLOW;
  return STRTOI_OK;
}

static t_token *lexExpectNumberOrLocalRef(t_lexer *lex)
{
  bool negative = lexAcceptSet(lex, "+-") == '-';
  bool decimal = false;
  uint32_t value;
  t_strToIntErr error;

  if (lexAcceptString(lex, "0x")) {
    error = lexStringToU32(&lex->lookahead, 16, &value);
  } else if (lexAcceptString(lex, "0b")) {
    error = lexStringToU32(&lex->lookahead, 2, &value);
  } else if (*lex->lookahead == '0') {
    // We do not accept the leading zero right away because
    // our number may consist of that zero alone
    error = lexStringToU32(&lex->lookahead, 8, &value);
  } else {
    decimal = true;
    error = lexStringToU32(&lex->lookahead, 10, &value);
  }

  if (error == STRTOI_NO_DIGITS)
    return lexNewToken(lex, TOK_UNRECOGNIZED);
  if (error == STRTOI_OVERFLOW || (negative && value > 0x80000000)) {
    fprintf(stderr, "error at %d,%d: integer literal overflow\n", lex->row+1, lex->column+1);
    return lexNewToken(lex, TOK_UNRECOGNIZED);
  }

  char direction;
  if (decimal && !negative && (direction = lexAcceptSet(lex, "fb"))) {
    if (value > 0x7FFFFFFF) {
      fprintf(stderr, "error at %d,%d: local label ID too large\n", lex->row+1, lex->column+1);
      return lexNewToken(lex, TOK_UNRECOGNIZED);
    }
    t_token *res = lexNewToken(lex, TOK_LOCAL_REF);
    res->value.localRef = direction == 'b' ? -(int32_t)value : value;
    return res;
  }

  t_token *res = lexNewToken(lex, TOK_NUMBER);
  res->value.number = negative ? -(int32_t)value : value;
  return res;
}


static t_token *lexExpectCharacterOrString(t_lexer *lex)
{
  char delimiter = lexAcceptSet(lex, "'\"");
  if (!delimiter)
    return lexNewToken(lex, TOK_UNRECOGNIZED);
  bool badTerm = false;
  while (!lexAcceptChar(lex, delimiter)) {
    lexAcceptChar(lex, '\\');
    if (*lex->lookahead == '\n' || *lex->lookahead == '\r' || *lex->lookahead == '\0') {
      badTerm = true;
      break;
    }
    lex->lookahead++;
  }

  if (badTerm) {
    fprintf(stderr, "error at %d,%d: string not properly terminated\n", lex->row + 1, lex->column + 1);
    return lexNewToken(lex, TOK_UNRECOGNIZED);
  }
  t_token *res = lexNewToken(lex, delimiter == '\'' ? TOK_CHARACTER : TOK_STRING);
  res->value.string = lexRangeToString(res->begin+1, res->end-1);
  return res;
}


static t_token *lexExpectDirective(t_lexer *lex)
{
  if (!lexAcceptChar(lex, '.'))
    return lexNewToken(lex, TOK_UNRECOGNIZED);
  lexAcceptIdentifier(lex);

  if (lexIdentEquals(lex, ".text"))
    return lexNewToken(lex, TOK_TEXT);
  if (lexIdentEquals(lex, ".data"))
    return lexNewToken(lex, TOK_DATA);
  if (lexIdentEquals(lex, ".space"))
    return lexNewToken(lex, TOK_SPACE);
  if (lexIdentEquals(lex, ".word"))
    return lexNewToken(lex, TOK_WORD);
  if (lexIdentEquals(lex, ".half"))
    return lexNewToken(lex, TOK_HALF);
  if (lexIdentEquals(lex, ".byte"))
    return lexNewToken(lex, TOK_BYTE);
  if (lexIdentEquals(lex, ".ascii"))
    return lexNewToken(lex, TOK_ASCII);
  if (lexIdentEquals(lex, ".align"))
    return lexNewToken(lex, TOK_ALIGN);
  if (lexIdentEquals(lex, ".balign"))
    return lexNewToken(lex, TOK_BALIGN);
  if (lexIdentEquals(lex, ".global"))
    return lexNewToken(lex, TOK_GLOBAL);
  
  return lexNewToken(lex, TOK_UNRECOGNIZED);
}


static t_token *lexExpectAddressing(t_lexer *lex)
{
  if (!lexAcceptChar(lex, '%'))
    return lexNewToken(lex, TOK_UNRECOGNIZED);
  lexAcceptIdentifier(lex);

  if (lexIdentEquals(lex, "%hi"))
    return lexNewToken(lex, TOK_HI);
  if (lexIdentEquals(lex, "%lo"))
    return lexNewToken(lex, TOK_LO);
  if (lexIdentEquals(lex, "%pcrel_hi"))
    return lexNewToken(lex, TOK_PCREL_HI);
  if (lexIdentEquals(lex, "%pcrel_lo"))
    return lexNewToken(lex, TOK_PCREL_LO);
  
  return lexNewToken(lex, TOK_UNRECOGNIZED);
}


typedef struct t_keywordData {
  const char *text;
  t_tokenID id;
  int32_t info;
} t_keywordData;

static t_token *lexExpectIdentifierOrKeyword(t_lexer *lex)
{
  static const t_keywordData kwdata[] = {
      {"x0",     TOK_REGISTER, 0},
      {"x1",     TOK_REGISTER, 1},
      {"x2",     TOK_REGISTER, 2},
      {"x3",     TOK_REGISTER, 3},
      {"x4",     TOK_REGISTER, 4},
      {"x5",     TOK_REGISTER, 5},
      {"x6",     TOK_REGISTER, 6},
      {"x7",     TOK_REGISTER, 7},
      {"x8",     TOK_REGISTER, 8},
      {"x8",     TOK_REGISTER, 8},
      {"x9",     TOK_REGISTER, 9},
      {"x10",    TOK_REGISTER, 10},
      {"x11",    TOK_REGISTER, 11},
      {"x12",    TOK_REGISTER, 12},
      {"x13",    TOK_REGISTER, 13},
      {"x14",    TOK_REGISTER, 14},
      {"x15",    TOK_REGISTER, 15},
      {"x16",    TOK_REGISTER, 16},
      {"x17",    TOK_REGISTER, 17},
      {"x18",    TOK_REGISTER, 18},
      {"x19",    TOK_REGISTER, 19},
      {"x20",    TOK_REGISTER, 20},
      {"x21",    TOK_REGISTER, 21},
      {"x22",    TOK_REGISTER, 22},
      {"x23",    TOK_REGISTER, 23},
      {"x24",    TOK_REGISTER, 24},
      {"x25",    TOK_REGISTER, 25},
      {"x26",    TOK_REGISTER, 26},
      {"x27",    TOK_REGISTER, 27},
      {"x28",    TOK_REGISTER, 28},
      {"x29",    TOK_REGISTER, 29},
      {"x30",    TOK_REGISTER, 30},
      {"x31",    TOK_REGISTER, 31},
      {"zero",   TOK_REGISTER, 0},
      {"ra",     TOK_REGISTER, 1},
      {"sp",     TOK_REGISTER, 2},
      {"gp",     TOK_REGISTER, 3},
      {"tp",     TOK_REGISTER, 4},
      {"t0",     TOK_REGISTER, 5},
      {"t1",     TOK_REGISTER, 6},
      {"t2",     TOK_REGISTER, 7},
      {"s0",     TOK_REGISTER, 8},
      {"fp",     TOK_REGISTER, 8},
      {"s1",     TOK_REGISTER, 9},
      {"a0",     TOK_REGISTER, 10},
      {"a1",     TOK_REGISTER, 11},
      {"a2",     TOK_REGISTER, 12},
      {"a3",     TOK_REGISTER, 13},
      {"a4",     TOK_REGISTER, 14},
      {"a5",     TOK_REGISTER, 15},
      {"a6",     TOK_REGISTER, 16},
      {"a7",     TOK_REGISTER, 17},
      {"s2",     TOK_REGISTER, 18},
      {"s3",     TOK_REGISTER, 19},
      {"s4",     TOK_REGISTER, 20},
      {"s5",     TOK_REGISTER, 21},
      {"s6",     TOK_REGISTER, 22},
      {"s7",     TOK_REGISTER, 23},
      {"s8",     TOK_REGISTER, 24},
      {"s9",     TOK_REGISTER, 25},
      {"s10",    TOK_REGISTER, 26},
      {"s11",    TOK_REGISTER, 27},
      {"t3",     TOK_REGISTER, 28},
      {"t4",     TOK_REGISTER, 29},
      {"t5",     TOK_REGISTER, 30},
      {"t6",     TOK_REGISTER, 31},
      {"add",    TOK_MNEMONIC, INSTR_OPC_ADD},
      {"sub",    TOK_MNEMONIC, INSTR_OPC_SUB},
      {"xor",    TOK_MNEMONIC, INSTR_OPC_XOR},
      {"or",     TOK_MNEMONIC, INSTR_OPC_OR},
      {"and",    TOK_MNEMONIC, INSTR_OPC_AND},
      {"sll",    TOK_MNEMONIC, INSTR_OPC_SLL},
      {"srl",    TOK_MNEMONIC, INSTR_OPC_SRL},
      {"sra",    TOK_MNEMONIC, INSTR_OPC_SRA},
      {"slt",    TOK_MNEMONIC, INSTR_OPC_SLT},
      {"sltu",   TOK_MNEMONIC, INSTR_OPC_SLTU},
      {"mul",    TOK_MNEMONIC, INSTR_OPC_MUL},
      {"mulh",   TOK_MNEMONIC, INSTR_OPC_MULH},
      {"mulhsu", TOK_MNEMONIC, INSTR_OPC_MULHSU},
      {"mulhu",  TOK_MNEMONIC, INSTR_OPC_MULHU},
      {"div",    TOK_MNEMONIC, INSTR_OPC_DIV},
      {"divu",   TOK_MNEMONIC, INSTR_OPC_DIVU},
      {"rem",    TOK_MNEMONIC, INSTR_OPC_REM},
      {"remu",   TOK_MNEMONIC, INSTR_OPC_REMU},
      {"addi",   TOK_MNEMONIC, INSTR_OPC_ADDI},
      {"xori",   TOK_MNEMONIC, INSTR_OPC_XORI},
      {"ori",    TOK_MNEMONIC, INSTR_OPC_ORI},
      {"andi",   TOK_MNEMONIC, INSTR_OPC_ANDI},
      {"slli",   TOK_MNEMONIC, INSTR_OPC_SLLI},
      {"srli",   TOK_MNEMONIC, INSTR_OPC_SRLI},
      {"srai",   TOK_MNEMONIC, INSTR_OPC_SRAI},
      {"slti",   TOK_MNEMONIC, INSTR_OPC_SLTI},
      {"sltiu",  TOK_MNEMONIC, INSTR_OPC_SLTIU},
      {"lb",     TOK_MNEMONIC, INSTR_OPC_LB},
      {"lh",     TOK_MNEMONIC, INSTR_OPC_LH},
      {"lw",     TOK_MNEMONIC, INSTR_OPC_LW},
      {"lbu",    TOK_MNEMONIC, INSTR_OPC_LBU},
      {"lhu",    TOK_MNEMONIC, INSTR_OPC_LHU},
      {"sb",     TOK_MNEMONIC, INSTR_OPC_SB},
      {"sh",     TOK_MNEMONIC, INSTR_OPC_SH},
      {"sw",     TOK_MNEMONIC, INSTR_OPC_SW},
      {"nop",    TOK_MNEMONIC, INSTR_OPC_NOP},
      {"ecall",  TOK_MNEMONIC, INSTR_OPC_ECALL},
      {"ebreak", TOK_MNEMONIC, INSTR_OPC_EBREAK},
      {"lui",    TOK_MNEMONIC, INSTR_OPC_LUI},
      {"auipc",  TOK_MNEMONIC, INSTR_OPC_AUIPC},
      {"jal",    TOK_MNEMONIC, INSTR_OPC_JAL},
      {"jalr",   TOK_MNEMONIC, INSTR_OPC_JALR},
      {"beq",    TOK_MNEMONIC, INSTR_OPC_BEQ},
      {"bne",    TOK_MNEMONIC, INSTR_OPC_BNE},
      {"blt",    TOK_MNEMONIC, INSTR_OPC_BLT},
      {"bge",    TOK_MNEMONIC, INSTR_OPC_BGE},
      {"bltu",   TOK_MNEMONIC, INSTR_OPC_BLTU},
      {"bgeu",   TOK_MNEMONIC, INSTR_OPC_BGEU},
      {"li",     TOK_MNEMONIC, INSTR_OPC_LI},
      {"la",     TOK_MNEMONIC, INSTR_OPC_LA},
      {"j",      TOK_MNEMONIC, INSTR_OPC_J},
      {"bgt",    TOK_MNEMONIC, INSTR_OPC_BGT},
      {"ble",    TOK_MNEMONIC, INSTR_OPC_BLE},
      {"bgtu",   TOK_MNEMONIC, INSTR_OPC_BGTU},
      {"bleu",   TOK_MNEMONIC, INSTR_OPC_BLEU},
      {"beqz",   TOK_MNEMONIC, INSTR_OPC_BEQZ},
      {"bnez",   TOK_MNEMONIC, INSTR_OPC_BNEZ},
      {"blez",   TOK_MNEMONIC, INSTR_OPC_BLEZ},
      {"bgez",   TOK_MNEMONIC, INSTR_OPC_BGEZ},
      {"bltz",   TOK_MNEMONIC, INSTR_OPC_BLTZ},
      {"bgtz",   TOK_MNEMONIC, INSTR_OPC_BGTZ},
      {NULL,     TOK_UNRECOGNIZED, 0}
  };

  lexAcceptIdentifier(lex);
  for (int i = 0; kwdata[i].text != NULL; i++) {
    if (lexIdentEquals(lex, kwdata[i].text)) {
      t_token *res = lexNewToken(lex, kwdata[i].id);
      if (kwdata[i].id == TOK_REGISTER)
        res->value.reg = kwdata[i].info;
      else if (kwdata[i].id == TOK_MNEMONIC)
        res->value.mnemonic = kwdata[i].info;
      else
        assert(0 && "bad keyword data table");
      return res;
    }
  }

  t_token *res = lexNewToken(lex, TOK_ID);
  res->value.id = lexRangeToString(res->begin, res->end);
  return res;
}


t_token *lexNextToken(t_lexer *lex)
{
  lexSkipWhitespaceAndComments(lex);

  if (*lex->lookahead == '\0') {
    if (lex->lookahead != (lex->buf + lex->bufSize)) {
      fprintf(stderr, "error at %d,%d: null character in input\n", lex->row+1, lex->column+1);
      return lexNewToken(lex, TOK_UNRECOGNIZED);
    }
    return lexNewToken(lex, TOK_EOF);
  }

  if (lexAcceptNewline(lex))
    return lexNewToken(lex, TOK_NEWLINE);
  if (lexAcceptChar(lex, ';'))
    return lexNewToken(lex, TOK_NEWLINE);

  if (lexAcceptChar(lex, ','))
    return lexNewToken(lex, TOK_COMMA);
  if (lexAcceptChar(lex, ':'))
    return lexNewToken(lex, TOK_COLON);
  if (lexAcceptChar(lex, '('))
    return lexNewToken(lex, TOK_LPAR);
  if (lexAcceptChar(lex, ')'))
    return lexNewToken(lex, TOK_RPAR);

  if (isdigit(*lex->lookahead) || *lex->lookahead == '-' || *lex->lookahead == '+')
    return lexExpectNumberOrLocalRef(lex);
  if (*lex->lookahead == '"' || *lex->lookahead == '\'')
    return lexExpectCharacterOrString(lex);
  if (*lex->lookahead == '.')
    return lexExpectDirective(lex);
  if (*lex->lookahead == '%')
    return lexExpectAddressing(lex);
  if (isalpha(*lex->lookahead) || *lex->lookahead == '_')
    return lexExpectIdentifierOrKeyword(lex);

  return lexNewToken(lex, TOK_UNRECOGNIZED);
}
