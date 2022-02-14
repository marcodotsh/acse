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

/* function prototypes */
static t_list * newElement(void *data);


/* remove the first element of the list. Returns the new
 * head of the list */
t_list * removeFirst(t_list *list)
{
   t_list *first_elem;
   
   if (list == NULL)
      return NULL;

   first_elem = list;
   list = list->next;
   if (list != NULL)
      list->prev = NULL;
   
   free(first_elem);

   /* postconditions: return the new head of the list */
   return list;
}

/* add an element `data' to the list `list' at position `pos'. If pos is negative
 * , or is larger than the number of elements in the list, the new element is
 * added on to the end of the list. Function `addElement' returns a pointer
 * to the new head of the list */
t_list * addElement(t_list *list, void * data, int pos)
{
   t_list *result;
   t_list *current_elem;
   t_list *last_elem;
   int   current_pos;
   
   /* initialize the value of `result' */
   result = newElement(data);

   if (list == NULL)
      return result;
   
   if (pos == 0)
   {
      /* update the control informations */
      result->next = list;
      list->prev = result;
      
      /* return the new head of the list */
      return result;
   }
   
   /* retrieve the last element of the list */
   last_elem = getLastElement(list);
   assert(last_elem != NULL);
   
   if (pos < 0)
   {
      /* update the control informations */
      last_elem->next = result;
      result->prev = last_elem;
      
      /* update the value of result */
      return list;
   }
   
   /* `pos' is a positive integer */
   current_pos = 0;
   current_elem = list;
   
   while(current_pos < pos)
   {
      if (current_elem == last_elem)
      {
         /* update the control informations */
         last_elem->next = result;
         result->prev = last_elem;
         
         /* update the value of result */
         return list;
      }
      
      /* update the loop informations */
      current_elem = current_elem->next;
      current_pos++;
   }

   /* assertions */
   assert(current_elem != NULL);
   
   /* update the control informations */
   result->next = current_elem;
   result->prev = current_elem->prev;
   current_elem->prev->next = result;
   current_elem->prev = result;
   
   /* return the new head of the list */
   return list;
}

/* add an element at the beginning of the list */
t_list * addFirst(t_list *list, void * data)
{
   t_list *result;
   
   /* initialize the value of `result' */
   result = newElement(data);
   
   if (list == NULL)
      return result;
   
   /* postconditions */
   list->prev = result;
   result->next = list;
   
   /* return the new head of the list */
   return result;
}

/* add an element to the end of the list */
t_list * addLast(t_list *list, void * data)
{
   /* call the `addElement' */
   return addElement(list, data, -1);
}

t_list *addBefore(t_list *list, t_list *listPos, void *data)
{
   t_list *newElem;

   if (!listPos) {
      /* add at the end of the list */
      return addElement(list, data, -1);
   }

   newElem = newElement(data);
   assert(newElem);

   newElem->next = listPos;
   newElem->prev = listPos->prev;
   listPos->prev = newElem;
   if (newElem->prev) {
      newElem->prev->next = newElem;
   } else {
      /* we are adding at the head of the list */
      assert(list == listPos);
      list = newElem;
   }

   return list;
}

t_list *addAfter(t_list *list, t_list *listPos, void *data)
{
   t_list *newElem;

   if (listPos == NULL) {
      /* add at the beginning of the list */
      return addElement(list, data, 0);
   }

   newElem = newElement(data);
   assert(newElem);

   newElem->next = listPos->next;
   newElem->prev = listPos;
   listPos->next = newElem;
   if (newElem->next)
      newElem->next->prev = newElem;

   return list;
}

/* remove an element from the list */
t_list * removeElement(t_list *list, void * data)
{
   t_list *current_elem;
   
   /* preconditions: the list shouldn't be empty */
   if (list == NULL)
      return NULL;
   
   /* intialize the value of `current_elem' */
   current_elem = list;
   while (  (current_elem != NULL)
            && (current_elem->data != data))
   {
      current_elem = current_elem->next;
   }
   
   return removeElementLink(list, current_elem);
}

/* remove all the elements of a list */
void freeList(t_list *list)
{
   /* verify the preconditions */
   if (list == NULL)
      return;

   /* recursively call the freeList */
   freeList(list->next);
   
   /* deallocate memory for the current element of the list */
   free(list);
}

t_list * newElement(void *data)
{
   t_list * result;
   
   /* create an instance of t_list in memory */
   result = (t_list *)malloc(sizeof(t_list));

   /* verify the out of memory condition */
   if (result == NULL)
   {
      fprintf(stderr, "COLLECTIONS.C:: _ALLOC_FUNCTION returned a NULL pointer \n");
      abort();
   }

   /* set the internal value of the just created t_list element */
   result->data = data;
   result->prev = NULL;
   result->next = NULL;
   
   /* postconditions : return the element */
   return result;
}

t_list * getLastElement(t_list *list)
{
   /* preconditions */
   if (list == NULL)
      return NULL;
   
   /* test if the current element is the last element of the list */
   if (list->next == NULL)
      return list;
   
   /* recursively call the getLastElement on the next item of the list */
   return getLastElement(list->next);
}

/* remove a link from the list `list' */
extern t_list * removeElementLink(t_list *list, t_list *element)
{  
   /* preconditions */
   if (list == NULL || element == NULL)
      return list;
   assert(list->prev == NULL && "prev link of head of list not NULL");
   if ((element->prev == NULL) && (element != list))
      return list;

   /* the value is found */
   if (element->prev != NULL)
   {
      element->prev->next = element->next;
      if (element->next != NULL)
         element->next->prev = element->prev;
      
      free(element);
   }
   else
   {
      /* check the preconditions */
      assert(list == element);
      
      if (element->next != NULL)
      {
         element->next->prev = NULL;
         
         /* update the new head of the list */
         list = element->next;
      }
      else
         list = NULL;
      
      free(element);
   }
   
   /* return the new top of the list */
   return list;
}

/* find an element inside the list `list'. The current implementation calls the
 * findElementWithCallback' passing a NULL reference as `func' */
t_list * findElement(t_list *list, void *data)
{
   t_list *current_elem;
   
   /* if the list is empty returns a NULL pointer */
   if (list == NULL)
      return NULL;

   /* intialize the value of `current_elem' */
   current_elem = list;
   while (  (current_elem != NULL)
         && (  current_elem->data != data) )
   {
      current_elem = current_elem->next;
   }
   
   /* postconditions */
   return current_elem;
}

/* find an element inside the list `list'. */
t_list * findElementWithCallback(t_list *list, void *data
      , int (*compareFunc)(void *a, void *b))
{
   t_list *current_elem;   
   
   /* preconditions */
   if (compareFunc == NULL)
      return findElement(list, data);
   
   if (list == NULL)
      return NULL;
   
   /* intialize the value of `current_elem' */
   current_elem = list;
   while (current_elem != NULL)
   {
      void *other_Data;

      other_Data = current_elem->data;

      if (compareFunc(other_Data, data))
         break;
      
      current_elem = current_elem->next;
   }  

   /* postconditions */
   return current_elem;
}

int getPosition(t_list *list, t_list *element)
{
   int counter;
   
   /* preconditions */
   if (list == NULL || element == NULL)
      return -1;
   
   /* initialize the local variable `counter' */
   counter = 0;
   
   if (list == element)
      return counter;
   
   /* update values */
   counter++;
   list = list->next;
   
   while (list != NULL)
   {
      if (list == element)
         return counter;
      
      counter++;
      list = list->next;
   }
   
   return -1;
}

int getLength(t_list *list)
{
   int counter;
   
   /* initialize the local variable `counter' */
   counter = 0;

   /* precondition */
   if (list == NULL)
      return counter;
   
   while (list != NULL)
   {
      counter++;
      list = list->next;
   }
   
   /* postconditions: return the length of the list */
   return counter;
}

t_list * cloneList(t_list *list)
{
   t_list *result;
   t_list *current_element;

   /* initialize the local variables */
   result = NULL;
   current_element = list;

   /* preconditions */
   if (current_element == NULL)
      return result;

   while(current_element != NULL)
   {
      /* add an element to the new list */
      result = addElement(result, current_element->data, -1);

      /* retrieve the next element of the list */
      current_element = current_element->next;
   }

   /* return the new list */
   return result;
}

t_list * getElementAt(t_list *list, unsigned int position)
{
   t_list *current_element;
   unsigned int current_pos;
   
   if (list == NULL)
      return NULL;

   /* initialize the local variables */
   current_element = list;
   current_pos = 0;
   while((current_element != NULL) && (current_pos < position))
   {
      current_element = current_element->next;
      current_pos++;
   }

   /* return the element at the requested position */
   return current_element;
}

t_list * addList(t_list *list, t_list *elements)
{
   t_list *current_element;
   void *current_data;

   /* if the list of elements is null, this function
    * will return `list' unmodified */
   if (elements == NULL)
      return list;

   /* initialize the value of `current_element' */
   current_element = elements;
   while (current_element != NULL)
   {
      /* retrieve the data associated with the current element */
      current_data = current_element->data;
      list = addElement(list, current_data, -1);

      /* retrieve the next element in the list */
      current_element = current_element->next;
   }

   /* return the new list */
   return list;
}

t_list * addListToSet(t_list *list, t_list *elements
      , int (*compareFunc)(void *a, void *b), int *modified)
{
   t_list *current_element;
   void *current_data;

   /* if the list of elements is NULL returns the current list */
   if (elements == NULL)
      return list;

   /* initialize the value of `current_element' */
   current_element = elements;
   while (current_element != NULL)
   {
      /* retrieve the data associated with the current element */
      current_data = current_element->data;

      /* Test if the element was already inserted. */
      if (findElementWithCallback(list, current_data, compareFunc) == NULL)
      {
         list = addElement(list, current_data, -1);
         if (modified != NULL)
            (* modified) = 1;
      }

      /* retrieve the next element in the list */
      current_element = current_element->next;
   }

   /* return the new list */
   return list;
}

/* add sorted */
t_list * addSorted(t_list *list, void * data
            , int (*compareFunc)(void *a, void *b))
{
   t_list *current_element;
   void *current_data;
   int counter;
   
   /* preconditions */
   if (list == NULL)
      return addFirst(list, data);

   counter = 0;
   current_element = list;
   while(current_element != NULL)
   {
      /* get the current interval informations */
      current_data = current_element->data;
      assert(current_data != NULL);

      if (compareFunc(current_data, data) >= 0)
      {
         list = addElement(list, data, counter);
         return list;
      }
         
      /* retrieve the next element */
      current_element = current_element->next;

      /* update the value of counter */
      counter++;
   }

   return addElement(list, data, -1);
}
