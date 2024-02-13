/// @file list.c

#include <stdlib.h>
#include <assert.h>
#include "list.h"
#include "acse.h"


t_listNode *newElement(void *data)
{
  t_listNode *result;

  /* create an instance of t_listNode in memory */
  result = (t_listNode *)malloc(sizeof(t_listNode));

  /* verify the out of memory condition */
  if (result == NULL)
    fatalError("out of memory");

  /* set the internal value of the just created t_listNode element */
  result->data = data;
  result->prev = NULL;
  result->next = NULL;

  /* postconditions : return the element */
  return result;
}


t_listNode *addLinkAfter(
    t_listNode *list, t_listNode *listPos, t_listNode *newElem)
{
  if (listPos == NULL) {
    /* add at the beginning of the list */
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


t_listNode *addAfter(t_listNode *list, t_listNode *listPos, void *data)
{
  t_listNode *newElem;

  newElem = newElement(data);
  return addLinkAfter(list, listPos, newElem);
}


t_listNode *getLastElement(t_listNode *list)
{
  /* preconditions */
  if (list == NULL)
    return NULL;

  /* find the last element of the list */
  while (list->next != NULL)
    list = list->next;
  return list;
}


t_listNode *addBefore(t_listNode *list, t_listNode *listPos, void *data)
{
  if (!listPos) {
    /* add at the end of the list */
    return addAfter(list, getLastElement(list), data);
  }
  return addAfter(list, listPos->prev, data);
}


t_listNode *getElementAt(t_listNode *list, unsigned int position)
{
  t_listNode *current_element;
  unsigned int current_pos;

  if (list == NULL)
    return NULL;

  /* initialize the local variables */
  current_element = list;
  current_pos = 0;
  while ((current_element != NULL) && (current_pos < position)) {
    current_element = current_element->next;
    current_pos++;
  }

  /* return the element at the requested position */
  return current_element;
}


t_listNode *addElement(t_listNode *list, void *data, int pos)
{
  t_listNode *prev;

  if (pos < 0) {
    /* add last */
    prev = NULL;
  } else {
    prev = getElementAt(list, (unsigned int)pos);
  }

  return addBefore(list, prev, data);
}


t_listNode *addSorted(
    t_listNode *list, void *data, int (*compareFunc)(void *a, void *b))
{
  t_listNode *cur_elem, *prev_elem;
  void *current_data;

  prev_elem = NULL;
  cur_elem = list;
  while (cur_elem != NULL) {
    current_data = cur_elem->data;

    if (compareFunc(current_data, data) >= 0)
      return addBefore(list, cur_elem, data);

    prev_elem = cur_elem;
    cur_elem = cur_elem->next;
  }

  return addAfter(list, prev_elem, data);
}


static bool defaultListItemCompareFunc(void *a, void *b)
{
  return a == b;
}

t_listNode *findElementWithCallback(
    t_listNode *list, void *data, bool (*compareFunc)(void *a, void *b))
{
  t_listNode *current_elem;
  void *other_data;

  /* preconditions */
  if (compareFunc == NULL)
    compareFunc = defaultListItemCompareFunc;

  if (list == NULL)
    return NULL;

  /* intialize the value of `current_elem' */
  current_elem = list;
  while (current_elem != NULL) {
    other_data = current_elem->data;

    if (compareFunc(other_data, data))
      break;

    current_elem = current_elem->next;
  }

  /* postconditions */
  return current_elem;
}


t_listNode *findElement(t_listNode *list, void *data)
{
  return findElementWithCallback(list, data, NULL);
}


t_listNode *removeElement(t_listNode *list, t_listNode *element)
{
  /* preconditions */
  if (list == NULL || element == NULL)
    return list;
  assert(list->prev == NULL && "prev link of head of list not NULL");
  if ((element->prev == NULL) && (element != list))
    return list;

  if (element->prev != NULL) {
    /* in the middle or at the end of the list */
    element->prev->next = element->next;
    if (element->next != NULL)
      element->next->prev = element->prev;
  } else {
    /* head of the list */
    assert(list == element);

    if (element->next != NULL) {
      element->next->prev = NULL;

      /* update the new head of the list */
      list = element->next;
    } else
      list = NULL;
  }

  free(element);

  /* return the new top of the list */
  return list;
}


t_listNode *removeElementWithData(t_listNode *list, void *data)
{
  t_listNode *current_elem;

  current_elem = findElement(list, data);
  if (current_elem)
    list = removeElement(list, current_elem);
  return list;
}


t_listNode *freeList(t_listNode *list)
{
  while (list != NULL)
    list = removeElement(list, list);
  return NULL;
}


int getPosition(t_listNode *list, t_listNode *element)
{
  int counter;

  if (list == NULL || element == NULL)
    return -1;

  counter = 0;
  while (list != NULL && list != element) {
    counter++;
    list = list->next;
  }

  if (list == NULL)
    return -1;
  return counter;
}


int getLength(t_listNode *list)
{
  int counter;

  counter = 0;
  while (list != NULL) {
    counter++;
    list = list->next;
  }

  return counter;
}


t_listNode *addList(t_listNode *list, t_listNode *elements)
{
  t_listNode *current_src, *current_dest, *new_elem;

  current_src = elements;
  current_dest = getLastElement(list);

  while (current_src != NULL) {
    new_elem = newElement(current_src->data);
    list = addLinkAfter(list, current_dest, new_elem);
    current_dest = new_elem;

    current_src = current_src->next;
  }

  return list;
}


t_listNode *cloneList(t_listNode *list)
{
  return addList(NULL, list);
}
