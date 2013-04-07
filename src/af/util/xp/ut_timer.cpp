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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */
 

#include "ut_timer.h"
#include "ut_assert.h"

// declare static member
static UT_GenericVector<UT_Timer*> static_vecTimers;

UT_Timer::UT_Timer()
	: m_iIdentifier(0)
{
	static_vecTimers.addItem(this);
}

UT_Timer::~UT_Timer()
{
	UT_sint32 ndx = static_vecTimers.findItem(this);
	UT_ASSERT(ndx >= 0);

	if (ndx >= 0)
	{
		static_vecTimers.deleteNthItem(ndx);
	}
}

UT_GenericVector<UT_Timer*> & UT_Timer::_getVecTimers ()
{ 
	return static_vecTimers;
}

void UT_Timer::setIdentifier(UT_uint32 iIdentifier)
{
	m_iIdentifier = iIdentifier;
}

UT_uint32 UT_Timer::getIdentifier()
{
	return m_iIdentifier;
}

void UT_Timer::setInstanceData(void* p)
{
  _setInstanceData (p);
}

void UT_Timer::setCallback(UT_WorkerCallback pCallback)
{
  _setCallback (pCallback);
}

UT_Timer* UT_Timer::findTimer(UT_uint32 iIdentifier)
{
	int count = static_vecTimers.getItemCount();
	for (int i=0; i<count; i++)
	{
		UT_Timer* pTimer = static_vecTimers.getNthItem(i);
		UT_ASSERT(pTimer);
		
		if (pTimer->getIdentifier() == iIdentifier)
		{
			return pTimer;
		}
	}

	return NULL;
}
