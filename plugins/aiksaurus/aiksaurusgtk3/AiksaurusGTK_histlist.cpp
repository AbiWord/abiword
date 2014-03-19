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

#include "AiksaurusGTK_histlist.h"
#include "AiksaurusGTK_strlist.h"

#ifndef NDEBUG
	#include <iostream>
	using namespace std;
#endif


AiksaurusGTK_histlist::AiksaurusGTK_histlist(unsigned int maxElements)
{
	d_maxElements = maxElements;
	d_list_ptr = new AiksaurusGTK_strlist;
}


AiksaurusGTK_histlist::~AiksaurusGTK_histlist()
{
	delete d_list_ptr;
}


void AiksaurusGTK_histlist::addItem(const char* str)
{
	d_list_ptr->remove_first(str);

	d_list_ptr->push_front(str);

	if (d_list_ptr->size() > d_maxElements)
	{
		d_list_ptr->pop_back();
	}
}


const GList* AiksaurusGTK_histlist::list() const
{
	return d_list_ptr->list();
}



#ifndef NDEBUG

	void AiksaurusGTK_histlist::debug()
	{
		cout << "AiksaurusGTK_histlist::debug() {" << endl;
		cout << "  MaxElements is " << d_maxElements << endl;
		cout << "  List information follows: " << endl;
		d_list_ptr->debug();
		cout << "}" << endl;
	}

#endif // NDEBUG

