/*
 * Andrea Di Biagio
 * Politecnico di Milano, 2007
 *
 * list.c
 * Formal Languages & Compilers Machine, 2007/2008
 *
 */

#include <stdlib.h>
#include <assert.h>
#include "list.h"
#include "errors.h"


t_list *newElement(void *data)
{
   t_list *result;

   /* create an instance of t_list in memory */
   result = (t_list *)malloc(sizeof(t_list));

   /* verify the out of memory condition */
   if (result == NULL)
      fatalError(AXE_OUT_OF_MEMORY);

   /* set the internal value of the just created t_list element */
   result->data = data;
   result->prev = NULL;
   result->next = NULL;

   /* postconditions : return the element */
   return result;
}


t_list *addLinkAfter(t_list *list, t_list *listPos, t_list *newElem)
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


t_list *addAfter(t_list *list, t_list *listPos, void *data)
{
   t_list *newElem;

   newElem = newElement(data);
   return addLinkAfter(list, listPos, newElem);
}


t_list *getLastElement(t_list *list)
{
   /* preconditions */
   if (list == NULL)
      return NULL;

   /* find the last element of the list */
   while (list->next != NULL)
      list = list->next;
   return list;
}


t_list *addBefore(t_list *list, t_list *listPos, void *data)
{
   if (!listPos) {
      /* add at the end of the list */
      return addAfter(list, getLastElement(list), data);
   }
   return addAfter(list, listPos->prev, data);
}


t_list *getElementAt(t_list *list, unsigned int position)
{
   t_list *current_element;
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


t_list *addElement(t_list *list, void *data, int pos)
{
   t_list *prev;

   if (pos < 0) {
      /* add last */
      prev = NULL;
   } else {
      prev = getElementAt(list, pos);
   }

   return addBefore(list, prev, data);
}


t_list *addSorted(
      t_list *list, void *data, int (*compareFunc)(void *a, void *b))
{
   t_list *cur_elem, *prev_elem;
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


int defaultListItemCompareFunc(void *a, void *b)
{
   return a == b;
}

t_list *findElementWithCallback(
      t_list *list, void *data, int (*compareFunc)(void *a, void *b))
{
   t_list *current_elem;
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


t_list *findElement(t_list *list, void *data)
{
   return findElementWithCallback(list, data, NULL);
}


t_list *removeElement(t_list *list, t_list *element)
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


t_list *removeElementWithData(t_list *list, void *data)
{
   t_list *current_elem;

   current_elem = findElement(list, data);
   if (current_elem)
      list = removeElement(list, current_elem);
   return list;
}


t_list *freeList(t_list *list)
{
   while (list != NULL)
      list = removeElement(list, list);
   return NULL;
}


int getPosition(t_list *list, t_list *element)
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


int getLength(t_list *list)
{
   int counter;

   counter = 0;
   while (list != NULL) {
      counter++;
      list = list->next;
   }

   return counter;
}


t_list *addList(t_list *list, t_list *elements)
{
   t_list *current_src, *current_dest, *new_elem;

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


t_list *cloneList(t_list *list)
{
   return addList(NULL, list);
}
