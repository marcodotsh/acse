/// @file list.h
/// @brief A double-linked list.

#ifndef LIST_H
#define LIST_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/** Convert an integer from a list data pointer */
#define INT_TO_LIST_DATA(data) ((void *)((intptr_t)(data)))

/** Convert a data item pointer created by INT_TO_LIST_DATA() to an integer */
#define LIST_DATA_TO_INT(data) ((int)((intptr_t)(data)))

/** A list element */
typedef struct t_listNode {
   struct t_listNode *next; /* The next element in the chain, if it exists,
                              * or NULL instead. */
   struct t_listNode *prev; /* The previous element in the chain, if it exists,
                              * or NULL instead. */
   void *data;               /* Pointer to the data associated to this node */
} t_listNode;


/** Add an element to the given list in a specific position.
 * @param list The list where to add the element.
 * @param data The data pointer that will be associated to the new element.
 * @param pos  The zero-based index where to put the new element in the list.
 *             If pos is negative, or is larger than the number of elements in
 *             the list, the new element is added on to the end of the list.
 * @returns A pointer to the new head of the list. */
extern t_listNode *addElement(t_listNode *list, void *data, int pos);

/** Add a new element in a list after another given element.
 * @param list    The list where to add the element.
 * @param listPos The existing element after which the new element will be
 *                inserted. If NULL, the element will be added at the beginning
 *                of the list.
 * @param data The data pointer that will be associated to the new element.
 * @returns A pointer to the new head of the list. */
extern t_listNode *addAfter(
      t_listNode *list, t_listNode *listPos, void *data);

/** Add a new element in a list before another given element.
 * @param list    The list where to add the element.
 * @param listPos The existing element before which the new element will be
 *                inserted. If NULL, the element will be added at the end
 *                of the list.
 * @param data The data pointer that will be associated to the new element.
 * @returns A pointer to the new head of the list. */
extern t_listNode *addBefore(
      t_listNode *list, t_listNode *listPos, void *data);

/** Add a new element in a sorted list.
 * @param list          The list where to add the element.
 * @param data          The data pointer that will be associated to the new
 *                      element.
 * @param compareFunc   A function for comparing two data pointers. The function
 *                      shall return -1 if a < b, 0 if a == b, and 1 if a > b.
 * @returns A pointer to the new head of the list. */
extern t_listNode *addSorted(
      t_listNode *list, void *data, int (*compareFunc)(void *a, void *b));

/** Add elements to a list by copying them from another list.
 * @param list       The list where to add the elements.
 * @param elements   Another list to be copied from.
 * @returns The new head of the first list, after the elements from the second
 *          list have been copied and added to it. */
extern t_listNode *addList(t_listNode *list, t_listNode *elements);


/** Remove a given element from a list.
 * @param list    The list where to remove an element.
 * @param element The element to remove from the list.
 * @returns A pointer to the new head of the list. */
extern t_listNode *removeElement(t_listNode *list, t_listNode *element);

/** Finds an element in a list with a given data pointer, and then deletes that
 * element.
 * @param list The list where to remove an element.
 * @param data The data pointer of the element to be deleted. If multiple
 *             elements have the same data pointer, only the first one will be
 *             deleted.
 * @returns A pointer to the new head of the list. */
extern t_listNode *removeElementWithData(t_listNode *list, void *data);


/** Finds an element in a list with a data pointer equal to a given one.
 * @param list The list where to search for the element.
 * @param data The data pointer to search for. If multiple elements have the
 *             same data pointer, only the first one will be found.
 * @returns The list element with the given data pointer, or NULL if no such
 *          element is in the list. */
extern t_listNode *findElement(t_listNode *list, void *data);

/** Finds an element in a list associated with a given data item, according to
 * a comparison function.
 * @param list          The list where to search for the element.
 * @param data          Pointer to the data to search for.
 * @param compareFunc   A function that is used to determine if two data
 *                      pointers are equal or not. Shall return true only if
 *                      the two data items are considered equal.
 * @returns The first list element that satisfies the comparison function,
 *          or NULL if no such element is in the list. */
extern t_listNode *findElementWithCallback(
      t_listNode *list, void *data, int (*compareFunc)(void *a, void *b));

/** Finds the position of an element inside a list.
 * @param list    The list where the element belongs.
 * @param element Element to retrieve the position of.
 * @returns The zero-based position of the element, or -1 if the element does
 *          not belong in the list. */
extern int getPosition(t_listNode *list, t_listNode *element);

/** Retrieves the element in a given position in a list.
 * @param list       The list where to find the element.
 * @param position   The zero-based location of the element to retrieve.
 * @returns The element in the given position, or NULL if the list is empty or
 *          if it is too short for the position to be valid. */
extern t_listNode *getElementAt(t_listNode *list, unsigned int position);

/** Retrieves the last element in a list.
 * @param list The list where to find the element.
 * @returns The last element in the list, or NULL if the list is empty. */
extern t_listNode *getLastElement(t_listNode *list);


/** Find the size of a list.
 * @param list A list.
 * @returns The number of elements in the list. */
extern int getLength(t_listNode *list);

/** Create a new list with the same elements as another.
 * @param list The list that will be copied.
 * @returns A new list where all the elements have the same data pointers and
 *          in the same order as the given list. */
extern t_listNode *cloneList(t_listNode *list);


/** Remove all the elements of a list.
 * @param list The list to be deleted.
 * @returns NULL. */
extern t_listNode *freeList(t_listNode *list);


#endif
