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
	: m_nIDEvent(0)
{
	setCallback(pCallback);
	setInstanceData(pData);
	m_bStarted = false;
	m_iMilliseconds = 0;

	GR_Win32Graphics * pWinG = static_cast<GR_Win32Graphics *>(pG);
	
	m_hWnd = ((pWinG) ? pWinG->getHwnd() : 0);

	setIdentifier(_createIdentifier());
	m_nIDEvent = _createWin32Identifier();
}


/*****************************************************************/

VOID CALLBACK Global_Win32TimerProc(HWND hwnd, 
									UINT uMsg, 
									UINT idEvent, 
									DWORD dwTime)
{
	UT_Timer* pMyTimer = UT_Win32Timer::findWin32Timer(hwnd, idEvent);
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
	
	UINT idTimer = SetTimer(m_hWnd, m_nIDEvent, iMilliseconds, (TIMERPROC) Global_Win32TimerProc);
	if (idTimer == 0)
		return -1;
	
	m_iMilliseconds = iMilliseconds;
	m_bStarted = true;
	m_nIDEvent = idTimer;
	return 0;
}

void UT_Win32Timer::stop(void)
{
	// stop the delivery of timer events.
	// stop the OS timer from firing, but do not delete the class.
	
	if (m_bStarted) 
		KillTimer(m_hWnd, m_nIDEvent);
	m_bStarted = false;
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

	if ((*ppTimer1)->getHWnd() < (*ppTimer2)->getHWnd())
	{
		return -1;
	}
	
	if ((*ppTimer1)->getHWnd() > (*ppTimer2)->getHWnd())
	{
		return 1;
	}
	
	if ((*ppTimer1)->getWin32Identifier() < (*ppTimer2)->getWin32Identifier())
	{
		return -1;
	}
	
	if ((*ppTimer1)->getWin32Identifier() > (*ppTimer2)->getWin32Identifier())
	{
		return 1;
	}
	
	return 0;
}

UT_uint32 UT_Win32Timer::_createIdentifier(void)
{
	static UT_uint32 uniqueNumber=0;

	// TODO: probably should be protected via mutex or something & handle
	//       case where uniqueNumber wraps (approx after 4294967295 UT_Win32Timers created)
	uniqueNumber++;

	return uniqueNumber;
}

// Returns a number that is at most 16bits & unique for the Window Handle
UINT UT_Win32Timer::_createWin32Identifier(void)
{
	// when hWnd is NULL, the id is specified by OS
	if (this->getHWnd() == 0) return 0;

	// sort the timers so arranged by hWnd and for a given hWnd by OS identifier
	UT_Timer::static_vecTimers.qsort(UT_Win32Timer::_compareIdentifiers);

	// Take the first unused identifier number different from zero (for a given Handle)
	UINT iIdentifier = 1;
	UT_uint32 i, count = _getVecTimers().getItemCount();
	for (i = 0; i < count; i++)
	{
		UT_Win32Timer* pTimer = (UT_Win32Timer*) _getVecTimers().getNthItem(i);
		UT_ASSERT(pTimer);
		
		// have we reached a match, 1st of possibly many
		// [we should always reach a match as we are in list]
		if (pTimer->getHWnd() == this->getHWnd())
		{
			for (; i<count; i++)
			{
				pTimer = (UT_Win32Timer*) _getVecTimers().getNthItem(i);
				UT_ASSERT(pTimer);
		
				// note we are included in this list, if this is initial call from
				// constructor we will have Win32Id of 0, so skip it
				UINT pTId = pTimer->getWin32Identifier();
				if (pTId && (pTId != iIdentifier))
				{
					break;
				}
				else
				{
					if (pTId) iIdentifier++;
				}
			}

			// at this point iIdentifier should be a value >0 that is unique to this hWnd
			break;
		}

		UT_ASSERT(pTimer->getHWnd() < this->getHWnd());
	}
	// if (i>=count) then no timers for this hWnd

	// Should be 16 bits maximum
	UT_ASSERT((iIdentifier & 0xFFFF0000) == 0);

	return iIdentifier;
}

UT_Win32Timer* UT_Win32Timer::findWin32Timer(HWND hwnd, UINT win32ID)
{
	int count = _getVecTimers().getItemCount();
	for (int i=0; i<count; i++)
	{
		UT_Win32Timer* pTimer = (UT_Win32Timer*) _getVecTimers().getNthItem(i);
		UT_ASSERT(pTimer);
		
		if ((pTimer->getHWnd() == hwnd) && (pTimer->getWin32Identifier() == win32ID))
		{
			return pTimer;
		}
	}

	return NULL;
}
