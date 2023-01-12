#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
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
  t_object *object;
  t_objSection *curSection;
  int numErrors;
  int hasLastToken;
  t_tokenID lastToken;
  t_localLabel *backLabels;
  t_localLabel *forwardLabels;
} t_parserState;


static t_localLabel *parserGetLocalLabel(
    t_parserState *state, int identifier, int back)
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
  int r, c;

  if (!msg)
    msg = "unexpected token";
  r = lexGetLastTokenRow(state->lex) + 1;
  c = lexGetLastTokenColumn(state->lex);
  fprintf(stderr, "error at %d,%d: %s\n", r, c, msg);
  state->numErrors++;
}


static t_tokenID parserPeekToken(t_parserState *state)
{
  if (state->hasLastToken)
    return state->lastToken;
  state->lastToken = lexNextToken(state->lex);
  state->hasLastToken = 1;
  return state->lastToken;
}

static t_tokenID parserConsumeToken(t_parserState *state)
{
  t_tokenID res;

  if (!state->hasLastToken) {
    res = lexNextToken(state->lex);
  } else {
    res = state->lastToken;
  }
  state->hasLastToken = 0;
  return res;
}


static t_parserError parserAccept(t_parserState *state, t_tokenID tok)
{
  t_tokenID nextTok;

  nextTok = parserPeekToken(state);
  if (nextTok == tok) {
    parserConsumeToken(state);
    return P_ACCEPT;
  }
  return P_REJECT;
}

static t_parserError parserExpect(
    t_parserState *state, t_tokenID tok, const char *msg)
{
  int res;

  res = parserAccept(state, tok);
  if (res == P_ACCEPT)
    return res;
  parserEmitError(state, msg);
  return P_SYN_ERROR;
}

static t_parserError expectRegister(
    t_parserState *state, t_instrRegID *res, int last)
{
  if (parserExpect(state, TOK_REGISTER, "expected a register") != P_ACCEPT)
    return P_SYN_ERROR;
  *res = lexGetLastRegisterID(state->lex);
  if (!last &&
      parserExpect(state, TOK_COMMA,
          "register name must be followed by a comma") != P_ACCEPT)
    return P_SYN_ERROR;
  return P_ACCEPT;
}

static t_parserError expectNumber(
    t_parserState *state, int32_t *res, int32_t min, int32_t max)
{
  int32_t tmp;

  if (parserExpect(state, TOK_NUMBER, "expected a constant") != P_ACCEPT)
    return P_SYN_ERROR;

  tmp = lexGetLastNumberValue(state->lex);
  if (tmp < min || tmp > max) {
    parserEmitError(state, "numeric constant out of bounds");
    return P_SYN_ERROR;
  }
  *res = tmp;
  return P_ACCEPT;
}

static t_parserError acceptLabel(t_parserState *state, t_instruction *instr)
{
  if (parserAccept(state, TOK_LOCAL_REF) == P_ACCEPT) {
    int n = lexGetLastNumberValue(state->lex);
    int back = n < 0;
    if (back)
      n = -n;
    t_localLabel *ll = parserGetLocalLabel(state, n, back);
    instr->label = ll->label;
    return P_ACCEPT;

  } else if (parserAccept(state, TOK_ID) == P_ACCEPT) {
    char *tmp = lexGetLastTokenText(state->lex);
    instr->label = objGetLabel(state->object, tmp);
    free(tmp);
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
  int32_t min, max;

  if (parserAccept(state, TOK_LO) == P_ACCEPT) {
    instr->immMode = INSTR_IMM_LBL_LO12;
  } else if (parserAccept(state, TOK_HI) == P_ACCEPT) {
    instr->immMode = INSTR_IMM_LBL_HI20;
  } else if (parserAccept(state, TOK_PCREL_LO) == P_ACCEPT) {
    instr->immMode = INSTR_IMM_LBL_PCREL_LO12;
  } else if (parserAccept(state, TOK_PCREL_HI) == P_ACCEPT) {
    instr->immMode = INSTR_IMM_LBL_PCREL_HI20;
  } else {
    instr->immMode = INSTR_IMM_CONST;
  }

  if (instr->immMode == INSTR_IMM_CONST) {
    if (size == IMM_SIZE_5) {
      min = 0;
      max = 31;
    } else if (size == IMM_SIZE_12) {
      min = -0x800;
      max = 0x7FF;
    } else if (size == IMM_SIZE_20) {
      min = -0x80000;
      max = 0xFFFFF;
    } else
      assert(0 && "invalid immediate size");

    return expectNumber(state, &instr->constant, min, max);
  }

  if (size < IMM_SIZE_12) {
    parserEmitError(state, "immediate too large");
    return P_SYN_ERROR;
  }
  if (instr->immMode == INSTR_IMM_LBL_HI20 ||
      instr->immMode == INSTR_IMM_LBL_PCREL_HI20) {
    if (size < IMM_SIZE_20) {
      parserEmitError(state, "immediate too large");
      return P_SYN_ERROR;
    }
  }

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
  FORMAT_OP,       /* mnemonic rd, rs1, rs2               */
  FORMAT_OPIMM,    /* mnemonic rd, rs1, imm               */
  FORMAT_LOAD,     /* mnemonic rd, imm(rs1) / label       */
  FORMAT_STORE,    /* mnemonic rs2, imm(rs1) / label, rd  */
  FORMAT_LUI,      /* mnemonic rd, imm                    */
  FORMAT_LI,       /* mnemonic rd, number                 */
  FORMAT_LA,       /* mnemonic rd, label                  */
  FORMAT_JAL,      /* mnemonic rd, label                  */
  FORMAT_JALR,     /* mnemonic rs1, rs2, imm              */
  FORMAT_BRANCH,   /* mnemonic rs1, rs2, label            */
  FORMAT_BRANCH_Z, /* mnemonic rs1, label                 */
  FORMAT_JUMP,     /* mnemonic label                      */
  FORMAT_SYSTEM    /* mnemonic                            */
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


static t_parserError expectInstruction(
    t_parserState *state, t_tokenID lastToken)
{
  t_instrFormat format;
  t_immSizeClass immSize;
  t_parserError err;
  t_instruction instr = {0};

  if (lastToken != TOK_MNEMONIC)
    return P_SYN_ERROR;
  instr.opcode = lexGetLastMnemonicOpcode(state->lex);

  format = instrOpcodeToFormat(instr.opcode);
  switch (format) {
    case FORMAT_OP:
      if (expectRegister(state, &instr.dest, 0) != P_ACCEPT)
        return P_SYN_ERROR;
      if (expectRegister(state, &instr.src1, 0) != P_ACCEPT)
        return P_SYN_ERROR;
      if (expectRegister(state, &instr.src2, 1) != P_ACCEPT)
        return P_SYN_ERROR;
      break;

    case FORMAT_OPIMM:
      if (expectRegister(state, &instr.dest, 0) != P_ACCEPT)
        return P_SYN_ERROR;
      if (expectRegister(state, &instr.src1, 0) != P_ACCEPT)
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
      if (expectRegister(state, &instr.dest, 0) != P_ACCEPT)
        return P_SYN_ERROR;
      if (acceptLabel(state, &instr) == P_ACCEPT) {
        instr.opcode = instr.opcode - INSTR_OPC_LB + INSTR_OPC_LB_G;
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

    case FORMAT_STORE:
      if (expectRegister(state, &instr.src2, 0) != P_ACCEPT)
        return P_SYN_ERROR;
      if (acceptLabel(state, &instr) == P_ACCEPT) {
        if (parserExpect(state, TOK_COMMA, "expected comma") != P_ACCEPT)
          return P_SYN_ERROR;
        if (expectRegister(state, &instr.dest, 1) != P_ACCEPT)
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
      if (expectRegister(state, &instr.dest, 0) != P_ACCEPT)
        return P_SYN_ERROR;
      if (expectNumber(state, &instr.constant, INT32_MIN, INT32_MAX) !=
          P_ACCEPT)
        return P_SYN_ERROR;
      break;

    case FORMAT_LUI:
      if (expectRegister(state, &instr.dest, 0) != P_ACCEPT)
        return P_SYN_ERROR;
      if (expectImmediate(state, &instr, IMM_SIZE_20) != P_ACCEPT)
        return P_SYN_ERROR;
      break;

    case FORMAT_LA:
    case FORMAT_JAL:
      if (parserAccept(state, TOK_REGISTER) != P_ACCEPT) {
        instr.dest = 1;
      } else {
        instr.dest = lexGetLastRegisterID(state->lex);
        if (parserExpect(state, TOK_COMMA,
                "register name must be followed by a comma") != P_ACCEPT)
          return P_SYN_ERROR;
      }
      if (expectLabel(state, &instr) != P_ACCEPT)
        return P_SYN_ERROR;
      instr.immMode = INSTR_IMM_LBL;
      break;

    case FORMAT_JALR:
      if (expectRegister(state, &instr.dest, 0) != P_ACCEPT)
        return P_SYN_ERROR;
      if (parserAccept(state, TOK_REGISTER) == P_ACCEPT) {
        instr.src1 = lexGetLastRegisterID(state->lex);
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
        if (expectRegister(state, &instr.src1, 1) != P_ACCEPT)
          return P_SYN_ERROR;
        if (parserExpect(state, TOK_RPAR, "expected parenthesis") != P_ACCEPT)
          return P_SYN_ERROR;
      }
      break;

    case FORMAT_BRANCH:
      if (expectRegister(state, &instr.src1, 0) != P_ACCEPT)
        return P_SYN_ERROR;
      if (expectRegister(state, &instr.src2, 0) != P_ACCEPT)
        return P_SYN_ERROR;
      if (expectLabel(state, &instr) != P_ACCEPT)
        return P_SYN_ERROR;
      instr.immMode = INSTR_IMM_LBL;
      break;

    case FORMAT_BRANCH_Z:
      if (expectRegister(state, &instr.src1, 0) != P_ACCEPT)
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


static t_parserError expectData(t_parserState *state, t_tokenID lastToken)
{
  int32_t temp;
  t_data data = {0};

  if (lastToken == TOK_SPACE) {
    if (!parserExpect(state, TOK_NUMBER, "expected number after \".space\""))
      return P_SYN_ERROR;
    data.dataSize = lexGetLastNumberValue(state->lex);
    data.initialized = 0;
    objSecAppendData(state->curSection, data);
    return P_ACCEPT;
  }

  if (lastToken == TOK_WORD || lastToken == TOK_HALF) {
    do {
      if (!parserExpect(state, TOK_NUMBER, "expected number"))
        return P_SYN_ERROR;
      temp = lexGetLastNumberValue(state->lex);
      if (lastToken == TOK_WORD)
        data.dataSize = 4;
      else if (lastToken == TOK_HALF)
        data.dataSize = 2;
      data.initialized = 1;
      data.data[0] = temp & 0xFF;
      data.data[1] = (temp >> 8) & 0xFF;
      if (lastToken == TOK_WORD) {
        data.data[2] = (temp >> 16) & 0xFF;
        data.data[3] = (temp >> 24) & 0xFF;
      }
      objSecAppendData(state->curSection, data);
    } while (parserAccept(state, TOK_COMMA));
    return P_ACCEPT;
  }

  if (lastToken == TOK_BYTE) {
    do {
      data.dataSize = sizeof(uint8_t);
      data.initialized = 1;
      if (parserAccept(state, TOK_NUMBER)) {
        data.data[0] = lexGetLastNumberValue(state->lex);
      } else if (parserAccept(state, TOK_CHARACTER)) {
        char *str = lexGetLastStringValue(state->lex);
        data.data[0] = *str;
      } else {
        parserEmitError(state, "expected numeric or character constant");
        return P_SYN_ERROR;
      }
      objSecAppendData(state->curSection, data);
    } while (parserAccept(state, TOK_COMMA));
    return P_ACCEPT;
  }

  if (lastToken == TOK_ASCII) {
    do {
      if (!parserExpect(state, TOK_STRING, "expected string after \".ascii\""))
        return P_SYN_ERROR;
      char *str = lexGetLastStringValue(state->lex);
      data.dataSize = sizeof(char);
      data.initialized = 1;
      for (; *str != '\0'; str++) {
        data.data[0] = *str;
        objSecAppendData(state->curSection, data);
      }
    } while (parserAccept(state, TOK_COMMA));
    return P_ACCEPT;
  }

  return P_SYN_ERROR;
}


static t_parserError expectAlign(t_parserState *state, t_tokenID lastToken)
{
  t_alignData align = {0};

  if (!parserExpect(state, TOK_NUMBER, "expected alignment amount"))
    return P_SYN_ERROR;
  int32_t amt = lexGetLastNumberValue(state->lex);
  if (amt <= 0) {
    parserEmitError(state, "alignment amount must be a positive integer");
    return P_SYN_ERROR;
  }
  if (lastToken == TOK_ALIGN)
    align.alignModulo = 1U << amt;
  else
    align.alignModulo = amt;

  if (objSecGetID(state->curSection) == OBJ_SECTION_TEXT) {
    if ((align.alignModulo % 4) != 0)
      fprintf(stderr, "%s",
          "warning: alignment directive in .text with an amount which is not "
          "multiple of 4\n");
    else
      align.nopFill = true;
  }

  if (parserAccept(state, TOK_COMMA)) {
    if (!parserExpect(state, TOK_NUMBER, "expected alignment padding value"))
      return P_SYN_ERROR;
    int32_t pad = lexGetLastNumberValue(state->lex);
    if (pad < -128 || pad >= 256) {
      parserEmitError(state, "alignment padding must fit in a 8-bit byte");
      return P_SYN_ERROR;
    }
    align.nopFill = false;
    align.fillByte = (uint8_t)pad;
  } else {
    align.fillByte = 0;
  }

  objSecAppendAlignmentData(state->curSection, align);
  return P_ACCEPT;
}


static t_parserError expectLineContent(t_parserState *state)
{
  if (parserAccept(state, TOK_SPACE) == P_ACCEPT)
    return expectData(state, TOK_SPACE);
  if (parserAccept(state, TOK_WORD) == P_ACCEPT)
    return expectData(state, TOK_WORD);
  if (parserAccept(state, TOK_HALF) == P_ACCEPT)
    return expectData(state, TOK_HALF);
  if (parserAccept(state, TOK_BYTE) == P_ACCEPT)
    return expectData(state, TOK_BYTE);
  if (parserAccept(state, TOK_ASCII) == P_ACCEPT)
    return expectData(state, TOK_ASCII);
  if (parserAccept(state, TOK_ALIGN) == P_ACCEPT)
    return expectAlign(state, TOK_ALIGN);
  if (parserAccept(state, TOK_BALIGN) == P_ACCEPT)
    return expectAlign(state, TOK_BALIGN);
  if (parserAccept(state, TOK_MNEMONIC) == P_ACCEPT)
    return expectInstruction(state, TOK_MNEMONIC);
  parserEmitError(state, "expected a data directive or an instruction");
  return P_SYN_ERROR;
}


static t_parserError expectLine(t_parserState *state)
{
  if (parserAccept(state, TOK_NEWLINE) == P_ACCEPT)
    return P_ACCEPT;

  if (parserAccept(state, TOK_TEXT) == P_ACCEPT) {
    state->curSection = objGetSection(state->object, OBJ_SECTION_TEXT);
    return parserExpect(state, TOK_NEWLINE, "expected end of the line");
  } else if (parserAccept(state, TOK_DATA) == P_ACCEPT) {
    state->curSection = objGetSection(state->object, OBJ_SECTION_DATA);
    return parserExpect(state, TOK_NEWLINE, "expected end of the line");
  }

  if (parserAccept(state, TOK_GLOBAL) == P_ACCEPT) {
    if (parserExpect(state, TOK_ID, "expected label identifier") != P_ACCEPT)
      return P_SYN_ERROR;
    return parserExpect(state, TOK_NEWLINE, "expected end of the line");
  }

  if (parserAccept(state, TOK_NUMBER) == P_ACCEPT) {
    int n = lexGetLastNumberValue(state->lex);
    if (n < 0) {
      parserEmitError(state, "local labels must be positive numbers");
      return P_SYN_ERROR;
    }
    if (parserExpect(state, TOK_COLON,
            "expected colon after number to define a local label") != P_ACCEPT)
      return P_SYN_ERROR;
    t_localLabel *ll = parserGetLocalLabel(state, n, 0);
    parserDeclareLocalLabel(state, ll);
  } else if (parserAccept(state, TOK_ID) == P_ACCEPT) {
    char *temp = lexGetLastTokenText(state->lex);
    if (parserExpect(state, TOK_COLON, "expected label or valid mnemonic") !=
        P_ACCEPT)
      return P_SYN_ERROR;
    t_objLabel *label = objGetLabel(state->object, temp);
    if (!objSecDeclareLabel(state->curSection, label))
      parserEmitError(state, "label already declared");
    free(temp);
  }

  if (parserAccept(state, TOK_NEWLINE) == P_ACCEPT)
    return P_ACCEPT;
  if (expectLineContent(state) != P_ACCEPT)
    return P_SYN_ERROR;
  return parserExpect(state, TOK_NEWLINE, "expected end of the line");
}


t_object *parseObject(t_lexer *lex)
{
  t_parserError err;
  t_parserState state;

  state.lex = lex;
  state.object = newObject();
  if (!state.object)
    goto error;
  state.curSection = objGetSection(state.object, OBJ_SECTION_TEXT);
  state.numErrors = 0;
  state.hasLastToken = 0;
  state.backLabels = NULL;
  state.forwardLabels = NULL;

  while (parserAccept(&state, TOK_EOF) != P_ACCEPT) {
    err = expectLine(&state);
    if (err != P_ACCEPT) {
      t_tokenID tmp;

      if (state.numErrors > 10) {
        fprintf(stderr, "too many errors, aborting...\n");
        break;
      }

      /* try to ignore the error and advance to the next line */
      tmp = parserPeekToken(&state);
      while (tmp != TOK_NEWLINE && tmp != TOK_EOF) {
        parserConsumeToken(&state);
        tmp = parserPeekToken(&state);
      }
    }
  }

  deleteLocalLabelList(state.backLabels);
  deleteLocalLabelList(state.forwardLabels);

  if (state.numErrors > 0)
    goto error;

  return state.object;
error:
  deleteObject(state.object);
  return NULL;
}
