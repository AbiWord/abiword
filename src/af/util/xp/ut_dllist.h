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

#ifndef DLLIST_INCLUDE_BEFORE
#define DLLIST_INCLUDE_BEFORE


/*********************************************************************
	UT_DLList Class - Double Linked List 
 *********************************************************************/
 


#ifndef NIL 
#define NIL (void *) 0
#endif




class UT_DLList
{
private:

	struct UT_DLListElement
	{
		UT_DLListElement *nextPtr;
		UT_DLListElement *prevPtr;
		void			 *data;
	};

	UT_DLListElement *listHead;
	UT_DLListElement *listTail;
	UT_DLListElement *listCurrent;
	int				 numberOfElements;

public:

	UT_DLList(void);
	~UT_DLList(void);

	void * previous(void);
	void * next(void);
	void * current(void);
	void * tail(void);
	void * head(void);

	int append(void * data);
	int prepend(void * data);
	int remove(void);
	int	size();

};

#endif /* DLLIST_INCLUDE_BEFORE */
