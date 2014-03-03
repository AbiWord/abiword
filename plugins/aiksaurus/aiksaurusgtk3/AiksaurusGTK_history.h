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

#ifndef INCLUDED_GPL_JARED_AIKSAURUSGTK_HISTORY_H
#define INCLUDED_GPL_JARED_AIKSAURUSGTK_HISTORY_H

#include "AiksaurusGTK_strlist.h"

class AiksaurusGTK_history
{
	private:
		
		AiksaurusGTK_strlist d_back;
		AiksaurusGTK_strlist d_forward;

		AiksaurusGTK_history(const AiksaurusGTK_history& rhs);
		const AiksaurusGTK_history& operator=(const AiksaurusGTK_history& rhs);

		char* d_current_ptr;

        int d_maxentries;
        
		mutable char* d_forward_tip_ptr;
		mutable char* d_back_tip_ptr;
		
	public:

		AiksaurusGTK_history();
		~AiksaurusGTK_history();

		void search(const char* str);
		
		void move_back();
		void move_forward();

        void move_back_to(GList* element);
        void move_forward_to(GList* element);
        
		const char* tip_back() const;
		const char* tip_forward() const;
		
		const char* current() const;
		
		unsigned int size_back() const;
		unsigned int size_forward() const;
	
		const AiksaurusGTK_strlist& list_back() const;
		const AiksaurusGTK_strlist& list_forward() const;
		
		#ifndef NDEBUG
		void debug();
		#endif
};

#endif // INCLUDED_GPL_JARED_AIKSAURUS_HISTORY_H
