/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Program Utilities
 * Copyright (C) 1998-2003 AbiSource, Inc.
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

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
#include "ut_assert.h"

// ----------------------------------------------------------------
/*
	The following class is a simple, portable implementation of a vector.
	Following in Mozilla's footsteps, we don't use STL because templates 
	are not yet portable enough for our needs.  (Same goes for exceptions
	and namespaces, BTW.)
*/

#define UT_VECTOR_CLEANUP(d, v, r) \
	do	{	int utv_max = v.getItemCount();				\
			for (int utv=utv_max-1; utv>=0; utv--)		\
			{											\
				d utv_p = (d) v.getNthItem(utv);		\
				UT_ASSERT(utv_p);						\
				if (utv_p)								\
					r (utv_p);						\
			}											\
	} while (0)

#define UT_VECTOR_SPARSECLEANUP(d, v, r) \
	do	{	int utv_max = v.getItemCount();				\
			for (int utv=utv_max-1; utv>=0; utv--)		\
			{											\
				d utv_p = (d) v.getNthItem(utv);		\
				if (utv_p)								\
					r (utv_p);						\
			}											\
	} while (0)

#define UT_VECTOR_PURGEALL(d, v) UT_VECTOR_CLEANUP(d, v, delete)
#define UT_VECTOR_FREEALL(d, v) UT_VECTOR_CLEANUP(d, v, free)
#define UT_VECTOR_SPARSEPURGEALL(d, v) UT_VECTOR_SPARSECLEANUP(d, v, delete)
#define UT_VECTOR_SPARSEFREEALL(d, v) UT_VECTOR_SPARSECLEANUP(d, v, free)

/* don't call this macro unless you are in Obj-C++ */
/* release any non nil objective-C object of the array */
#define UT_VECTOR_RELEASE(v) \
	{						\
		int utv_max = v.getItemCount();				\
			for (int utv=utv_max-1; utv>=0; utv--)		\
			{											\
				id utv_p = (id) v.getNthItem(utv);		\
				[utv_p release];								\
			}										\
	}


#ifndef ABI_OPT_STL
class ABI_EXPORT UT_Vector
{
public:
	UT_Vector(UT_uint32 sizehint = 2048);
	UT_Vector(const UT_Vector&);
	UT_Vector& operator=(const UT_Vector&);
	~UT_Vector();

	UT_sint32	addItem(const void*);
	inline UT_sint32	push_back(void *item)	{ return addItem(item); }
	bool				pop_back();
	inline const void*	back() const			{ return getLastItem(); }
	 
	UT_sint32	addItem(const void* p, UT_uint32 * pIndex);
	inline void*getNthItem(UT_uint32 n) const
	{
	    UT_ASSERT(m_pEntries);
	    UT_ASSERT(m_iCount > 0);
	    UT_ASSERT(n<m_iCount);

	    if(n >= m_iCount || !m_pEntries) return NULL;
	    return m_pEntries[n];
	}

	const void*		operator[](UT_uint32 i) const;
	UT_sint32	setNthItem(UT_uint32 ndx, void * pNew, void ** ppOld);
	void*		getFirstItem() const;
	void*		getLastItem() const;
	inline UT_uint32 getItemCount() const {	return m_iCount; }
	UT_sint32	findItem(void*) const;

	UT_sint32	insertItemAt(void*, UT_uint32 ndx);
	void		deleteNthItem(UT_uint32 n);
	void		clear();
	void		qsort(int (*compar)(const void *, const void *));

	bool		copy(const UT_Vector *pVec);
	inline UT_uint32 size() const { return getItemCount(); }

private:
	UT_sint32		grow(UT_uint32);
	
	void**			m_pEntries;
	UT_uint32		m_iCount;
	UT_uint32		m_iSpace;
	UT_uint32		m_iCutoffDouble;
	UT_uint32		m_iPostCutoffIncrement;
};

#else /* ABI_OPT_STL */

#include <vector>
#include <algorithm>

class ABI_EXPORT UT_Vector
{
 public:
	UT_Vector(UT_uint32 sizehint = 2048);
	~UT_Vector();

	UT_sint32	addItem(const void*);
	UT_sint32	addItem(const void* p, UT_uint32 * pIndex);
	void*		getNthItem(UT_uint32 n) const;
	const void*	operator[](UT_uint32 i) const;
	UT_sint32	setNthItem(UT_uint32 ndx, void * pNew, void ** ppOld);
	void*	getFirstItem() const;
	void*	getLastItem() const;
	UT_uint32	getItemCount() const;
	UT_sint32	findItem(void*) const;

	UT_sint32	insertItemAt(void*, UT_uint32 ndx);
	void		deleteNthItem(UT_uint32 n);
	void		clear();
	void		qsort(int (*compar)(const void *, const void *));

	bool		copy(UT_Vector *pVec);
	UT_uint32 inline size() const { return getItemCount(); }
	inline UT_sint32	push_back(void *item)	{ m_STLVec.push_back(item); return 0; }
	inline bool			pop_back()
    {
		m_STLVec.pop_back();
		return true;
	}
	inline void*		back() const			{ return m_STLVec.back(); }

private:
	vector<void *>  m_STLVec;
};

#endif /* ABI_OPT_STL */

class ABI_EXPORT UT_NumberVector
{
public:
	/* sizehint: initial size of internal array
	 * baseincr: size of increments
	 */
	UT_NumberVector (UT_uint32 sizehint = 2048, UT_uint32 baseincr = 256);
	UT_NumberVector (const UT_NumberVector & NV);

	~UT_NumberVector ();

	UT_NumberVector &	operator= (const UT_NumberVector & NV);

	bool				copy (const UT_NumberVector & NV);

	void				clear (bool free_memory = true);

	inline UT_uint32	size () const
	{
		return m_iCount;
	}
	inline UT_uint32	getItemCount() const
	{
		return m_iCount;
	}

	/* addItem() & push_back() return 0 on success, -1 on failure:
	 */
	inline UT_sint32	addItem (UT_sint32 number, UT_uint32 * pIndex)
	{
		if (addItem (number)) return -1;
		if (pIndex)
			*pIndex = m_iCount - 1;
		return 0;
	}
	UT_sint32			addItem (UT_sint32 number);

	inline UT_sint32	push_back (UT_sint32 number)
	{
		return addItem (number);
	}
	inline bool			pop_back ()
	{
		if (!m_iCount) return false;
		m_pEntries[--m_iCount] = 0;
		return true;
	}

	inline UT_sint32	operator[] (UT_uint32 index) const
	{
		return getNthItem (index);
	}
	inline UT_sint32	getFirstItem () const
	{
		return getNthItem (0);
	}
	inline UT_sint32	getLastItem () const
	{
		return getNthItem (m_iCount ? (m_iCount - 1) : 0);
	}
	inline UT_sint32	back () const // == getLastItem
	{
		return getNthItem (m_iCount ? (m_iCount - 1) : 0);
	}
	UT_sint32			getNthItem (UT_uint32 index) const;

	/* setNthItem() and insertItemAt() return 0 on success, -1 on failure:
	 */
	UT_sint32			setNthItem (UT_uint32 index, UT_sint32 new_number, UT_sint32 * old_number = 0);

	UT_sint32			insertItemAt (UT_sint32 number, UT_uint32 index);

	void				deleteNthItem (UT_uint32 index);

	/* findItem() returns index >= 0 of first instance of number, or -1 if not found:
	 */
	UT_sint32			findItem (UT_sint32 number) const;

	// void				qsort (bool ascending = true);

private:
	/* grow() returns 0 on success, -1 on failure:
	 */
	UT_sint32			grow (UT_uint32 requirement);
	
	UT_sint32 *			m_pEntries;

	UT_uint32			m_iCount;
	UT_uint32			m_iSpace;
	UT_uint32			m_iInitialSize;
	UT_uint32			m_iIncrement;
};

#endif /* UTVECTOR_H */
