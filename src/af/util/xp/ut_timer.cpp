 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiSource Utilities.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/

#include "ut_timer.h"

#include "ut_assert.h"

UT_Timer::UT_Timer()
{
	m_pCallback = NULL;
	m_pInstanceData = NULL;
	m_iIdentifier = 0;
	
	static_vecTimers.addItem(this);
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
