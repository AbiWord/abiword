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
 

#include "ut_timer.h"
#include "ut_assert.h"

UT_Timer::UT_Timer()
	: m_pCallback(0),
	  m_pInstanceData(0),
	  m_iIdentifier(0)
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

// declare static member
UT_Vector UT_Timer::static_vecTimers;

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
	m_pInstanceData = p;
}

void* UT_Timer::getInstanceData()
{
	return m_pInstanceData;
}

void UT_Timer::setCallback(UT_TimerCallback pCallback)
{
	m_pCallback = pCallback;
}

UT_TimerCallback UT_Timer::getCallback()
{
	return m_pCallback;
}

UT_Timer* UT_Timer::findTimer(UT_uint32 iIdentifier)
{
	int count = static_vecTimers.getItemCount();
	for (int i=0; i<count; i++)
	{
		UT_Timer* pTimer = (UT_Timer*) static_vecTimers.getNthItem(i);
		UT_ASSERT(pTimer);
		
		if (pTimer->getIdentifier() == iIdentifier)
		{
			return pTimer;
		}
	}

	return NULL;
}

void UT_Timer::fire()
{
	UT_ASSERT(m_pCallback);
	
	m_pCallback(this);
}
