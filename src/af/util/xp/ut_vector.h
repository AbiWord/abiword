/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Program Utilities
 * Copyright (C) 1998-2000 AbiSource, Inc.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
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
#include "ut_debugmsg.h"
// ----------------------------------------------------------------

#define UT_VECTOR_CLEANUP(d, v, r) \
	do	{	int utv_max = v.getItemCount();				\
			for (int utv=utv_max-1; utv>=0; utv--)		\
			{											\
				d utv_p = (d) v.getNthItem(utv);		\
				UT_ASSERT_HARMLESS(utv_p);				\
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
#define UT_VECTOR_FREEALL(d, v) UT_VECTOR_CLEANUP(d, v, g_free)
#define UT_VECTOR_SPARSEPURGEALL(d, v) UT_VECTOR_SPARSECLEANUP(d, v, delete)
#define UT_VECTOR_SPARSEFREEALL(d, v) UT_VECTOR_SPARSECLEANUP(d, v, g_free)

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


template <class T> class UT_GenericVector
{
public:
	typedef int (*compar_fn_t) (const void *, const void *);

	// Real-life tests shown that vast majority of our vectors contain less than 4 items
	// and virtually all contains less than 32 elements; I have adjusted the initial
	// values accordingly. Where vectors are known to be larger, bigger values should be
	// passed to the constructor, and, if approprite, pre-allocation forced
	UT_GenericVector(UT_sint32 sizehint = 32, UT_sint32 baseincr = 4, bool bPrealloc = false);
	UT_GenericVector(const UT_GenericVector<T>&);
	UT_GenericVector<T>& operator=(const UT_GenericVector<T>&);
	virtual ~UT_GenericVector();

	UT_sint32	addItem(const T);
	inline UT_sint32	push_back(const T item)	{ return addItem(item); }
	bool				pop_back();
	inline const T	back() const			{ return getLastItem(); }

	UT_sint32	addItem(const T p, UT_sint32 * pIndex);

	/* FIXME -- this function assumes that it is possible to do
	 *          static_cast<T>(0)
	 */
	inline T getNthItem(UT_sint32 n) const
	{
	    UT_ASSERT_HARMLESS(m_pEntries);
	    UT_ASSERT_HARMLESS(m_iCount > 0);
	    UT_ASSERT_HARMLESS(n<m_iCount);

	    if(n >= m_iCount || !m_pEntries) {
			return 0;
		}
	    return m_pEntries[n];
	}

	const T		operator[](UT_sint32 i) const;
	UT_sint32	setNthItem(UT_sint32 ndx, T pNew, T * ppOld);
	const T		getFirstItem() const;
	const T		getLastItem() const;
	inline UT_sint32 getItemCount() const {	return m_iCount; }
	UT_sint32	findItem(T) const;

	UT_sint32	insertItemAt(T, UT_sint32 ndx);
	UT_sint32   addItemSorted(const T p, int (*compar)(const void *, const void *));
	void		deleteNthItem(UT_sint32 n);
	void		clear();
	void		qsort(int (*compar)(const void *, const void *));
	UT_sint32	binarysearch(const void* key, int (*compar)(const void *, const void *)) const;

	bool		copy(const UT_GenericVector<T> *pVec);
	inline UT_sint32 size() const { return getItemCount(); }

private:
	UT_sint32		grow(UT_sint32);
	UT_sint32		binarysearchForSlot(const void* key, compar_fn_t compar) const;

	T*			m_pEntries;
	UT_sint32		m_iCount;
	UT_sint32		m_iSpace;
	UT_sint32		m_iCutoffDouble;
	UT_sint32		m_iPostCutoffIncrement;
};

// TODO Rob: try to export like this once plugin loading is fixed:
// template class ABI_EXPORT UT_GenericVector<void const *>;
class ABI_EXPORT UT_Vector : public UT_GenericVector<void const *> {
public:
	UT_Vector(UT_sint32 sizehint = 32, UT_sint32 baseincr = 4, bool bPrealloc = false)
	: UT_GenericVector<void const *>(sizehint, baseincr, bPrealloc)
	{}
};

// TODO Rob: try to export like this once plugin loading is fixed:
// template class ABI_EXPORT UT_GenericVector<UT_sint32>;
class ABI_EXPORT UT_NumberVector : public UT_GenericVector<UT_sint32> {
public:
	UT_NumberVector(UT_sint32 sizehint = 32, UT_sint32 baseincr = 4, bool bPrealloc = false)
	: UT_GenericVector<UT_sint32>(sizehint, baseincr, bPrealloc)
	{}
};

#include <stdlib.h>
#include <string.h>

/*!
    sizehint: expected size of the vector

    baseincr: the amount by which the internal storage will grow once the sizehint has
              been reached (until then, the size gets doubled)

    bPrealoc: if true, we immediately allocate storage of at least sizehint (otherwise the
              space will be allocated when first item is inserted to baseincr)
 */
template <class T>
UT_GenericVector<T>::UT_GenericVector(UT_sint32 sizehint, UT_sint32 baseincr, bool bPrealoc)
  : m_pEntries(NULL), m_iCount(0), m_iSpace(0),
    m_iCutoffDouble(sizehint), m_iPostCutoffIncrement(baseincr)
{
	if(bPrealoc)
		grow(sizehint);
}

template <class T>
UT_GenericVector<T>::UT_GenericVector(const UT_GenericVector<T>& utv)
  : m_pEntries(NULL), m_iCount(0), m_iSpace(0),
  m_iCutoffDouble(utv.m_iCutoffDouble),
  m_iPostCutoffIncrement(utv.m_iPostCutoffIncrement)
{
	copy(&utv);
}

template <class T>
UT_GenericVector<T>& UT_GenericVector<T>::operator=(const UT_GenericVector<T>& utv)
{
	if(this != &utv)
	{
		m_iCutoffDouble = utv.m_iCutoffDouble;
		m_iPostCutoffIncrement = utv.m_iPostCutoffIncrement;
		copy(&utv);
	}
	return *this;
}

template <class T>
void UT_GenericVector<T>::clear()
{
	if(m_iCount > m_iCutoffDouble)
	{
		xxx_UT_DEBUGMSG(("Vector contained %d entries, allocated %d slots\n", m_iCount, m_iSpace));
	}

	m_iCount = 0;
	memset(m_pEntries, 0, m_iSpace * sizeof(T));
}


template <class T>
UT_GenericVector<T>::~UT_GenericVector()
{
	if(m_iCount >  m_iCutoffDouble)
	{
		xxx_UT_DEBUGMSG(("Vector contained %d entries, allocated %d slots\n", m_iCount, m_iSpace));
	}

	FREEP(m_pEntries);
}

/*
 This function is called everytime we want to insert a new element but don't have
 enough space.  In this case we grow the array to be _at least_ ndx size.
*/
template <class T>
UT_sint32 UT_GenericVector<T>::grow(UT_sint32 ndx)
{
	UT_sint32 new_iSpace;
	if(!m_iSpace) {
		new_iSpace = m_iPostCutoffIncrement;
	}
	else if (m_iSpace < m_iCutoffDouble) {
		xxx_UT_DEBUGMSG(("Vector growing (%d -> %d\n", m_iSpace, ndx));
		new_iSpace = m_iSpace * 2;
	}
	else {
		new_iSpace = m_iSpace + m_iPostCutoffIncrement;
	}

	if (new_iSpace < ndx)
	{
		new_iSpace = ndx;
	}

	T * new_pEntries = static_cast<T *>(g_try_realloc(m_pEntries, new_iSpace * sizeof(T)));
	if (!new_pEntries) {
		return -1;
	}
	//Is this required? We always check Count first anyways.
	// TMN: Unfortunately it is, since the class GR_CharWidths
	// uses UT_GenericVector<T> as a sparse array!
	memset(&new_pEntries[m_iSpace], 0, (new_iSpace - m_iSpace) * sizeof(T));
	m_iSpace = new_iSpace;
	m_pEntries = new_pEntries;

	return 0;
}

template <class T>
UT_sint32 UT_GenericVector<T>::insertItemAt(const T p, UT_sint32 ndx)
{
	if (ndx > m_iCount + 1)
		return -1;

	if ((m_iCount+1) > m_iSpace)
	{
		UT_sint32 err = grow(0);
		if (err)
		{
			return err;
		}
	}

	// bump the elements -> thataway up to the ndxth position
	memmove(&m_pEntries[ndx+1], &m_pEntries[ndx], (m_iCount - ndx) * sizeof(T));

	m_pEntries[ndx] = (T)p;
	++m_iCount;

	return 0;
}

template <class T>
UT_sint32 UT_GenericVector<T>::addItem(const T p, UT_sint32 * pIndex)
{
	UT_sint32 err = addItem(p);
	if (!err && pIndex)
		*pIndex = m_iCount-1;
	return err;
}

template <class T>
UT_sint32 UT_GenericVector<T>::addItem(const T p)
{
	if ((m_iCount+1) > m_iSpace)
	{
		UT_sint32 err = grow(0);
		if (err)
		{
			return err;
		}
	}

	m_pEntries[m_iCount++] = (T)p;  /*** bad, cast away const so we can build again ***/

	return 0;
}

template <class T>
UT_sint32 UT_GenericVector<T>::addItemSorted(const T p, int (*compar)(const void *, const void *))
{
	if (!m_iCount) {
		return addItem(p);
	}

	return insertItemAt( p, binarysearchForSlot((static_cast<void*>(const_cast<T*>(&p))), compar));
}

/** It returns true if there were no errors, false elsewhere */
template <class T>
bool UT_GenericVector<T>::pop_back()
{
	if (m_iCount > 0)
	{
		--m_iCount;
		return true;
	}
	else
		return false;
}

template <class T>
UT_sint32 UT_GenericVector<T>::setNthItem(UT_sint32 ndx, T pNew, T* ppOld)
{
	const UT_sint32 old_iSpace = m_iSpace;

	if (ndx >= m_iSpace)
	{
		const UT_sint32 err = grow(ndx+1);
		if (err)
		{
			return err;
		}
	}

	if (ppOld)
	{
		*ppOld = (ndx < old_iSpace) ? m_pEntries[ndx] : 0;
	}

	m_pEntries[ndx] = pNew;
	if (ndx >= m_iCount)
	{
		m_iCount = ndx + 1;
	}

	return 0;
}

template <class T>
const T UT_GenericVector<T>::getLastItem() const
{
	UT_ASSERT_HARMLESS(m_iCount > 0);

	return m_pEntries[m_iCount-1];
}

template <class T>
const T UT_GenericVector<T>::getFirstItem() const
{
	UT_ASSERT_HARMLESS(m_iCount > 0);
	UT_ASSERT_HARMLESS(m_pEntries);
	if (!m_pEntries) {
		return T();
	}
	return m_pEntries[0];
}

template <class T>
void UT_GenericVector<T>::deleteNthItem(UT_sint32 n)
{
	UT_ASSERT_HARMLESS(n < m_iCount);
	UT_ASSERT_HARMLESS(m_iCount > 0);

	memmove(&m_pEntries[n], &m_pEntries[n+1], (m_iCount - (n + 1)) * sizeof(T));

	m_pEntries[m_iCount-1] = 0;
	m_iCount--;

	return;
}

template <class T>
UT_sint32 UT_GenericVector<T>::findItem(T p) const
{
	for (UT_sint32 i=0; i<m_iCount; i++)
	{
		if (m_pEntries[i] == p)
		{
			return i;
		}
	}

	return -1;
}

template <class T>
void UT_GenericVector<T>::qsort(int (*compar)(const void *, const void *))
{
	::qsort(m_pEntries, m_iCount, sizeof(T), compar);
}

// this binary search finds the earliest element (lowest index)
// in the vector which matches the key
// based on code from Tim Bray's 'On the Goodness of Binary Search'
// http://tbray.org/ongoing/When/200x/2003/03/22/Binary

template <class T>
UT_sint32 UT_GenericVector<T>::binarysearch(const void* key, compar_fn_t compar) const
{
	UT_sint32 slot = binarysearchForSlot(key, compar);

	if ((slot == m_iCount) || (0 != (*compar)(key, &m_pEntries[slot])))
		return -1;
	else
		return slot;
}

template <class T>
UT_sint32 UT_GenericVector<T>::binarysearchForSlot(const void* key, compar_fn_t compar) const
{
	UT_sint32 high = m_iCount;
	UT_sint32 low = -1;
	UT_sint32 probe;

	while (high - low > 1)
	{
		int res;
		probe = (high + low) / 2;
		res = (*compar)(key, &m_pEntries[probe]);
		if (0 < res)
			low = probe;
		else
			high = probe;
	}

	return high;
}

template <class T>
bool UT_GenericVector<T>::copy(const UT_GenericVector<T> *pVec)
{
	clear();

	if (!pVec) {
		return false;
	}

	for (UT_sint32 i=0; i < pVec->m_iCount; i++)
	{
		UT_sint32 err;

		err = addItem(pVec->m_pEntries[i]);
		if(err == -1)
			return (err ? true : false);
	}

	return false;
}

template <class T>
const T UT_GenericVector<T>::operator[](UT_sint32 i) const
{
	return this->getNthItem(i);
}


#endif /* UTVECTOR_H */
