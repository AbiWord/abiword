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

#include "AiksaurusGTK_history.h"
#include "AiksaurusGTK_strlist.h"
#include "AiksaurusGTK_utils.h"

#include <cassert>

#ifndef NDEBUG
	#include <iostream>
	using namespace std;
#endif

//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   Creation and Destruction                                           //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

AiksaurusGTK_history::AiksaurusGTK_history()
{
	d_forward_tip_ptr = d_back_tip_ptr = d_current_ptr = static_cast<char*>(NULL);
}


AiksaurusGTK_history::~AiksaurusGTK_history()
{
	if (d_current_ptr)
		delete[] d_current_ptr;

	if (d_forward_tip_ptr)
		delete[] d_forward_tip_ptr;

	if (d_back_tip_ptr)
		delete[] d_back_tip_ptr;
}


const char*
AiksaurusGTK_history::tip_forward() const
{
	static const char* forward = "Forward";
	static const char* forwardto = "Forward to ";

	const char* nextone = d_forward.look_front();

	if (!nextone)
	{
		return forward;
	}

	if (d_forward_tip_ptr)
		delete[] d_forward_tip_ptr;

	d_forward_tip_ptr = AiksaurusGTK_strConcat(forwardto, nextone);

	return d_forward_tip_ptr;
}

const char*
AiksaurusGTK_history::tip_back() const
{
	static const char* back = "Back";
	static const char* backto = "Back to ";

	const char* backone = d_back.look_front();

	if (!backone)
	{
		return back;
	}

	if (d_back_tip_ptr)
	{
		delete[] d_back_tip_ptr;
		d_back_tip_ptr = 0;
	}

	d_back_tip_ptr = AiksaurusGTK_strConcat(backto, backone);

	return d_back_tip_ptr;
}


void
AiksaurusGTK_history::search(const char* str)
{
	// eliminate all entries which are in forward.
	d_forward.clear();

	// push current entry to top of back.
	if (d_current_ptr != NULL)
	{
		d_back.push_front(d_current_ptr);
		delete[] d_current_ptr;
	}

	// make current element mirror str.
	d_current_ptr = AiksaurusGTK_strCopy(str);
}


void
AiksaurusGTK_history::move_back()
{
	// make sure there is something to go back to before continuing.
	if (!d_back.size())
		return;

	// make current element become first element of forward.
	d_forward.push_front(d_current_ptr);

    while (d_forward.size() > 200)
        d_forward.pop_back();

	// make first element of back become current.
	delete[] d_current_ptr;
	d_current_ptr = AiksaurusGTK_strCopy(d_back.look_front());

	// pop first element of back
	d_back.pop_front();
}


void
AiksaurusGTK_history::move_forward()
{
	// make sure there is something to move forward to.
	if (!d_forward.size())
		return;

	// make current element become first element of back.
	d_back.push_front(d_current_ptr);

    while (d_back.size() > 200)
        d_back.pop_back();

	// make first element of forward become current.
	delete[] d_current_ptr;
	d_current_ptr = AiksaurusGTK_strCopy(d_forward.look_front());

	// pop first element of forward.
	d_forward.pop_front();
}


void
AiksaurusGTK_history::move_back_to(GList* element)
{
    int back_steps = 0;
    for(GList* itor = const_cast<GList*>(d_back.list()); itor != NULL; itor = itor->next)
    {
        ++back_steps;

        if (itor == element)
        {
            for(int i = 0;i < back_steps;++i)
                move_back();

            return;
        }
    }

    #ifndef NDEBUG
    cout << "AiksaurusGTK_history::move_back_to(" << element << ")\n"
         << "Warning: element is not in back list, and it should be.\n";
    debug();
    #endif // NDEBUG
}


void
AiksaurusGTK_history::move_forward_to(GList* element)
{
    int forward_steps = 0;
    for(GList* itor = const_cast<GList*>(d_forward.list()); itor != NULL; itor = itor->next)
    {
        ++forward_steps;

        if (itor == element)
        {
            for(int i = 0;i < forward_steps;++i)
                move_forward();

            return;
        }
    }

    #ifndef NDEBUG
    cout << "AiksaurusGTK_history::move_forward_to(" << element << ")\n"
         << "Warning: element is not in forward list, and it should be.\n";
    debug();
    #endif // NDEBUG
}


unsigned int
AiksaurusGTK_history::size_back() const
{
	return d_back.size();
}


unsigned int
AiksaurusGTK_history::size_forward() const
{
	return d_forward.size();
}


const char*
AiksaurusGTK_history::current() const
{
	return d_current_ptr;
}


const AiksaurusGTK_strlist&
AiksaurusGTK_history::list_back() const
{
	return d_back;
}


const AiksaurusGTK_strlist&
AiksaurusGTK_history::list_forward() const
{
	return d_forward;
}










#ifndef NDEBUG
void AiksaurusGTK_history::debug()
{
    cout << "History Debug Information ======================" << endl;
	cout << tip_back() << "      " << tip_forward() << endl;
	cout << "Current: " << current() << endl;

	cout << "Back ";
	d_back.debug();

	cout << "Forward: ";
	d_forward.debug();
    cout << "================================================" << endl;
}
#endif
