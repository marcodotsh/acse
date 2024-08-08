/// @file list.c
/// @brief Implementation of a double-linked list.

#include <stdlib.h>
#include <assert.h>
#include "list.h"
#include "errors.h"


static t_listNode *newListNode(void *data)
{
  t_listNode *result = (t_listNode *)malloc(sizeof(t_listNode));
  if (result == NULL)
    fatalError("out of memory");

  result->data = data;
  result->prev = NULL;
  result->next = NULL;
  return result;
}


static t_listNode *listInsertNodeAfter(
    t_listNode *list, t_listNode *listPos, t_listNode *newElem)
{
  if (listPos == NULL) {
    // add at the beginning of the list
    if (list != NULL) {
      list->prev = newElem;
      newElem->next = list;
    }
    return newElem;
  }

  newElem->next = listPos->next;
  newElem->prev = listPos;
  listPos->next = newElem;
  if (newElem->next)
    newElem->next->prev = newElem;

  return list;
}


t_listNode *listInsertAfter(t_listNode *list, t_listNode *listPos, void *data)
{
  t_listNode *newElem = newListNode(data);
  return listInsertNodeAfter(list, listPos, newElem);
}


t_listNode *listGetLastNode(t_listNode *list)
{
  if (list == NULL)
    return NULL;
  while (list->next != NULL)
    list = list->next;
  return list;
}


t_listNode *listInsertBefore(t_listNode *list, t_listNode *listPos, void *data)
{
  if (!listPos) {
    // add at the end of the list
    return listInsertAfter(list, listGetLastNode(list), data);
  }
  return listInsertAfter(list, listPos->prev, data);
}


t_listNode *listGetNodeAt(t_listNode *list, unsigned int position)
{
  if (list == NULL)
    return NULL;

  t_listNode *curNode = list;
  unsigned int i = 0;
  while ((curNode != NULL) && (i < position)) {
    curNode = curNode->next;
    i++;
  }
  return curNode;
}


t_listNode *listInsert(t_listNode *list, void *data, int pos)
{
  t_listNode *prev;

  if (pos < 0) {
    // add last
    prev = NULL;
  } else {
    prev = listGetNodeAt(list, (unsigned int)pos);
  }
  return listInsertBefore(list, prev, data);
}


t_listNode *listInsertSorted(
    t_listNode *list, void *data, int (*compareFunc)(void *a, void *b))
{
  t_listNode *prevNode = NULL;
  t_listNode *curNode = list;
  while (curNode != NULL) {
    void *curData = curNode->data;

    if (compareFunc(curData, data) >= 0)
      return listInsertBefore(list, curNode, data);

    prevNode = curNode;
    curNode = curNode->next;
  }

  return listInsertAfter(list, prevNode, data);
}


static bool listDataDefaultCompareFunc(void *a, void *b)
{
  return a == b;
}

t_listNode *listFindWithCallback(
    t_listNode *list, void *data, bool (*compareFunc)(void *a, void *b))
{
  if (compareFunc == NULL)
    compareFunc = listDataDefaultCompareFunc;
  if (list == NULL)
    return NULL;

  t_listNode *curNode = list;
  while (curNode != NULL) {
    if (compareFunc(curNode->data, data))
      break;
    curNode = curNode->next;
  }
  return curNode;
}


t_listNode *listFind(t_listNode *list, void *data)
{
  return listFindWithCallback(list, data, NULL);
}


t_listNode *listRemoveNode(t_listNode *list, t_listNode *element)
{
  if (list == NULL || element == NULL)
    return list;
  assert(list->prev == NULL && "prev link of head of list not NULL");
  if ((element->prev == NULL) && (element != list))
    return list;

  if (element->prev != NULL) {
    // in the middle or at the end of the list
    element->prev->next = element->next;
    if (element->next != NULL)
      element->next->prev = element->prev;
  } else {
    // head of the list
    assert(list == element && "element to remove not belonging to the list");

    if (element->next != NULL) {
      element->next->prev = NULL;

      // update the new head of the list
      list = element->next;
    } else
      list = NULL;
  }

  free(element);
  // return the new head of the list
  return list;
}


t_listNode *listFindAndRemove(t_listNode *list, void *data)
{
  t_listNode *curNode = listFind(list, data);
  if (curNode)
    list = listRemoveNode(list, curNode);
  return list;
}


t_listNode *deleteList(t_listNode *list)
{
  while (list != NULL)
    list = listRemoveNode(list, list);
  return NULL;
}


int listNodePosition(t_listNode *list, t_listNode *element)
{
  if (list == NULL || element == NULL)
    return -1;

  int counter = 0;
  while (list != NULL && list != element) {
    counter++;
    list = list->next;
  }

  if (list == NULL)
    return -1;
  return counter;
}


int listLength(t_listNode *list)
{
  int counter = 0;
  while (list != NULL) {
    counter++;
    list = list->next;
  }
  return counter;
}


t_listNode *listAppendList(t_listNode *list, t_listNode *elements)
{
  t_listNode *curSrc = elements;
  t_listNode *curDest = listGetLastNode(list);

  while (curSrc != NULL) {
    t_listNode *newNode = newListNode(curSrc->data);
    list = listInsertNodeAfter(list, curDest, newNode);
    curDest = newNode;
    curSrc = curSrc->next;
  }

  return list;
}

t_listNode *listClone(t_listNode *list)
{
  return listAppendList(NULL, list);
}
