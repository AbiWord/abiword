/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <stdlib.h>
#include "ut_dllist.h"

/***************************************************
	UT_DLList - Double Linked list class
 ***************************************************/

#ifndef MALLOC
/**** CHANGE ME ******/
#define MALLOC(x) malloc(x)
#define FREE(x) free(x)
#endif


UT_DLList::UT_DLList(void)
{
	listHead =    (UT_DLListElement *) NIL;
	listTail =    (UT_DLListElement *) NIL;
	listCurrent = (UT_DLListElement *) NIL;
	numberOfElements = 0;
}

UT_DLList::~UT_DLList(void)
/* this destructor destroys the list, but not the data that was allocated 
   externally.  
*/
{
	(void) head();
	while (remove());
}

void * UT_DLList::head(void)
{
	listCurrent = listHead;
	if (listHead == NIL)
		return NIL;
	else
		return listHead->data;
}

void * UT_DLList::tail(void)
{
	listCurrent = listTail;
	if (listTail == NIL)
		return NIL;
	else
		return listTail->data;
}

void * UT_DLList::current(void)
{
	if (listCurrent == NIL)
			return NIL;
	else
			return listCurrent->data;

}


void * UT_DLList::next(void)
{

	void *retVal;

	if (listCurrent == NIL)
		retVal = NIL;
	else
		{
			if (listCurrent->nextPtr == NIL)
				retVal = NIL;
			else
				{
					listCurrent = listCurrent->nextPtr;
					retVal = listCurrent->data;
				}
		}

	return retVal;
}

void * UT_DLList::previous(void)
{

	void * retVal;

	if (listCurrent == NIL)
			retVal = NIL;
	else
		{
			if (listCurrent->prevPtr == NIL)
				retVal = NIL;
			else
				{
					listCurrent = listCurrent->prevPtr;
					retVal = listCurrent->data;
				}
		}

	return retVal;
}


int UT_DLList::append(void * data)
/* Add Data to the list.  Insterting just after the current pointer.
   The current pointer will then be updated to the new data 
   returns number of elements in list on success, 0 on failure 
*/
{
	UT_DLListElement *newElement;

	if (listCurrent != NIL)
	{
		if (!(newElement = (UT_DLListElement *) MALLOC(sizeof(UT_DLListElement))))
		{
			return 0;  /* memory allocation failure */
		}
		newElement->prevPtr = listCurrent;
		newElement->nextPtr = listCurrent->nextPtr;
		newElement->data = data;

		if (listTail == listCurrent)
		{
			listTail = newElement;
		}
		else
		{
			listCurrent->nextPtr->prevPtr = newElement;
		}
		listCurrent->nextPtr = newElement;
		listCurrent = newElement;
	}

	if (listHead == NIL)
	{
		/* list is empty */
		if (!(newElement = (UT_DLListElement *) MALLOC(sizeof(UT_DLListElement))))
		{
			return 0;  /* memory allocation failure */
		}
		newElement->prevPtr = (UT_DLListElement *) NIL;
		newElement->nextPtr = (UT_DLListElement *) NIL;
		newElement->data = data;
		listHead = listTail = listCurrent = newElement;
	}
	numberOfElements++;

	return numberOfElements;
}


int UT_DLList::prepend(void * data)
/* Add data to the list.  Inserting just before the current pointer.
	The current pointer will then be updated to point to the new data.
	returns number of elements in list on success, 0 on failure.
*/
{
	UT_DLListElement *newElement;

	if (listCurrent != NIL)
	{
		if (!(newElement = (UT_DLListElement *) MALLOC(sizeof(UT_DLListElement))))
		{
			return 0;  /* memory allocation failure */
		}
		newElement->prevPtr = listCurrent->prevPtr;
		newElement->nextPtr = listCurrent;
		newElement->data = data;

		if (listHead == listCurrent)
		{
			listHead = newElement;
		}
		else
		{
			listCurrent->prevPtr->nextPtr = newElement;
		}
		listCurrent->prevPtr = newElement;
		listCurrent = newElement;
	}

	if (listHead == NIL)
	{
		/* list is empty */
		if (!(newElement = (UT_DLListElement *) MALLOC(sizeof(UT_DLListElement))))
		{
			return 0;  /* memory allocation failure */
		}
		newElement->prevPtr = (UT_DLListElement *) NIL;
		newElement->nextPtr = (UT_DLListElement *) NIL;
		newElement->data = data;
		listHead = listTail = listCurrent = newElement;
	}
	numberOfElements++;

	return numberOfElements;
}



int UT_DLList::remove(void)
/* Delete current list element.  
   If not at tail, current is set to next element, else to previous element.
   Returns number of elements left in the list 
*/
{
	UT_DLListElement *undead;

	if (listCurrent == NIL)
	{
		return 0;
	}

	if (listHead == listCurrent)
		listHead = listCurrent->nextPtr;
	if (listTail == listCurrent)
		listTail = listCurrent->prevPtr;

	if (listCurrent->prevPtr != NIL)
		listCurrent->prevPtr->nextPtr = listCurrent->nextPtr;

	if (listCurrent->nextPtr != NIL)
		listCurrent->nextPtr->prevPtr = listCurrent->prevPtr;

	undead = listCurrent;
	if (listCurrent->nextPtr != NIL)
		listCurrent = listCurrent->nextPtr;
	else
		listCurrent = listCurrent->prevPtr;

	FREE(undead);

	numberOfElements--;

	return numberOfElements;
}


int UT_DLList::size()
{
	return numberOfElements;
}




