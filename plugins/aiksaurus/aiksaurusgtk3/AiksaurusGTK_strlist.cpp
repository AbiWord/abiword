/*
 * AiksaurusGTK - A GTK interface to the AikSaurus library
 * Copyright (C) 2001 by Jared Davis
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include "AiksaurusGTK_strlist.h"
#include "AiksaurusGTK_utils.h"
#include <cstring>


//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   Creation and Destruction                                           //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

AiksaurusGTK_strlist::AiksaurusGTK_strlist()
{
	d_size = 0;
	d_front_ptr = d_back_ptr = static_cast<GList*>(NULL);
}



AiksaurusGTK_strlist::~AiksaurusGTK_strlist()
{
	clear();
}



//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   Inspection Routines                                                //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

const GList*
AiksaurusGTK_strlist::list() const
{
	return d_front_ptr;
}


unsigned int
AiksaurusGTK_strlist::size() const
{
	return d_size;
}


const char*
AiksaurusGTK_strlist::look_back() const
{
	return (d_back_ptr)
		? (static_cast<char*>(d_back_ptr->data))
		: (static_cast<char*>(NULL));
}


const char*
AiksaurusGTK_strlist::look_front() const
{
	return (d_front_ptr)
		? (static_cast<char*>(d_front_ptr->data))
		: (static_cast<char*>(NULL));
}



//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   Node Destruction and Allocation                                    //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

GList*
AiksaurusGTK_strlist::create_node(const char* str) const
{
	char* x = AiksaurusGTK_strCopy(str);
	GList* ret = g_list_alloc();
	ret->data = x;
	ret->prev = ret->next = static_cast<GList*>(NULL);
	return ret;
}


void
AiksaurusGTK_strlist::free_data(GList* node) const
{
	delete[] static_cast<char*>(node->data);
}


void
AiksaurusGTK_strlist::remove_node(GList* node)
{
	// get pointers to the nodes we need to join.
	GList* node_next = node->next;
	GList* node_prev = node->prev;

	// if this is in the middle, we need to reassign both pointers.
	if ((node != d_front_ptr) && (node != d_back_ptr))
	{
		node_next->prev = node_prev;
		node_prev->next = node_next;
	}


	// else if front of list, we want to reassign front to be
	// the new front.
	if (node == d_front_ptr)
	{
		d_front_ptr = node_next;

		if (node_next)
			node_next->prev = static_cast<GList*>(NULL);
	}


	// similarly, if back of list, we want to reassign
	// back to be the new back.
	if (node == d_back_ptr)
	{
		d_back_ptr = node_prev;

		if (node_prev)
			node_prev->next = static_cast<GList*>(NULL);
	}


	// delete the node itself.
	free_data(node);
	node->prev = node->next = static_cast<GList*>(NULL);
	g_list_free(node);


	// mark that we have deleted the node by reducing our size by one.
	d_size--;
}


void
AiksaurusGTK_strlist::clear()
{
	for(GList* iterator = d_front_ptr; iterator != static_cast<GList*>(NULL); iterator = iterator->next)
		free_data(iterator);

	g_list_free(d_front_ptr);

	d_front_ptr = d_back_ptr = static_cast<GList*>(NULL);

    d_size = 0;
}



//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   Stack-Like Routines                                                //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

void
AiksaurusGTK_strlist::push_front(const char* str)
{
	// create new node from string.
	GList* node = create_node(str);

	// if nodes exist, prepend node to begin of list.
	if (d_front_ptr)
	{
		node->next = d_front_ptr;
		d_front_ptr->prev = node;
		d_front_ptr = node;
	}

	// else if no nodes, init list to single node.
	else
	{
		d_front_ptr = d_back_ptr = node;
	}

	// we have now added a node, so increase size by 1.
	d_size++;
}

void
AiksaurusGTK_strlist::push_back(const char* str)
{
	// create new node from string.
	GList* node = create_node(str);

	// if nodes exist, append node to end of list.
	if (d_back_ptr)
	{
		node->prev = d_back_ptr;
		d_back_ptr->next = node;
		d_back_ptr = node;
	}

	// else if no nodes, init list to single node.
	else
	{
		d_front_ptr = d_back_ptr = node;
	}

	// we have now added a node, so increase size by 1.
	d_size++;
}




void
AiksaurusGTK_strlist::pop_front()
{
	// first ensure that we have elements.  if not, nothing to do.
	if (!d_front_ptr)
		return;

	remove_node(d_front_ptr);
}


void
AiksaurusGTK_strlist::pop_back()
{
	// first ensure that we have elements.  if not, nothing to do.
	if (!d_back_ptr)
		return;

	remove_node(d_back_ptr);
}









//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   Deletion Routines                                                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

GList* AiksaurusGTK_strlist::find_first(const char* str)
{
	for(GList* iterator = d_front_ptr; iterator != static_cast<GList*>(NULL); iterator = iterator->next)
	{
		char* x = static_cast<char*>(iterator->data);

		if (AiksaurusGTK_strEquals(x, str))
		{
			return iterator;
		}
	}

	return static_cast<GList*>(NULL);
}

void AiksaurusGTK_strlist::remove_first(const char* str)
{
	GList* node = find_first(str);

	if (node)
		remove_node(node);
}






//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   Debugging Routines                                                 //
//                                                                      //
//////////////////////////////////////////////////////////////////////////
#ifndef NDEBUG

#include <iostream>
using namespace std;

void AiksaurusGTK_strlist::debug()
{
    cout << "Strlist Contents: (" << size() << " elements)\n";
    for(GList* i = d_front_ptr;i != static_cast<GList*>(NULL);i = i->next)
    {
        cout << "  " << i << ": " << static_cast<char*>(i->data) << "\n";
    }
}

#endif // NDEBUG
