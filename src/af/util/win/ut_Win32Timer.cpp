 
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

#include <windows.h>

#include "ut_Win32Timer.h"
#include "ut_assert.h"

VOID CALLBACK _Win32TimerProc(HWND hwnd, 
						UINT uMsg, 
						UINT idEvent, 
						DWORD dwTime)
{
	UT_Timer* pMyTimer = UT_Timer::findTimer(idEvent);
	UT_ASSERT(pMyTimer);

	pMyTimer->fire();
}

UT_sint32 UT_Win32Timer::set(UT_uint32 iMilliseconds)
{
	UINT idTimer = ::SetTimer(NULL, 0, iMilliseconds, (TIMERPROC) _Win32TimerProc);
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

