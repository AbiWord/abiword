/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
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
 
#ifndef UTVECTOR_H
#define UTVECTOR_H

#include "ut_types.h"
#include "ut_assert.h"

// TODO change the 'int' types to 'UT_[su]int32' whichever is appropriate.

// ----------------------------------------------------------------
/*
	The following class is a simple, portable implementation of a vector.
	Following in Mozilla's footsteps, we don't use STL because templates 
	are not yet portable enough for our needs.  (Same goes for exceptions
	and namespaces, BTW.)
*/
class UT_Vector
{
public:
	UT_Vector();
	~UT_Vector();

	UT_sint32	addItem(void*);
	UT_sint32	addItem(void* p, UT_uint32 * pIndex);
	void*		getNthItem(UT_uint32 n) const;
	UT_sint32	setNthItem(UT_uint32 ndx, void * pNew, void ** ppOld);
	void*		getFirstItem() const;
	void*		getLastItem() const;
	UT_uint32	getItemCount() const;
	UT_sint32	findItem(void*) const;

	UT_sint32	insertItemAt(void*, UT_uint32 ndx);
	void		deleteNthItem(UT_uint32 n);
	void		clear();
	void		qsort(int (*compar)(const void *, const void *));

protected:
	UT_uint32		calcNewSpace();
	UT_sint32		grow(UT_uint32);
	
	void**			m_pEntries;
	UT_uint32		m_iCount;
	UT_uint32		m_iSpace;
	UT_uint32		m_iCutoffDouble;
	UT_uint32		m_iPostCutoffIncrement;
};

// NB: this macro is useful only in destructors
#define UT_VECTOR_PURGEALL(d, v)						\
	do	{	int utv_max = v.getItemCount();				\
			for (int utv=utv_max-1; utv>=0; utv--)		\
			{											\
				d utv_p = (d) v.getNthItem(utv);		\
				UT_ASSERT(utv_p);						\
				if (utv_p)								\
					delete utv_p;						\
			}											\
	} while (0)

// NB: this macro is useful only in destructors
#define UT_VECTOR_FREEALL(d, v)							\
	do	{	int utv_max = v.getItemCount();				\
			for (int utv=utv_max-1; utv>=0; utv--)		\
			{											\
				d utv_p = (d) v.getNthItem(utv);		\
				UT_ASSERT(utv_p);						\
				if (utv_p)								\
					free(utv_p);						\
			}											\
	} while (0)

#endif /* UTVECTOR_H */
