
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

