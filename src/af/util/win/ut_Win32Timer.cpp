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
#include "gr_Win32Graphics.h"

/*****************************************************************/
	
UT_Timer* UT_Timer::static_constructor(UT_TimerCallback pCallback, void* pData, GR_Graphics * pG)
{
	UT_ASSERT(pCallback);
	UT_Win32Timer * p = new UT_Win32Timer(pCallback,pData,pG);

	return p;
}

UT_Win32Timer::UT_Win32Timer(UT_TimerCallback pCallback, void* pData, GR_Graphics * pG)
{
	setCallback(pCallback);
	setInstanceData(pData);
	m_bStarted = UT_FALSE;
	m_iMilliseconds = 0;

	GR_Win32Graphics * pWinG = static_cast<GR_Win32Graphics *>(pG);
	
	m_hWnd = ((pWinG) ? pWinG->getHwnd() : 0);

	setIdentifier(_createIdentifier());
}


/*****************************************************************/

VOID CALLBACK Global_Win32TimerProc(HWND hwnd, 
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

	// WinNT support TimerProc. Win95 also use it, but the documentation 
	// say it also need a message dispatch procedure for WM_TIMER.
	// I'm going to pass enough information so that we should be able
	// to get back to our timer callback regardless of the OS.
	// SetTimer return the timer identifier. When we create a new timer
	// and the window handle is zero, the function generate an id 
	// regardless of the one supplied.
	// Identifier need to be 16 bits because certains printers drivers are
	// made to work with "Windows 3.1" ! Theses drivers filter out bits
	// higher than 16.
	
	UINT idTimer = SetTimer(m_hWnd, (UINT) getIdentifier(), iMilliseconds, (TIMERPROC) Global_Win32TimerProc);
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
	
	KillTimer(m_hWnd, idTimer);
	m_bStarted = UT_FALSE;
}

void UT_Win32Timer::start(void)
{
	// resume the delivery of events using the last freq set.

	UT_ASSERT(m_iMilliseconds > 0);
	
	if (!m_bStarted)
		set(m_iMilliseconds);
}

int UT_Win32Timer::_compareIdentifiers(const void* p1, const void* p2)
{
	UT_Win32Timer** ppTimer1 = (UT_Win32Timer**) p1;
	UT_Win32Timer** ppTimer2 = (UT_Win32Timer**) p2;

	if ((*ppTimer1)->getIdentifier() < (*ppTimer2)->getIdentifier())
	{
		return -1;
	}
	
	if ((*ppTimer1)->getIdentifier() > (*ppTimer2)->getIdentifier())
	{
		return 1;
	}
	
	return 0;
}

UT_uint32 UT_Win32Timer::_createIdentifier(void)
{
	UT_Timer::static_vecTimers.qsort(UT_Win32Timer::_compareIdentifiers);

	// Take the first unused identifier number different from zero
	UT_uint32 iIdentifier = 0;
	UT_uint32 count = static_vecTimers.getItemCount();
	for (UT_uint32 i=0; i<count; i++, iIdentifier++)
	{
		UT_Timer* pTimer = (UT_Timer*) static_vecTimers.getNthItem(i);
		UT_ASSERT(pTimer);
		
		UT_uint32 iTimerId = pTimer->getIdentifier();
		if (iTimerId && iTimerId != iIdentifier)
		{
			break;
		}
	}

	// Should be 16 bits maximum
	UT_ASSERT((iIdentifier & 0xFFFF0000) == 0);

	return iIdentifier;
}
