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


#include <windows.h>

#include "ut_Win32Timer.h"
#include "ut_assert.h"

/*****************************************************************/
	
UT_Timer* UT_Timer::static_constructor(UT_TimerCallback pCallback, void* pData)
{
	UT_ASSERT(pCallback);
	UT_Win32Timer * p = new UT_Win32Timer(pCallback,pData);

	return p;
}

UT_Win32Timer::UT_Win32Timer(UT_TimerCallback pCallback, void* pData)
{
	setCallback(pCallback);
	setInstanceData(pData);
	m_bStarted = UT_FALSE;
	m_iMilliseconds = 0;
}


/*****************************************************************/

VOID CALLBACK _Win32TimerProc(HWND hwnd, 
						UINT uMsg, 
						UINT idEvent, 
						DWORD dwTime)
{
	UT_Timer* pMyTimer = UT_Timer::findTimer(idEvent);
	UT_ASSERT(pMyTimer);

	pMyTimer->fire();
}

UT_Win32Timer::~UT_Win32Timer()
{
	stop();
}

UT_sint32 UT_Win32Timer::set(UT_uint32 iMilliseconds)
{
	// set the freq and start firing events.
	
	UINT idTimer = SetTimer(NULL, 0, iMilliseconds, (TIMERPROC) _Win32TimerProc);
	if (idTimer == 0)
		return -1;
	
	m_iMilliseconds = iMilliseconds;
	m_bStarted = UT_TRUE;
	setIdentifier(idTimer);
	return 0;
}

void UT_Win32Timer::stop(void)
{
	// stop the delivery of timer events.
	// stop the OS timer from firing, but do not delete the class.
	
	UINT idTimer = getIdentifier();

	if (idTimer == 0)
		return;
	
	KillTimer(NULL, idTimer);
	m_bStarted = UT_FALSE;
}

void UT_Win32Timer::start(void)
{
	// resume the delivery of events using the last freq set.

	UT_ASSERT(m_iMilliseconds > 0);
	
	if (!m_bStarted)
		set(m_iMilliseconds);
}
