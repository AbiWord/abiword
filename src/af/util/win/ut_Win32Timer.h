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
 


#ifndef UT_WIN32TIMER_H
#define UT_WIN32TIMER_H

#include "ut_timer.h"

class ABI_EXPORT UT_Win32Timer : public UT_Timer
{
public:
	UT_Win32Timer(UT_TimerCallback pCallback, void* pData, GR_Graphics * pG);
	~UT_Win32Timer();

	virtual UT_sint32 set(UT_uint32 iMilliseconds);
	virtual void stop(void);
	virtual void start(void);

	bool isActive(void) { return m_bStarted; }

	HWND getHWnd(void) { return m_hWnd; }
	UINT getWin32Identifier(void) { return m_nIDEvent; }
	static UT_Win32Timer* findWin32Timer(HWND hwnd, UINT win32ID);

protected:
	UT_sint32 m_iMilliseconds;
	bool m_bStarted;
	HWND m_hWnd;
	UINT m_nIDEvent;	
	static int _compareIdentifiers(const void* p1, const void* p2);
	UT_uint32 _createIdentifier(void);
	UINT _createWin32Identifier(void);
};

VOID CALLBACK Global_Win32TimerProc(HWND hwnd, 
									UINT uMsg, 
									UINT idEvent, 
									DWORD dwTime);

#endif /* UT_WIN32TIMER_H */

