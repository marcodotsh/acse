#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "errors.h"
#include "object.h"
#include "encode.h"

struct t_objLabel {
  struct t_objLabel *next;
  char *name;
  t_objSecItem *pointer;
};

struct t_objSection {
  t_objSectionID id;
  t_objSecItem *items;
  t_objSecItem *lastItem;
  uint32_t start;
  uint32_t size;
};

struct t_object {
  t_objSection *data;
  t_objSection *text;
  t_objLabel *labelList;
};


static t_objSection *newSection(t_objSectionID id)
{
  t_objSection *sec;

  sec = malloc(sizeof(t_objSection));
  if (!sec)
    fatalError("out of memory");
  sec->id = id;
  sec->items = NULL;
  sec->lastItem = NULL;
  sec->start = 0;
  sec->size = 0;
  return sec;
}


static void deleteSection(t_objSection *sec)
{
  t_objSecItem *item, *nextItm;

  if (!sec)
    return;

  for (item = sec->items; item != NULL; item = nextItm) {
    nextItm = item->next;
    free(item);
  }

  free(sec);
}


t_object *newObject(void)
{
  t_object *obj;

  obj = malloc(sizeof(t_object));
  if (!obj)
    fatalError("out of memory");
  obj->data = newSection(OBJ_SECTION_DATA);
  obj->text = newSection(OBJ_SECTION_TEXT);
  obj->labelList = NULL;
  return obj;
}


void deleteObject(t_object *obj)
{
  t_objLabel *lbl, *nextLbl;

  if (!obj)
    return;

  deleteSection(obj->data);
  deleteSection(obj->text);

  for (lbl = obj->labelList; lbl != NULL; lbl = nextLbl) {
    nextLbl = lbl->next;
    free(lbl->name);
    free(lbl);
  }

  free(obj);
}


t_objLabel *objFindLabel(t_object *obj, const char *name)
{
  t_objLabel *lbl;
  for (lbl = obj->labelList; lbl != NULL; lbl = lbl->next) {
    if (strcmp(lbl->name, name) == 0)
      break;
  }
  return lbl;
}

t_objLabel *objGetLabel(t_object *obj, const char *name)
{
  t_objLabel *lbl = objFindLabel(obj, name);
  if (lbl)
    return lbl;

  lbl = malloc(sizeof(t_objLabel));
  if (!lbl)
    fatalError("out of memory");
  lbl->name = strdup(name);
  if (!lbl->name)
    fatalError("out of memory");
  lbl->next = obj->labelList;
  lbl->pointer = NULL;
  obj->labelList = lbl;
  return lbl;
}


t_objSection *objGetSection(t_object *obj, t_objSectionID id)
{
  if (id == OBJ_SECTION_TEXT)
    return obj->text;
  if (id == OBJ_SECTION_DATA)
    return obj->data;
  return NULL;
}


t_objSectionID objSecGetID(t_objSection *sec)
{
  return sec->id;
}


static void objSecInsertAfter(
    t_objSection *sec, t_objSecItem *item, t_objSecItem *prev)
{
  if (prev == NULL) {
    item->next = sec->items;
    if (sec->lastItem == NULL)
      sec->lastItem = item;
    sec->items = item;
    return;
  }

  item->next = prev->next;
  prev->next = item;
  if (item->next == NULL)
    sec->lastItem = item;
}

static void objSecAppend(t_objSection *sec, t_objSecItem *item)
{
  objSecInsertAfter(sec, item, sec->lastItem);
}


void objSecAppendData(t_objSection *sec, t_data data)
{
  t_objSecItem *itm;

  itm = malloc(sizeof(t_objSecItem));
  if (!itm)
    fatalError("out of memory");
  itm->address = 0;
  itm->class = OBJ_SEC_ITM_CLASS_DATA;
  itm->body.data = data;
  objSecAppend(sec, itm);
}

void objSecAppendAlignmentData(t_objSection *sec, t_alignData align)
{
  t_objSecItem *itm;

  itm = malloc(sizeof(t_objSecItem));
  if (!itm)
    fatalError("out of memory");
  itm->address = 0;
  itm->class = OBJ_SEC_ITM_CLASS_ALIGN_DATA;
  itm->body.alignData = align;
  objSecAppend(sec, itm);
}

t_objSecItem *objSecInsertInstructionAfter(
    t_objSection *sec, t_instruction instr, t_objSecItem *prev)
{
  t_objSecItem *itm;

  itm = malloc(sizeof(t_objSecItem));
  if (!itm)
    fatalError("out of memory");
  itm->address = 0;
  itm->class = OBJ_SEC_ITM_CLASS_INSTR;
  itm->body.instr = instr;
  objSecInsertAfter(sec, itm, prev);
  return itm;
}

void objSecAppendInstruction(t_objSection *sec, t_instruction instr)
{
  objSecInsertInstructionAfter(sec, instr, sec->lastItem);
}

bool objSecDeclareLabel(t_objSection *sec, t_objLabel *label)
{
  t_objSecItem *itm;

  if (label->pointer)
    return false;

  itm = malloc(sizeof(t_objSecItem));
  if (!itm)
    fatalError("out of memory");
  itm->address = 0;
  itm->class = OBJ_SEC_ITM_CLASS_VOID;
  objSecAppend(sec, itm);

  label->pointer = itm;
  return true;
}


t_objSecItem *objSecGetItemList(t_objSection *sec)
{
  return sec->items;
}

uint32_t objSecGetStart(t_objSection *sec)
{
  return sec->start;
}

uint32_t objSecGetSize(t_objSection *sec)
{
  return sec->size;
}


t_objSecItem *objLabelGetPointedItem(t_objLabel *lbl)
{
  t_objSecItem *item = lbl->pointer;
  while (item && item->next && item->class == OBJ_SEC_ITM_CLASS_VOID)
    item = item->next;
  return item;
}

const char *objLabelGetName(t_objLabel *lbl)
{
  return lbl->name;
}

uint32_t objLabelGetPointer(t_objLabel *lbl)
{
  if (!lbl->pointer)
    return 0;
  return lbl->pointer->address;
}


static bool objSecExpandPseudoInstructions(t_objSection *sec)
{
  t_instruction buf[MAX_EXP_FACTOR];
  int n, i;
  t_objSecItem *itm;

  for (itm = sec->items; itm != NULL; itm = itm->next) {
    if (itm->class != OBJ_SEC_ITM_CLASS_INSTR)
      continue;

    n = encExpandPseudoInstruction(itm->body.instr, buf);
    if (n == 0)
      return false;
    i = 0;
    itm->body.instr = buf[i++];
    for (; i < n; i++)
      itm = objSecInsertInstructionAfter(sec, buf[i], itm);
  }
  return true;
}

static bool objSecMaterializeAddresses(t_objSection *sec, uint32_t *curAddr)
{
  size_t alignAmt;
  t_objSecItem *itm;

  sec->start = *curAddr;
  sec->size = 0;
  for (itm = sec->items; itm != NULL; itm = itm->next) {
    itm->address = *curAddr;
    size_t thisSize = 0;
    t_fileLocation loc;
    switch (itm->class) {
      case OBJ_SEC_ITM_CLASS_DATA:
        loc = itm->body.data.location;
        thisSize = itm->body.data.dataSize;
        break;
      case OBJ_SEC_ITM_CLASS_ALIGN_DATA:
        loc = itm->body.alignData.location;
        alignAmt = itm->body.alignData.alignModulo;
        if (*curAddr % alignAmt == 0)
          thisSize = 0;
        else
          thisSize = alignAmt - (*curAddr % alignAmt);
        itm->body.alignData.effectiveSize = thisSize;
        if (objSecGetID(sec) == OBJ_SECTION_TEXT) {
          if (itm->body.alignData.nopFill &&
              (thisSize % 4 != 0 || itm->address % 4 != 0)) {
            emitWarning(loc,
                "implicit nop-fill alignment in .text not aligned to "
                "a multiple of 4 bytes, using zero-fill instead");
            itm->body.alignData.nopFill = false;
            itm->body.alignData.fillByte = 0;
          }
        }
        break;
      case OBJ_SEC_ITM_CLASS_INSTR:
        loc = itm->body.instr.location;
        thisSize = encGetInstrLength(itm->body.instr);
        break;
    }
    // Same as 0x100000000 - *curAddr but safe for 32-bit-sized size_t
    size_t sizeLeft = ((size_t)0xFFFFFFFF - (size_t)(*curAddr)) + (size_t)1;
    if (thisSize > sizeLeft) {
      emitError(loc, "section overflows addressing space");
      return false;
    }
    sec->size += (uint32_t)thisSize;
    *curAddr += (uint32_t)thisSize;
  }
  return true;
}

static bool objSecResolveImmediates(t_objSection *sec)
{
  t_objSecItem *itm;

  for (itm = sec->items; itm != NULL; itm = itm->next) {
    if (itm->class != OBJ_SEC_ITM_CLASS_INSTR)
      continue;
    if (!encResolveImmediates(&itm->body.instr, itm->address))
      return false;
  }
  return true;
}

static bool objSecMaterializeInstructions(t_objSection *sec)
{
  t_objSecItem *itm;

  for (itm = sec->items; itm != NULL; itm = itm->next) {
    t_data tmp = {0};

    if (itm->class != OBJ_SEC_ITM_CLASS_INSTR)
      continue;

    if (!encPhysicalInstruction(itm->body.instr, itm->address, &tmp))
      return false;
    itm->class = OBJ_SEC_ITM_CLASS_DATA;
    itm->body.data = tmp;
  }
  return true;
}

bool objMaterialize(t_object *obj)
{
  // Transform pseudo-instructions to normal instructions
  if (!objSecExpandPseudoInstructions(obj->text))
    return false;
  if (!objSecExpandPseudoInstructions(obj->data))
    return false;

  // Assign an address to every item in the object
  uint32_t curAddr = 0x1000;
  if (!objSecMaterializeAddresses(obj->text, &curAddr))
    return false;
  if (!objSecMaterializeAddresses(obj->data, &curAddr))
    return false;

  // Transform label references into constants
  if (!objSecResolveImmediates(obj->text))
    return false;
  if (!objSecResolveImmediates(obj->data))
    return false;

  // Transform instructions into data
  if (!objSecMaterializeInstructions(obj->text))
    return false;
  if (!objSecMaterializeInstructions(obj->data))
    return false;

  return true;
}


static void objSecDump(t_objSection *sec)
{
  printf("{\n");
  printf("  Start = 0x%08x\n", sec->start);
  printf("  Size = 0x%08x\n", sec->size);
  for (t_objSecItem *itm = sec->items; itm != NULL; itm = itm->next) {
    printf("  %p = {\n", (void *)itm);
    printf("    Address = 0x%08x,\n", itm->address);
    printf("    Class = %d,\n", itm->class);
    if (itm->class == OBJ_SEC_ITM_CLASS_INSTR) {
      printf("    Opcode = %d,\n", itm->body.instr.opcode);
      printf("    Dest = %d,\n", itm->body.instr.dest);
      printf("    Src1 = %d,\n", itm->body.instr.src1);
      printf("    Src2 = %d,\n", itm->body.instr.src2);
      printf("    Immediate mode = %d,\n", itm->body.instr.immMode);
      printf("      Constant = %d,\n", itm->body.instr.constant);
      printf("      Label = %p,\n", (void *)itm->body.instr.label);
    } else if (itm->class == OBJ_SEC_ITM_CLASS_DATA) {
      printf("    DataSize = %ld,\n", itm->body.data.dataSize);
      printf("    Initialized = %d,\n", itm->body.data.initialized);
      if (itm->body.data.initialized) {
        printf("    Data = { ");
        for (int i = 0; i < DATA_MAX; i++)
          printf("%02x ", itm->body.data.data[i]);
        printf("}\n");
      }
    } else if (itm->class == OBJ_SEC_ITM_CLASS_ALIGN_DATA) {
      printf("    Alignment value = %ld,\n", itm->body.alignData.alignModulo);
      printf("    Effective size = %ld,\n", itm->body.alignData.effectiveSize);
      printf("    Fill value = %02x\n", itm->body.alignData.fillByte);
    } else if (itm->class == OBJ_SEC_ITM_CLASS_VOID) {
      printf("    (null contents)\n");
    } else {
      printf("    (class is invalid!)\n");
    }
    printf("  },\n");
  }
  printf("}\n");
  fflush(stdout);
}

void objDump(t_object *obj)
{
  t_objLabel *label;

  printf("Labels: {\n");
  for (label = obj->labelList; label != NULL; label = label->next) {
    printf("  %p = {Name = \"%s\", Pointer = %p},\n", (void *)label,
        label->name, (void *)label->pointer);
  }
  printf("}\n");

  printf("Data section: ");
  objSecDump(obj->data);

  printf("Text section: ");
  objSecDump(obj->text);
}
