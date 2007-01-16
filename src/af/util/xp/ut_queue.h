/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Program Utilities
 * Copyright (C) 2004 Tomas Frydrych <tomasfrydrych@yahoo.co.uk>
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
 


#ifndef UT_QUEUE_H
#define UT_QUEUE_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
#include "ut_vector.h"

template <class T> class ABI_EXPORT UT_GenericQueue
{
public:
	bool      push(T t);
	bool      pop(T &t);
	bool      front(T &t) const;
	bool      back(T &t) const;
	UT_uint32 size() const;
	void      clear();
	
private:
	UT_GenericVector<T> m_vecQueue;
};


template <class T> bool UT_GenericQueue<T>::push(T t)
{
	return (m_vecQueue.insertItemAt(t, 0) >= 0);
}

template <class T> bool UT_GenericQueue<T>::pop(T &t)
{
	UT_uint32 i = m_vecQueue.getItemCount();
	if(!i)
		return false;
	i--;
	
	t = m_vecQueue.getNthItem(i);
	m_vecQueue.deleteNthItem(i);
	return true;
}

template <class T> bool UT_GenericQueue<T>::front(T &t) const
{
	UT_uint32 i = m_vecQueue.getItemCount();
	if(!i)
		return false;
	i--;
	
			
	t = m_vecQueue.getNthItem(i);
	return true;
}

template <class T> bool UT_GenericQueue<T>::back(T &t) const
{
	UT_uint32 i = m_vecQueue.getItemCount();
	if(!i)
		return false;
			
	t = m_vecQueue.getNthItem(0);
	return true;
}

template <class T> UT_uint32 UT_GenericQueue<T>::size() const
{
	return m_vecQueue.getItemCount();
}

template <class T> void UT_GenericQueue<T>::clear()
{
	m_vecQueue.clear();
}
#endif /* UT_QUEUE_H */
