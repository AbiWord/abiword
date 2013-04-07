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



#ifndef UT_WIN32TIMER_H
#define UT_WIN32TIMER_H

#include "ut_timer.h"

class ABI_EXPORT UT_Win32Timer : public UT_Timer
{
public:
	UT_Win32Timer(UT_TimerCallback pCallback, void* pData);
	~UT_Win32Timer();

	virtual UT_sint32 set(UT_uint32 iMilliseconds);
	virtual void      stop(void);
	virtual void      start(void);

	bool              isActive(void) const { return m_bStarted; }

	static UT_Win32Timer* findWin32Timer(UINT win32ID);
	static void           pauseAllTimers(bool bPause){s_bPauseAllTimers = bPause;}
	static bool           timersPaused(){return s_bPauseAllTimers;}

protected:
	UT_sint32 m_iMilliseconds;
	bool      m_bStarted;
	UINT      m_win32ID;

private:
	static bool s_bPauseAllTimers;
};

VOID CALLBACK Global_Win32TimerProc(HWND hwnd,
									UINT uMsg,
									UINT idEvent,
									DWORD dwTime);

#endif /* UT_WIN32TIMER_H */

