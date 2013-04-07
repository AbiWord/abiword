/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
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


#include <windows.h>

#include "ut_Win32Timer.h"
#include "ut_assert.h"

bool UT_Win32Timer::s_bPauseAllTimers = false;

/*****************************************************************/
	
UT_Timer* UT_Timer::static_constructor(UT_TimerCallback pCallback, void* pData)
{
	UT_ASSERT(pCallback);
	UT_Win32Timer * p = new UT_Win32Timer(pCallback,pData);

	return p;
}

UT_Win32Timer::UT_Win32Timer(UT_TimerCallback pCallback, void* pData):
	m_iMilliseconds (0),
	m_bStarted (false),
	m_win32ID (0)
{
	setCallback(pCallback);
	setInstanceData(pData);
}


/*****************************************************************/

VOID CALLBACK Global_Win32TimerProc(HWND /*hwnd*/, 
									UINT /*uMsg*/,
									UINT idEvent, 
									DWORD /*dwTime*/)
{
	UT_Timer* pMyTimer = UT_Win32Timer::findWin32Timer(idEvent);
	UT_ASSERT(pMyTimer);

	if (pMyTimer &&
		static_cast<UT_Win32Timer *>(pMyTimer)->isActive() &&
		!UT_Win32Timer::timersPaused())
	{
		pMyTimer->fire();
	}
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
	
	m_win32ID = SetTimer(NULL, 0, iMilliseconds,
						 (TIMERPROC) Global_Win32TimerProc);
	
	if (m_win32ID == 0)
		return -1;
	
	m_iMilliseconds = iMilliseconds;
	m_bStarted = true;

	if (!getIdentifier ())
		setIdentifier (m_win32ID);
	
	return 0;
}

void UT_Win32Timer::stop(void)
{
	// stop the delivery of timer events.
	// stop the OS timer from firing, but do not delete the class.
	if (m_bStarted)
	{
		KillTimer(NULL, m_win32ID);
		m_bStarted = false;
	}
}

void UT_Win32Timer::start(void)
{
	// resume the delivery of events using the last freq set.

	UT_ASSERT(m_iMilliseconds > 0);
	
	if (!m_bStarted)
		set(m_iMilliseconds);
}

UT_Win32Timer* UT_Win32Timer::findWin32Timer(UINT win32ID)
{
	int count = _getVecTimers().getItemCount();
	for (int i=0; i<count; i++)
	{
		UT_Win32Timer* pTimer = (UT_Win32Timer*) _getVecTimers().getNthItem(i);
		UT_ASSERT(pTimer);
		
		if (pTimer->m_win32ID == win32ID)
		{
			return pTimer;
		}
	}

	return NULL;
}
