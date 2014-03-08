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

#ifndef INCLUDED_JARED_GPL_AIKSAURUSGTK_STRLIST_H
#define INCLUDED_JARED_GPL_AIKSAURUSGTK_STRLIST_H

#include <glib.h>

//
// AiksaurusGTK_strlist class
// --------------------------
//   This is a wrapper for the GList class which provides string copying
//   features so that you can add strings and store them directly in the list
//   with a minimum of fuss.
//
//   In other words, when you push_back() or push_front() strings to this list,
//   a copy of the string will be put in the list.  This makes it much easier 
//   to reuse pointers and so forth, and you don't have to remember to delete 
//   the strings you've added because the destructor does it for you.
//   

class AiksaurusGTK_strlist
{
	private:
	
		// d_list_ptr: pointer to the actual list itself.
		GList* d_front_ptr;
		GList* d_back_ptr;

		// d_size: stores number of elements in list.
		int d_size;
		
		GList* find_first(const char* str);

		// do not allow copying.
		AiksaurusGTK_strlist(const AiksaurusGTK_strlist& rhs);
		const AiksaurusGTK_strlist& operator=(const AiksaurusGTK_strlist& rhs);
	
		GList* create_node(const char* str) const;
		void free_data(GList* node) const;
		
		void remove_node(GList* node);
		
	public:

		AiksaurusGTK_strlist();
		~AiksaurusGTK_strlist();
		
		
		
		// size: returns number of elements in list.
		unsigned int size() const;
		
		// getList: returns ptr to actual glist.
		const GList* list() const;
		
		// look_back: returns pointer to last element in list
		// or NULL if list is empty.
		const char* look_back() const;
		
		// look_front: returns pointer to first element in list
		// or NULL if list is empty.
		const char* look_front() const;
		

		
		// clear: destroy all data in glist.
		void clear();
		
		// push_back: adds str to end of list.
		void push_back(const char* str);
		
		// pop_back: removes last element of list, or does
		// nothing if list is empty.
		void pop_back();
		
		// push_front: adds str to front of list.
		void push_front(const char* str);

		// pop_front: removes first element from list, or does
		// nothing if list is empty.
		void pop_front();
		

		
		// remove_first: removes first instance of str in list if found.
		void remove_first(const char* str);
	
		#ifndef NDEBUG
		// debug: prints debug information.
		void debug();
		#endif
};

#endif // INCLUDED_JARED_GPL_AIKSAURUSGTK_STRLIST_H
