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
	
UT_Timer* UT_Timer::static_constructor(UT_TimerCallback pCallback,
									   void* pData)
{
	UT_ASSERT(pCallback);

	UT_Timer * p = new UT_Win32Timer();

	if (p)
	{
		p->setCallback(pCallback);
		p->setInstanceData(pData);
	}

	return p;
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
	UINT idTimer = getIdentifier();

	if (idTimer != 0)
	{
		KillTimer(NULL, idTimer);
	}
}

UT_sint32 UT_Win32Timer::set(UT_uint32 iMilliseconds)
{
	UINT idTimer = SetTimer(NULL, 0, iMilliseconds, (TIMERPROC) _Win32TimerProc);
	if (idTimer != 0)
	{
		setIdentifier(idTimer);
		return 0;
	}
	else
	{
		return -1;
	}
}

