#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include "parser.h"


typedef int t_parserError;
enum {
  P_ACCEPT = 1,
  P_REJECT = 0,
  P_SYN_ERROR = -1
};

typedef struct t_localLabel {
  struct t_localLabel *next;
  int identifier;
  t_objLabel *label;
} t_localLabel;

typedef struct t_parserState {
  t_lexer *lex;
  t_token *curToken;
  t_token *lookaheadToken;
  t_object *object;
  t_objSection *curSection;
  int numErrors;
  t_localLabel *backLabels;
  t_localLabel *forwardLabels;
} t_parserState;


static t_localLabel *parserGetLocalLabel(
    t_parserState *state, int identifier, bool back)
{
  t_localLabel *cur;
  if (back)
    cur = state->backLabels;
  else
    cur = state->forwardLabels;
  while (cur) {
    if (cur->identifier == identifier)
      break;
    cur = cur->next;
  }
  if (cur)
    return cur;
  if (back)
    return NULL;

  cur = calloc(1, sizeof(t_localLabel));
  cur->identifier = identifier;
  char realLblName[50];
  static int progressive = 0;
  snprintf(realLblName, 50, ".local_%d_%d", identifier, progressive++);
  cur->label = objGetLabel(state->object, realLblName);
  cur->next = state->forwardLabels;
  state->forwardLabels = cur;
  return cur;
}

static void parserDeclareLocalLabel(t_parserState *state, t_localLabel *label)
{
  t_localLabel *prev = NULL, *cur = state->forwardLabels;
  while (cur && cur != label) {
    prev = cur;
    cur = cur->next;
  }
  assert(cur != NULL && "attempted to declare a label which was never created");
  if (prev == NULL) {
    state->forwardLabels = label->next;
  } else {
    prev->next = label->next;
  }
  label->next = state->backLabels;
  state->backLabels = label;
  objSecDeclareLabel(state->curSection, label->label);
}

static void deleteLocalLabelList(t_localLabel *head)
{
  t_localLabel *next;
  while (head) {
    next = head->next;
    free(head);
    head = next;
  }
}


static void parserEmitError(t_parserState *state, const char *msg)
{
  if (!msg)
    msg = "unexpected token";
  int row = state->lookaheadToken->row + 1;
  int col = state->lookaheadToken->column;
  fprintf(stderr, "error at %d,%d: %s\n", row, col, msg);
  state->numErrors++;
}


static void parserNextToken(t_parserState *state)
{
  if (state->curToken && state->curToken->id == TOK_EOF) {
    fprintf(stderr, "BUG: trying to advance past EOF\n");
    abort();
  }
  deleteToken(state->curToken);
  state->curToken = state->lookaheadToken;
  state->lookaheadToken = lexNextToken(state->lex);
}

static t_parserError parserAccept(t_parserState *state, t_tokenID tok)
{
  if (state->lookaheadToken->id == tok) {
    parserNextToken(state);
    return P_ACCEPT;
  }
  return P_REJECT;
}

static t_parserError parserExpect(
    t_parserState *state, t_tokenID tok, const char *msg)
{
  t_parserError res = parserAccept(state, tok);
  if (res == P_ACCEPT)
    return res;
  parserEmitError(state, msg);
  return P_SYN_ERROR;
}


static t_parserError expectRegister(
    t_parserState *state, t_instrRegID *res, bool last)
{
  if (parserExpect(state, TOK_REGISTER, "expected a register") != P_ACCEPT)
    return P_SYN_ERROR;
  *res = state->curToken->value.reg;
  if (!last &&
      parserExpect(state, TOK_COMMA,
          "register name must be followed by a comma") != P_ACCEPT)
    return P_SYN_ERROR;
  return P_ACCEPT;
}

static t_parserError expectNumber(
    t_parserState *state, int32_t *res, int32_t min, int32_t max)
{
  if (state->lookaheadToken->id == TOK_NUMBER) {
    int32_t value = state->lookaheadToken->value.number;
    if (min <= value && value <= max) {
      parserNextToken(state);
      *res = value;
      return P_ACCEPT;
    } else {
      parserEmitError(state, "numeric constant out of bounds");
      return P_SYN_ERROR;
    }
  }
  parserEmitError(state, "expected a constant");
  return P_SYN_ERROR;
}

static t_parserError acceptLabel(t_parserState *state, t_instruction *instr)
{
  if (parserAccept(state, TOK_LOCAL_REF) == P_ACCEPT) {
    int n = state->curToken->value.localRef;
    bool back = n < 0;
    if (back)
      n = -n;
    t_localLabel *ll = parserGetLocalLabel(state, n, back);
    instr->label = ll->label;
    return P_ACCEPT;

  } else if (parserAccept(state, TOK_ID) == P_ACCEPT) {
    instr->label = objGetLabel(state->object, state->curToken->value.id);
    return P_ACCEPT;
  }

  return P_REJECT;
}

static t_parserError expectLabel(t_parserState *state, t_instruction *instr)
{
  if (acceptLabel(state, instr) != P_ACCEPT) {
    parserEmitError(state, "expected a label identifier");
    return P_SYN_ERROR;
  }
  return P_ACCEPT;
}

typedef int t_immSizeClass;
enum {
  IMM_SIZE_5,
  IMM_SIZE_12,
  IMM_SIZE_20
};

static t_parserError expectImmediate(
    t_parserState *state, t_instruction *instr, t_immSizeClass size)
{
  if (state->lookaheadToken->id == TOK_NUMBER) {
    instr->immMode = INSTR_IMM_CONST;
    int32_t min, max;
    if (size == IMM_SIZE_5) {
      min = 0;
      max = 31;
    } else if (size == IMM_SIZE_12) {
      min = -0x800;
      max = 0x7FF;
    } else if (size == IMM_SIZE_20) {
      min = -0x80000;
      max = 0xFFFFF;
    } else {
      assert(0 && "invalid immediate size");
    }
    return expectNumber(state, &instr->constant, min, max);
  }

  if (state->lookaheadToken->id == TOK_LO) {
    instr->immMode = INSTR_IMM_LBL_LO12;
  } else if (state->lookaheadToken->id == TOK_HI) {
    instr->immMode = INSTR_IMM_LBL_HI20;
  } else if (state->lookaheadToken->id == TOK_PCREL_LO) {
    instr->immMode = INSTR_IMM_LBL_PCREL_LO12;
  } else if (state->lookaheadToken->id == TOK_PCREL_HI) {
    instr->immMode = INSTR_IMM_LBL_PCREL_HI20;
  } else {
    parserEmitError(state, "expected valid immediate");
    return P_SYN_ERROR;
  }
  if (instr->immMode == INSTR_IMM_LBL_HI20 ||
      instr->immMode == INSTR_IMM_LBL_PCREL_HI20) {
    if (size < IMM_SIZE_20) {
      parserEmitError(state, "immediate too large");
      return P_SYN_ERROR;
    }
  } else {
    if (size < IMM_SIZE_12) {
      parserEmitError(state, "immediate too large");
      return P_SYN_ERROR;
    }
  }
  parserNextToken(state);

  if (parserExpect(state, TOK_LPAR, "expected left parenthesis") != P_ACCEPT)
    return P_SYN_ERROR;
  if (expectLabel(state, instr) != P_ACCEPT)
    return P_SYN_ERROR;
  if (parserExpect(state, TOK_RPAR, "expected right parenthesis") != P_ACCEPT)
    return P_SYN_ERROR;
  return P_ACCEPT;
}


typedef int t_instrFormat;
enum {
  FORMAT_OP,       // mnemonic rd, rs1, rs2
  FORMAT_OPIMM,    // mnemonic rd, rs1, imm
  FORMAT_LOAD,     // mnemonic rd, imm(rs1) / label
  FORMAT_STORE,    // mnemonic rs2, imm(rs1) / label, rd
  FORMAT_LUI,      // mnemonic rd, imm
  FORMAT_LI,       // mnemonic rd, number
  FORMAT_LA,       // mnemonic rd, label
  FORMAT_JAL,      // mnemonic rd, label
  FORMAT_JALR,     // mnemonic rs1, rs2, imm
  FORMAT_BRANCH,   // mnemonic rs1, rs2, label
  FORMAT_BRANCH_Z, // mnemonic rs1, label
  FORMAT_JUMP,     // mnemonic label
  FORMAT_SYSTEM    // mnemonic
};

static t_instrFormat instrOpcodeToFormat(t_instrOpcode opcode)
{
  switch (opcode) {
    case INSTR_OPC_ADD:
    case INSTR_OPC_SUB:
    case INSTR_OPC_AND:
    case INSTR_OPC_OR:
    case INSTR_OPC_XOR:
    case INSTR_OPC_MUL:
    case INSTR_OPC_MULH:
    case INSTR_OPC_MULHSU:
    case INSTR_OPC_MULHU:
    case INSTR_OPC_DIV:
    case INSTR_OPC_DIVU:
    case INSTR_OPC_REM:
    case INSTR_OPC_REMU:
    case INSTR_OPC_SLL:
    case INSTR_OPC_SRL:
    case INSTR_OPC_SRA:
    case INSTR_OPC_SLT:
    case INSTR_OPC_SLTU:
      return FORMAT_OP;
    case INSTR_OPC_ADDI:
    case INSTR_OPC_ANDI:
    case INSTR_OPC_ORI:
    case INSTR_OPC_XORI:
    case INSTR_OPC_SLLI:
    case INSTR_OPC_SRLI:
    case INSTR_OPC_SRAI:
    case INSTR_OPC_SLTI:
    case INSTR_OPC_SLTIU:
      return FORMAT_OPIMM;
    case INSTR_OPC_J:
      return FORMAT_JUMP;
    case INSTR_OPC_BEQ:
    case INSTR_OPC_BNE:
    case INSTR_OPC_BLT:
    case INSTR_OPC_BLTU:
    case INSTR_OPC_BGE:
    case INSTR_OPC_BGEU:
    case INSTR_OPC_BGT:
    case INSTR_OPC_BLE:
    case INSTR_OPC_BGTU:
    case INSTR_OPC_BLEU:
      return FORMAT_BRANCH;
    case INSTR_OPC_BEQZ:
    case INSTR_OPC_BNEZ:
    case INSTR_OPC_BLEZ:
    case INSTR_OPC_BGEZ:
    case INSTR_OPC_BLTZ:
    case INSTR_OPC_BGTZ:
      return FORMAT_BRANCH_Z;
    case INSTR_OPC_LB:
    case INSTR_OPC_LH:
    case INSTR_OPC_LW:
    case INSTR_OPC_LBU:
    case INSTR_OPC_LHU:
      return FORMAT_LOAD;
    case INSTR_OPC_SB:
    case INSTR_OPC_SH:
    case INSTR_OPC_SW:
      return FORMAT_STORE;
    case INSTR_OPC_LI:
      return FORMAT_LI;
    case INSTR_OPC_LA:
      return FORMAT_LA;
    case INSTR_OPC_LUI:
    case INSTR_OPC_AUIPC:
      return FORMAT_LUI;
    case INSTR_OPC_JAL:
      return FORMAT_JAL;
    case INSTR_OPC_JALR:
      return FORMAT_JALR;
    case INSTR_OPC_NOP:
    case INSTR_OPC_ECALL:
    case INSTR_OPC_EBREAK:
      return FORMAT_SYSTEM;
  }
  return -1;
}


static t_parserError expectInstruction(t_parserState *state)
{
  t_immSizeClass immSize;
  t_instruction instr = {0};

  parserExpect(state, TOK_MNEMONIC, NULL);
  instr.opcode = state->curToken->value.mnemonic;

  t_instrFormat format = instrOpcodeToFormat(instr.opcode);
  switch (format) {
    case FORMAT_OP:
      if (expectRegister(state, &instr.dest, false) != P_ACCEPT)
        return P_SYN_ERROR;
      if (expectRegister(state, &instr.src1, false) != P_ACCEPT)
        return P_SYN_ERROR;
      if (expectRegister(state, &instr.src2, true) != P_ACCEPT)
        return P_SYN_ERROR;
      break;

    case FORMAT_OPIMM:
      if (expectRegister(state, &instr.dest, false) != P_ACCEPT)
        return P_SYN_ERROR;
      if (expectRegister(state, &instr.src1, false) != P_ACCEPT)
        return P_SYN_ERROR;
      if (instr.opcode == INSTR_OPC_SLLI || instr.opcode == INSTR_OPC_SRLI ||
          instr.opcode == INSTR_OPC_SRAI) {
        immSize = IMM_SIZE_5;
      } else {
        immSize = IMM_SIZE_12;
      }
      if (expectImmediate(state, &instr, immSize) != P_ACCEPT)
        return P_SYN_ERROR;
      break;

    case FORMAT_LOAD:
      if (expectRegister(state, &instr.dest, false) != P_ACCEPT)
        return P_SYN_ERROR;
      if (acceptLabel(state, &instr) == P_ACCEPT) {
        instr.opcode = instr.opcode - INSTR_OPC_LB + INSTR_OPC_LB_G;
        instr.immMode = INSTR_IMM_LBL;
      } else {
        if (expectImmediate(state, &instr, IMM_SIZE_12) != P_ACCEPT)
          return P_SYN_ERROR;
        if (parserExpect(state, TOK_LPAR, "expected parenthesis") != P_ACCEPT)
          return P_SYN_ERROR;
        if (expectRegister(state, &instr.src1, true) != P_ACCEPT)
          return P_SYN_ERROR;
        if (parserExpect(state, TOK_RPAR, "expected parenthesis") != P_ACCEPT)
          return P_SYN_ERROR;
      }
      break;

    case FORMAT_STORE:
      if (expectRegister(state, &instr.src2, false) != P_ACCEPT)
        return P_SYN_ERROR;
      if (acceptLabel(state, &instr) == P_ACCEPT) {
        if (parserExpect(state, TOK_COMMA, "expected comma") != P_ACCEPT)
          return P_SYN_ERROR;
        if (expectRegister(state, &instr.dest, true) != P_ACCEPT)
          return P_SYN_ERROR;
        instr.opcode = instr.opcode - INSTR_OPC_SB + INSTR_OPC_SB_G;
        instr.immMode = INSTR_IMM_LBL;
      } else {
        if (expectImmediate(state, &instr, IMM_SIZE_12) != P_ACCEPT)
          return P_SYN_ERROR;
        if (parserExpect(state, TOK_LPAR, "expected parenthesis") != P_ACCEPT)
          return P_SYN_ERROR;
        if (expectRegister(state, &instr.src1, 1) != P_ACCEPT)
          return P_SYN_ERROR;
        if (parserExpect(state, TOK_RPAR, "expected parenthesis") != P_ACCEPT)
          return P_SYN_ERROR;
      }
      break;

    case FORMAT_LI:
      if (expectRegister(state, &instr.dest, false) != P_ACCEPT)
        return P_SYN_ERROR;
      if (expectNumber(state, &instr.constant, INT32_MIN, INT32_MAX) !=
          P_ACCEPT)
        return P_SYN_ERROR;
      break;

    case FORMAT_LUI:
      if (expectRegister(state, &instr.dest, false) != P_ACCEPT)
        return P_SYN_ERROR;
      if (expectImmediate(state, &instr, IMM_SIZE_20) != P_ACCEPT)
        return P_SYN_ERROR;
      break;

    case FORMAT_LA:
    case FORMAT_JAL:
      if (parserAccept(state, TOK_REGISTER) != P_ACCEPT) {
        instr.dest = 1; // RA (X1) register
      } else {
        instr.dest = state->curToken->value.reg;
        if (parserExpect(state, TOK_COMMA,
                "register name must be followed by a comma") != P_ACCEPT)
          return P_SYN_ERROR;
      }
      if (expectLabel(state, &instr) != P_ACCEPT)
        return P_SYN_ERROR;
      instr.immMode = INSTR_IMM_LBL;
      break;

    case FORMAT_JALR:
      if (expectRegister(state, &instr.dest, false) != P_ACCEPT)
        return P_SYN_ERROR;
      if (parserAccept(state, TOK_REGISTER) == P_ACCEPT) {
        instr.src1 = state->curToken->value.reg;
        if (parserExpect(state, TOK_COMMA,
                "register name must be followed by a comma") != P_ACCEPT)
          return P_SYN_ERROR;
        if (expectImmediate(state, &instr, IMM_SIZE_12) != P_ACCEPT)
          return P_SYN_ERROR;
      } else {
        if (expectImmediate(state, &instr, IMM_SIZE_12) != P_ACCEPT)
          return P_SYN_ERROR;
        if (parserExpect(state, TOK_LPAR, "expected parenthesis") != P_ACCEPT)
          return P_SYN_ERROR;
        if (expectRegister(state, &instr.src1, true) != P_ACCEPT)
          return P_SYN_ERROR;
        if (parserExpect(state, TOK_RPAR, "expected parenthesis") != P_ACCEPT)
          return P_SYN_ERROR;
      }
      break;

    case FORMAT_BRANCH:
      if (expectRegister(state, &instr.src1, false) != P_ACCEPT)
        return P_SYN_ERROR;
      if (expectRegister(state, &instr.src2, false) != P_ACCEPT)
        return P_SYN_ERROR;
      if (expectLabel(state, &instr) != P_ACCEPT)
        return P_SYN_ERROR;
      instr.immMode = INSTR_IMM_LBL;
      break;

    case FORMAT_BRANCH_Z:
      if (expectRegister(state, &instr.src1, false) != P_ACCEPT)
        return P_SYN_ERROR;
      if (expectLabel(state, &instr) != P_ACCEPT)
        return P_SYN_ERROR;
      instr.immMode = INSTR_IMM_LBL;
      break;

    case FORMAT_JUMP:
      if (expectLabel(state, &instr) != P_ACCEPT)
        return P_SYN_ERROR;
      instr.immMode = INSTR_IMM_LBL;
      break;

    case FORMAT_SYSTEM:
      break;

    default:
      return P_SYN_ERROR;
  }

  objSecAppendInstruction(state->curSection, instr);
  return P_ACCEPT;
}


/** Replace escape characters in a given string with their non-escaped
 * equivalent. The string is modified in-place. After the escape removal the
 * string is no longer null-terminated, the end pointer returned by the
 * function must be used to determine its length instead.
 * @param str The null terminated string where to remove escapes.
 * @returns The end pointer to the string buffer after removing the escapes. */
static char *performStringEscapes(char *in)
{
  char *out = in;
  bool stop = false;
  while (!stop) {
    char c = *in++;
    switch (c) {
      case '\0':
      case '\n':
      case '\r':
        stop = true;
        break;
      case '\\':
        c = *in++;
        switch (c) {
          case '\0':
            stop = true;
            break;
          case 'b':
            *out++ = '\b';
            break;
          case 'f':
            *out++ = '\f';
            break;
          case 'n':
            *out++ = '\n';
            break;
          case 'r':
            *out++ = '\r';
            break;
          case 't':
            *out++ = '\t';
            break;
          case 'v':
            *out++ = '\013';
            break;
          case '\\':
          case 'x':
          case 'X':
            c = (char)strtol(in, &in, 16);
            *out++ = c;
            break;
          default:
            if (c == '\'' || c == '"') {
              *out++ = c;
            } else if (isdigit(c)) {
              c = (char)strtol(in - 1, &in, 8);
              *out++ = c;
            } else {
              fprintf(stderr, "warning: invalid escape character in string\n");
              *out++ = c;
            }
            break;
        }
        break;
      default:
        if (c == '\0') {
          stop = true;
        } else {
          *out++ = c;
        }
        break;
    }
  }
  return out;
}

static t_parserError expectData(t_parserState *state)
{
  t_data data = {0};

  if (parserAccept(state, TOK_SPACE)) {
    if (parserExpect(state, TOK_NUMBER,
            "arguments to \".space\" must be numbers") != P_ACCEPT)
      return P_SYN_ERROR;
    data.dataSize = (uint32_t)state->curToken->value.number;
    data.initialized = false;
    objSecAppendData(state->curSection, data);
    return P_ACCEPT;
  }

  if (parserAccept(state, TOK_WORD) || parserAccept(state, TOK_HALF)) {
    size_t dataSize = state->curToken->id == TOK_WORD ? 4 : 2;
    do {
      if (state->lookaheadToken->id != TOK_NUMBER) {
        parserEmitError(
            state, "arguments to \".word\" or \".half\" must be numbers");
        return P_SYN_ERROR;
      }
      int32_t value = state->lookaheadToken->value.number;
      if ((dataSize == 2) && (value < -0x8000 || value > 0xFFFF)) {
        parserEmitError(state,
            "arguments to \".half\" must be numbers between -32768 and 65536");
        return P_SYN_ERROR;
      }
      parserNextToken(state);
      data.dataSize = dataSize;
      data.initialized = true;
      data.data[0] = (uint32_t)value & 0xFF;
      data.data[1] = ((uint32_t)value >> 8) & 0xFF;
      if (dataSize == 4) {
        data.data[2] = ((uint32_t)value >> 16) & 0xFF;
        data.data[3] = ((uint32_t)value >> 24) & 0xFF;
      }
      objSecAppendData(state->curSection, data);
    } while (parserAccept(state, TOK_COMMA));
    return P_ACCEPT;
  }

  if (parserAccept(state, TOK_BYTE)) {
    do {
      data.dataSize = sizeof(uint8_t);
      data.initialized = true;
      if (state->lookaheadToken->id == TOK_NUMBER) {
        int32_t value = state->lookaheadToken->value.number;
        if (value < -128 || value > 255) {
          parserEmitError(state,
              "numeric arguments to \".byte\" must be between -128 and 255");
          return P_SYN_ERROR;
        }
        data.data[0] = (uint8_t)value;
        parserNextToken(state);
      } else if (state->lookaheadToken->id == TOK_CHARACTER) {
        char *bufBegin = state->lookaheadToken->value.string;
        char *bufEnd = performStringEscapes(bufBegin);
        if (bufEnd - bufBegin != 1) {
          parserEmitError(state,
              "character arguments to \".byte\" must be representable in a "
              "single byte");
          return P_SYN_ERROR;
        }
        data.data[0] = (uint8_t)*bufBegin;
        parserNextToken(state);
      } else {
        parserEmitError(state,
            "arguments to \".byte\" must be number or character literals");
        return P_SYN_ERROR;
      }
      objSecAppendData(state->curSection, data);
    } while (parserAccept(state, TOK_COMMA));
    return P_ACCEPT;
  }

  if (parserAccept(state, TOK_ASCII)) {
    do {
      if (parserExpect(state, TOK_STRING,
              "arguments to \".ascii\" must be strings") != P_ACCEPT)
        return P_SYN_ERROR;
      char *bufBegin = state->curToken->value.string;
      char *bufEnd = performStringEscapes(bufBegin);
      data.dataSize = sizeof(char);
      data.initialized = true;
      for (char *p = bufBegin; p != bufEnd; p++) {
        data.data[0] = (uint8_t)*p;
        objSecAppendData(state->curSection, data);
      }
    } while (parserAccept(state, TOK_COMMA));
    return P_ACCEPT;
  }

  return P_SYN_ERROR;
}


static t_parserError expectAlign(t_parserState *state)
{
  t_alignData align = {0};

  t_tokenID alignType;
  if (parserAccept(state, TOK_ALIGN))
    alignType = TOK_ALIGN;
  else if (parserAccept(state, TOK_BALIGN))
    alignType = TOK_BALIGN;
  else
    return P_SYN_ERROR;

  if (state->lookaheadToken->id != TOK_NUMBER) {
    parserEmitError(state, "expected alignment amount");
    return P_SYN_ERROR;
  }
  int32_t amt = state->lookaheadToken->value.number;
  if (amt <= 0) {
    parserEmitError(
        state, "alignment amount must be a positive non-zero integer");
    return P_SYN_ERROR;
  }
  if (alignType == TOK_ALIGN && amt >= 32) {
    parserEmitError(state, "alignment amount too large");
    return P_SYN_ERROR;
  }
  parserNextToken(state);

  if (alignType == TOK_ALIGN)
    align.alignModulo = (size_t)1 << amt;
  else
    align.alignModulo = (size_t)amt;

  if (parserAccept(state, TOK_COMMA)) {
    int32_t pad;
    if (expectNumber(state, &pad, -128, 256) != P_ACCEPT)
      return P_SYN_ERROR;
    align.nopFill = false;
    align.fillByte = (uint8_t)pad;
  } else {
    if (objSecGetID(state->curSection) == OBJ_SECTION_TEXT) {
      if ((align.alignModulo % 4) != 0)
        fprintf(stderr,
            "warning at %d,%d: alignment in .text with an "
            "amount which is not a multiple of 4\n",
            state->curToken->row + 1, state->curToken->column);
      align.nopFill = true;
    } else {
      align.nopFill = false;
      align.fillByte = 0;
    }
  }

  objSecAppendAlignmentData(state->curSection, align);
  return P_ACCEPT;
}


static t_parserError expectLineContent(t_parserState *state)
{
  if (state->lookaheadToken->id == TOK_MNEMONIC)
    return expectInstruction(state);
  if (state->lookaheadToken->id == TOK_SPACE ||
      state->lookaheadToken->id == TOK_WORD ||
      state->lookaheadToken->id == TOK_HALF ||
      state->lookaheadToken->id == TOK_BYTE ||
      state->lookaheadToken->id == TOK_ASCII)
    return expectData(state);
  if (state->lookaheadToken->id == TOK_ALIGN ||
      state->lookaheadToken->id == TOK_BALIGN)
    return expectAlign(state);
  parserEmitError(state, "expected a data directive or an instruction");
  return P_SYN_ERROR;
}


static t_parserError expectLine(t_parserState *state)
{
  if (parserAccept(state, TOK_NEWLINE) == P_ACCEPT)
    return P_ACCEPT;

  if (parserAccept(state, TOK_TEXT) == P_ACCEPT) {
    state->curSection = objGetSection(state->object, OBJ_SECTION_TEXT);
    return parserExpect(state, TOK_NEWLINE, ".text does not take arguments");
  } else if (parserAccept(state, TOK_DATA) == P_ACCEPT) {
    state->curSection = objGetSection(state->object, OBJ_SECTION_DATA);
    return parserExpect(state, TOK_NEWLINE, ".data does not take arguments");
  }

  if (parserAccept(state, TOK_GLOBAL) == P_ACCEPT) {
    if (parserExpect(state, TOK_ID,
            ".global needs exactly one label argument") != P_ACCEPT)
      return P_SYN_ERROR;
    return parserExpect(
        state, TOK_NEWLINE, ".global cannot have more than one argument");
  }

  if (state->lookaheadToken->id == TOK_NUMBER) {
    int n = state->lookaheadToken->value.number;
    if (n < 0) {
      parserEmitError(state, "local labels must be positive numbers");
      return P_SYN_ERROR;
    }
    parserNextToken(state);
    if (parserExpect(state, TOK_COLON,
            "expected colon after number to define a local label") != P_ACCEPT)
      return P_SYN_ERROR;
    t_localLabel *ll = parserGetLocalLabel(state, n, 0);
    parserDeclareLocalLabel(state, ll);
  } else if (parserAccept(state, TOK_ID) == P_ACCEPT) {
    char *id = state->curToken->value.id;
    t_objLabel *label = objGetLabel(state->object, id);
    if (parserExpect(state, TOK_COLON,
            "label declaration without trailing comma") != P_ACCEPT)
      return P_SYN_ERROR;
    if (!objSecDeclareLabel(state->curSection, label))
      parserEmitError(state, "label already declared");
  }

  if (parserAccept(state, TOK_NEWLINE) == P_ACCEPT)
    return P_ACCEPT;
  if (expectLineContent(state) != P_ACCEPT)
    return P_SYN_ERROR;
  return parserExpect(state, TOK_NEWLINE, "expected end of the line");
}


t_object *parseObject(t_lexer *lex)
{
  t_parserState state;
  state.lex = lex;
  state.object = newObject();
  if (!state.object)
    goto error;
  state.curSection = objGetSection(state.object, OBJ_SECTION_TEXT);
  state.numErrors = 0;
  state.curToken = NULL;
  state.lookaheadToken = lexNextToken(lex);
  state.backLabels = NULL;
  state.forwardLabels = NULL;

  while (parserAccept(&state, TOK_EOF) != P_ACCEPT) {
    t_parserError err = expectLine(&state);
    if (err != P_ACCEPT) {
      if (state.numErrors > 10) {
        fprintf(stderr, "too many errors, aborting...\n");
        break;
      }
      // try to ignore the error and advance to the next line
      while (state.lookaheadToken->id != TOK_NEWLINE &&
          state.lookaheadToken->id != TOK_EOF) {
        parserNextToken(&state);
      }
    }
  }

  deleteToken(state.curToken);
  deleteToken(state.lookaheadToken);
  deleteLocalLabelList(state.backLabels);
  deleteLocalLabelList(state.forwardLabels);

  if (state.numErrors > 0)
    goto error;

  return state.object;
error:
  deleteObject(state.object);
  return NULL;
}
